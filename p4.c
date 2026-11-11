#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    size_t capacity = 256;
    char *entry = malloc(capacity);
    if (entry == NULL) {
        perror("malloc");
        return 1;
    }

    size_t length = (size_t)snprintf(
        entry, capacity, "PID=%jd: ", (intmax_t)getpid()
    );

    fprintf(stderr, "Enter a log entry: ");

    char byte;
    ssize_t bytes_read;
    while ((bytes_read = read(0, &byte, 1)) == 1) {
        if (length == capacity - 1) {
            capacity *= 2;
            char *larger_entry = realloc(entry, capacity);
            if (larger_entry == NULL) {
                perror("realloc");
                free(entry);
                return 1;
            }
            entry = larger_entry;
        }

        entry[length++] = byte;
        if (byte == '\n') {
            break;
        }
    }

    if (bytes_read == -1) {
        perror("read");
        free(entry);
        return 1;
    }

    if (entry[length - 1] != '\n') {
        entry[length++] = '\n';
    }

    int fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open");
        free(entry);
        return 1;
    }

    size_t offset = 0;
    int status = 1;

    while (offset < length) {
        ssize_t written = write(fd, entry + offset, length - offset);
        if (written == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("write");
            goto cleanup;
        }
        if (written == 0) {
            fprintf(stderr, "write made no progress.\n");
            goto cleanup;
        }
        offset += (size_t)written;
    }

    off_t position = lseek(fd, 0, SEEK_CUR);
    if (position == -1) {
        perror("lseek");
        goto cleanup;
    }

    printf("Final file offset: %jd bytes\n", (intmax_t)position);
    status = 0;

cleanup:
    free(entry);
    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    return status;
}
