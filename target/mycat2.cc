#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

long io_blocksize() {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("sysconf(_SC_PAGESIZE) 获取失败");
        fprintf(stderr, "无法获取系统页大小，将使用默认值 4096 字节\n");
        return 4096;
    }
    return page_size;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "使用方法: %s <文件名>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    long buffer_size = io_blocksize();
    char *buffer = (char*)malloc(buffer_size);
    if (buffer == NULL) {
        perror("缓冲区内存分配失败");
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("无法打开文件");
        free(buffer); // 在退出前，释放已分配的内存
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {
        ssize_t total_written = 0;
        while (total_written < bytes_read) {
            ssize_t bytes_written = write(STDOUT_FILENO, buffer + total_written, bytes_read - total_written);
            if (bytes_written == -1) {
                perror("写入标准输出错误");
                free(buffer);
                close(fd);
                exit(EXIT_FAILURE);
            }
            total_written += bytes_written;
        }
    }

    if (bytes_read == -1) {
        perror("从文件读取时发生错误");
        free(buffer);
        close(fd);
        exit(EXIT_FAILURE);
    }

    free(buffer);
    if (close(fd) == -1) {
        perror("关闭文件时发生错误");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
