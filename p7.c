#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int read_path(const char *prompt, char *path, size_t capacity)
{
    fprintf(stderr, "%s", prompt);

    if (fgets(path, capacity, stdin) == NULL) {
        fprintf(stderr, "Could not read the file path.\n");
        return -1;
    }

    size_t length = strcspn(path, "\n");
    if (length == capacity - 1) {
        fprintf(stderr, "File path is too long.\n");
        return -1;
    }

    path[length] = '\0';
    return 0;
}

static ssize_t read_chunk(int fd, unsigned char *buffer, size_t capacity)
{
    size_t total = 0;

    // Keep reading until the buffer is full or we reach EOF.
    while (total < capacity) {
        ssize_t count = read(fd, buffer + total, capacity - total);

        if (count == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (count == 0) {
            break;
        }

        total += (size_t)count;
    }

    return (ssize_t)total;
}

int main(void)
{
    char first_path[4096];
    char second_path[4096];
    unsigned char first_buffer[4096];
    unsigned char second_buffer[4096];
    uintmax_t position = 0;
    int status = 2;

    if (read_path("Enter the first file path: ",
                  first_path, sizeof first_path) == -1 ||
        read_path("Enter the second file path: ",
                  second_path, sizeof second_path) == -1) {
        return 2;
    }

    int first_fd = open(first_path, O_RDONLY);
    if (first_fd == -1) {
        perror("open first file");
        return 2;
    }

    int second_fd = open(second_path, O_RDONLY);
    if (second_fd == -1) {
        perror("open second file");
        close(first_fd);
        return 2;
    }

    for (;;) {
        ssize_t first_count = read_chunk(first_fd, first_buffer,
                                         sizeof first_buffer);
        if (first_count == -1) {
            perror("read first file");
            break;
        }

        ssize_t second_count = read_chunk(second_fd, second_buffer,
                                          sizeof second_buffer);
        if (second_count == -1) {
            perror("read second file");
            break;
        }

        ssize_t common = first_count < second_count ? first_count : second_count;

        for (ssize_t i = 0; i < common; i++) {
            if (first_buffer[i] != second_buffer[i]) {
                printf("Files differ at byte %ju\n", position);
                status = 1;
                goto cleanup;
            }
            position++;
        }

        if (first_count != second_count) {
            printf("Files differ at byte %ju\n", position);
            status = 1;
            break;
        }

        if (first_count == 0) {
            printf("Files are identical\n");
            status = 0;
            break;
        }
    }

cleanup:
    if (close(first_fd) == -1) {
        perror("close first file");
        status = 2;
    }
    if (close(second_fd) == -1) {
        perror("close second file");
        status = 2;
    }

    return status;
}
