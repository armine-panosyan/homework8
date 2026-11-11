#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int write_all(int fd, const char *buffer, size_t length)
{
    while (length > 0) {
        ssize_t written = write(fd, buffer, length);

        if (written == -1) {
            perror("write");
            return -1;
        }
        if (written == 0) {
            fprintf(stderr, "write made no progress.\n");
            return -1;
        }

        buffer += written;
        length -= (size_t)written;
    }

    return 0;
}

int main(void)
{
    const char numbers[] = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n";
    const char replacement[] = "100\n";
    char remainder[sizeof numbers];
    char buffer[32];
    char byte;
    int status = 1;

    int fd = open("numbers.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open for writing");
        return 1;
    }

    int result = write_all(fd, numbers, sizeof numbers - 1);
    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    if (result == -1) {
        return 1;
    }

    fd = open("numbers.txt", O_RDWR);
    if (fd == -1) {
        perror("open for updating");
        return 1;
    }

    off_t position = 0;
    off_t line4_start = 0;
    int newlines = 0;

    // Scan through line 4 and remember its starting byte offset.
    while (newlines < 4) {
        ssize_t count = read(fd, &byte, 1);
        if (count == -1) {
            perror("read");
            goto cleanup;
        }
        if (count == 0) {
            fprintf(stderr, "Unexpected end of file before line 4 ended.\n");
            goto cleanup;
        }

        position++;
        if (byte == '\n') {
            newlines++;
            if (newlines == 3) {
                line4_start = position;
            }
        }
    }

    // The current position is now the start of line 5.
    size_t remainder_length = 0;
    ssize_t count;

    while (remainder_length < sizeof remainder) {
        count = read(fd, remainder + remainder_length,
                     sizeof remainder - remainder_length);
        if (count == -1) {
            perror("read remainder");
            goto cleanup;
        }
        if (count == 0) {
            break;
        }
        remainder_length += (size_t)count;
    }

    if (lseek(fd, line4_start, SEEK_SET) == -1) {
        perror("seek to line 4");
        goto cleanup;
    }

    if (write_all(fd, replacement, sizeof replacement - 1) == -1 ||
        write_all(fd, remainder, remainder_length) == -1) {
        goto cleanup;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("rewind");
        goto cleanup;
    }

    while ((count = read(fd, buffer, sizeof buffer)) > 0) {
        if (write_all(STDOUT_FILENO, buffer, (size_t)count) == -1) {
            goto cleanup;
        }
    }
    if (count == -1) {
        perror("read final content");
        goto cleanup;
    }

    status = 0;

cleanup:
    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    return status;
}
