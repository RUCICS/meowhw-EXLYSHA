#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>

long get_page_size();

long get_fs_block_size(int fd) {
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        return -1;
    }
    return st.st_blksize;
}

long gcd(long a, long b) {
    while (b != 0) {
        long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

long lcm(long a, long b) {
    return (a / gcd(a, b)) * b;
}

size_t io_blocksize(int fd) {
    long page_size = get_page_size();
    long fs_block_size = get_fs_block_size(fd);
    if (fs_block_size == -1) {
        return page_size;
    }
    return (size_t)lcm(page_size, fs_block_size);
}

void *aligned_allocate_page_size(size_t size) {
    void *memptr;
    int ret = posix_memalign(&memptr, size, size);
    if (ret != 0) {
        fprintf(stderr, "posix_memalign 失败，错误码: %d\n", ret);
        return NULL;
    }
    return memptr;
}

void aligned_free(void *ptr) {
    free(ptr);
}

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

    size_t buf_size = io_blocksize(fd);
    char *buffer = (char*)aligned_allocate_page_size(buf_size);
    if (buffer == NULL) {
        close(fd);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, buf_size)) > 0) {
        ssize_t total_written = 0;
        while (total_written < bytes_read) {
            ssize_t bytes_written = write(STDOUT_FILENO, buffer + total_written, bytes_read - total_written);
            if (bytes_written == -1) {
                perror("写入标准输出错误");
                aligned_free(buffer);
                close(fd);
                exit(EXIT_FAILURE);
            }
            total_written += bytes_written;
        }
    }

    if (bytes_read == -1) {
        perror("从文件读取时发生错误");
        aligned_free(buffer);
        close(fd);
        exit(EXIT_FAILURE);
    }

    aligned_free(buffer);
    if (close(fd) == -1) {
        perror("关闭文件时发生错误");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

long get_page_size() {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("sysconf(_SC_PAGESIZE) 获取失败");
        fprintf(stderr, "无法获取系统页大小，将使用默认值 4096 字节\n");
        return 4096;
    }
    return page_size;
}
