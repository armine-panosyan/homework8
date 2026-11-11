#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    char *source_path = NULL;
    char *destination_path = NULL;
    size_t source_capacity = 0;
    size_t destination_capacity = 0;
    int source = -1;
    int destination = -1;
    int status = EXIT_FAILURE;
    char buffer[4096];
    uintmax_t total = 0;

    printf("Enter the source file path: ");
    fflush(stdout);
    if (getline(&source_path, &source_capacity, stdin) == -1) {
        fprintf(stderr, "Could not read the source path.\n");
        goto cleanup;
    }
    source_path[strcspn(source_path, "\n")] = '\0';

    printf("Enter the destination file path: ");
    fflush(stdout);
    if (getline(&destination_path, &destination_capacity, stdin) == -1) {
        fprintf(stderr, "Could not read the destination path.\n");
        goto cleanup;
    }
    destination_path[strcspn(destination_path, "\n")] = '\0';

    source = open(source_path, O_RDONLY);
    if (source == -1) {
        perror("open source");
        goto cleanup;
    }

    destination = open(destination_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destination == -1) {
        perror("open destination");
        goto cleanup;
    }

    for (;;) {
        ssize_t bytes_read = read(source, buffer, sizeof buffer);

        if (bytes_read == -1) {
            perror("read");
            goto cleanup;
        }
        if (bytes_read == 0) {
            break;
        }

        size_t offset = 0;
        while (offset < (size_t)bytes_read) {
            ssize_t bytes_written =
                write(destination, buffer + offset, (size_t)bytes_read - offset);

            if (bytes_written == -1) {
                perror("write");
                goto cleanup;
            }
            if (bytes_written == 0) {
                fprintf(stderr, "write made no progress.\n");
                goto cleanup;
            }

            offset += (size_t)bytes_written;
            total += (uintmax_t)bytes_written;
        }
    }

    status = EXIT_SUCCESS;

cleanup:
    if (source != -1 && close(source) == -1) {
        perror("close source");
        status = EXIT_FAILURE;
    }
    if (destination != -1 && close(destination) == -1) {
        perror("close destination");
        status = EXIT_FAILURE;
    }

    free(source_path);
    free(destination_path);

    if (status == EXIT_SUCCESS) {
        printf("Total bytes copied: %" PRIuMAX "\n", total);
    }

    return status;
}
