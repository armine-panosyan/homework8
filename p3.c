#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    char path[4096];
    char byte;
    int status = 1;

    fprintf(stderr, "Enter the file path: ");
    if (fgets(path, sizeof path, stdin) == NULL) {
        fprintf(stderr, "Could not read the file path.\n");
        return 1;
    }
    path[strcspn(path, "\n")] = '\0';

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    off_t position = lseek(fd, 0, SEEK_END);
    if (position == -1) {
        perror("lseek to end");
        goto cleanup;
    }

    while (position > 0) {
        position--;

        if (lseek(fd, position, SEEK_SET) == -1) {
            perror("lseek");
            goto cleanup;
        }

        ssize_t bytes_read = read(fd, &byte, 1);
        if (bytes_read == -1) {
            perror("read");
            goto cleanup;
        }
        if (bytes_read == 0) {
            fprintf(stderr, "Unexpected end of file.\n");
            goto cleanup;
        }

        if (write(STDOUT_FILENO, &byte, 1) != 1) {
            perror("write byte");
            goto cleanup;
        }
    }

    if (write(STDOUT_FILENO, "\n", 1) != 1) {
        perror("write newline");
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
