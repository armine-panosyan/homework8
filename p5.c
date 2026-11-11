#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int fd = open("sparse.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open for writing");
        return 1;
    }

    if (write(fd, "START", 5) != 5) {
        fprintf(stderr, "Could not write all 5 bytes of START.\n");
        close(fd);
        return 1;
    }
    if (lseek(fd, 1024 * 1024, SEEK_CUR) == -1) {
        perror("lseek forward");
        close(fd);
        return 1;
    }

    if (write(fd, "END", 3) != 3) {
        fprintf(stderr, "Could not write all 3 bytes of END.\n");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close after writing");
        return 1;
    }

    fd = open("sparse.bin", O_RDONLY);
    if (fd == -1) {
        perror("open for reading");
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        perror("lseek to end");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close after reading");
        return 1;
    }

    printf("Apparent file size: %jd bytes\n", (intmax_t)size);
    return 0;
}
