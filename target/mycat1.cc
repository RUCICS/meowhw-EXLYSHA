#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "使用方法: %s <文件名>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("无法打开文件");
        exit(EXIT_FAILURE);
    }

    char buffer; // 用于存放单个字符的缓冲区
    ssize_t bytes_read; // read 返回读取的字节数

    while ((bytes_read = read(fd, &buffer, 1)) > 0) {
        if (write(STDOUT_FILENO, &buffer, 1) == -1) {
            perror("写入错误");
            close(fd); // 在退出前尝试关闭文件
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read == -1) {
        perror("读取错误");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1) {
        perror("关闭文件错误");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
