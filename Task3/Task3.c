#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    // Проверка количества аргументов
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* source_path = argv[1];
    const char* dest_path = argv[2];

    // Открываем исходный файл для чтения
    FILE* src = fopen(source_path, "rb");
    if (src == NULL) {
        perror("Failed to open source file");
        return EXIT_FAILURE;
    }

    // Открываем файл назначения для записи
    FILE* dst = fopen(dest_path, "wb");
    if (dst == NULL) {
        perror("Failed to open destination file");
        fclose(src);
        return EXIT_FAILURE;
    }

    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t total_copied = 0;

    // Копируем блоками по BUFFER_SIZE байт
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, dst);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Error writing to destination file\n");
            fclose(src);
            fclose(dst);
            return EXIT_FAILURE;
        }
        total_copied += bytes_written;
    }

    // Проверяем ошибки чтения
    if (ferror(src)) {
        perror("Error reading source file");
        fclose(src);
        fclose(dst);
        return EXIT_FAILURE;
    }

    printf("Successfully copied %zu bytes from '%s' to '%s'\n",
        total_copied, source_path, dest_path);

    // Закрываем файлы
    fclose(src);
    fclose(dst);

    return EXIT_SUCCESS;
}