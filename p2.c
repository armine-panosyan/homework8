#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    const char text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char buffer[26];
    int status = 1;

    int fd = open("data.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open for writing");
        return 1;
    }

    ssize_t written = write(fd, text, sizeof text - 1);
    if (written == -1) {
        perror("write");
        goto cleanup;
    }
    if (written != sizeof text - 1) {
        fprintf(stderr, "Could not write all 26 bytes.\n");
        goto cleanup;
    }
    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    fd = open("data.txt", O_RDWR);
    if (fd == -1) {
        perror("reopen");
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        perror("lseek to end");
        goto cleanup;
    }
    printf("Original size: %jd bytes\n", (intmax_t)size);

    if (ftruncate(fd, 10) == -1) {
        perror("ftruncate");
        goto cleanup;
    }

    size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        perror("lseek to end");
        goto cleanup;
    }
    printf("New size: %jd bytes\n", (intmax_t)size);

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("rewind");
        goto cleanup;
    }

    printf("Remaining content: ");
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof buffer)) > 0) {
        if (fwrite(buffer, 1, (size_t)bytes_read, stdout) != (size_t)bytes_read) {
            perror("print content");
            goto cleanup;
        }
    }
    if (bytes_read == -1) {
        perror("read");
        goto cleanup;
    }
    putchar('\n');
    status = 0;

cleanup:
    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    return status;
}
