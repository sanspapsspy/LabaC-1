#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Функция проверки, является ли число простым
int is_prime(unsigned char n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

// Флаг 1: xor8 - XOR всех байтов файла
void flag_xor8(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return;
    }

    uint8_t result = 0;
    int byte;
    long count = 0;

    while ((byte = fgetc(fp)) != EOF) {
        result ^= (uint8_t)byte;
        count++;
    }

    fclose(fp);

    printf("XOR8 result: 0x%02X (%d)\n", result, result);
    printf("Processed %ld bytes\n", count);
}

// Флаг 2: xorodd - XOR четырёхбайтовых последовательностей, где есть простой байт
void flag_xorodd(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return;
    }

    // Получаем размер файла
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < 4) {
        printf("File too small for 4-byte sequences (size: %ld bytes)\n", file_size);
        fclose(fp);
        return;
    }

    uint8_t buffer[4];
    uint32_t xor_result = 0;
    int sequences_processed = 0;
    int sequences_with_prime = 0;

    // Читаем файл по 4 байта
    for (long i = 0; i <= file_size - 4; i++) {
        // Читаем 4 байта на текущей позиции
        fseek(fp, i, SEEK_SET);
        size_t bytes_read = fread(buffer, 1, 4, fp);

        if (bytes_read == 4) {
            sequences_processed++;

            // Проверяем, есть ли среди байтов простое число
            int has_prime = 0;
            for (int j = 0; j < 4; j++) {
                if (is_prime(buffer[j])) {
                    has_prime = 1;
                    break;
                }
            }

            // Если есть простое число - XORим эту последовательность
            if (has_prime) {
                sequences_with_prime++;
                // Преобразуем 4 байта в 32-битное число (little-endian)
                uint32_t value = buffer[0] | (buffer[1] << 8) |
                    (buffer[2] << 16) | (buffer[3] << 24);
                xor_result ^= value;

                printf("Sequence at offset %ld: [%d %d %d %d] -> 0x%08X (has prime: yes)\n",
                    i, buffer[0], buffer[1], buffer[2], buffer[3], value);
            }
            else {
                printf("Sequence at offset %ld: [%d %d %d %d] -> (has prime: no, skipped)\n",
                    i, buffer[0], buffer[1], buffer[2], buffer[3]);
            }
        }
    }

    fclose(fp);

    printf("\nXORODD result: 0x%08X (%u)\n", xor_result, xor_result);
    printf("Total 4-byte sequences: %d\n", sequences_processed);
    printf("Sequences with prime byte: %d\n", sequences_with_prime);
}

// Флаг 3: mask - подсчет четырёхбайтовых чисел, соответствующих маске
void flag_mask(const char* filename, uint32_t mask_value) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return;
    }

    // Получаем размер файла
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < 4) {
        printf("File too small for 4-byte numbers (size: %ld bytes)\n", file_size);
        fclose(fp);
        return;
    }

    uint8_t buffer[4];
    int count = 0;
    int total_numbers = 0;

    printf("Mask: 0x%08X\n", mask_value);
    printf("Mask binary: ");
    for (int i = 31; i >= 0; i--) {
        printf("%d", (mask_value >> i) & 1);
        if (i % 8 == 0) printf(" ");
    }
    printf("\n\n");

    // Читаем файл по 4 байта
    for (long i = 0; i <= file_size - 4; i++) {
        fseek(fp, i, SEEK_SET);
        size_t bytes_read = fread(buffer, 1, 4, fp);

        if (bytes_read == 4) {
            total_numbers++;

            // Преобразуем 4 байта в 32-битное число (little-endian)
            uint32_t value = buffer[0] | (buffer[1] << 8) |
                (buffer[2] << 16) | (buffer[3] << 24);

            // Проверяем соответствие маске: (value & mask) == mask
            if ((value & mask_value) == mask_value) {
                count++;
                printf("Number %d at offset %ld: 0x%08X ", total_numbers, i, value);
                printf("[");
                for (int j = 0; j < 4; j++) {
                    printf("%d", buffer[j]);
                    if (j < 3) printf(" ");
                }
                printf("] -> MATCHES mask\n");

                // Показываем побитово
                printf("  value: ");
                for (int b = 31; b >= 0; b--) {
                    printf("%d", (value >> b) & 1);
                    if (b % 8 == 0) printf(" ");
                }
                printf("\n  mask:  ");
                for (int b = 31; b >= 0; b--) {
                    printf("%d", (mask_value >> b) & 1);
                    if (b % 8 == 0) printf(" ");
                }
                printf("\n  & res:");
                for (int b = 31; b >= 0; b--) {
                    printf("%d", ((value & mask_value) >> b) & 1);
                    if (b % 8 == 0) printf(" ");
                }
                printf("\n\n");
            }
        }
    }

    fclose(fp);

    printf("\nMASK result: %d numbers match the mask\n", count);
    printf("Total 4-byte numbers processed: %d\n", total_numbers);
}

// Функция для создания тестовых файлов
void create_test_files() {
    // Тест для xor8
    FILE* f1 = fopen("test_xor8.bin", "wb");
    unsigned char data1[] = { 1, 2, 3, 4 };
    fwrite(data1, 1, 4, f1);
    fclose(f1);

    // Тест для xorodd
    FILE* f2 = fopen("test_xorodd.bin", "wb");
    // 2 - простое, 4 - нет, 3 - простое, 5 - простое
    unsigned char data2[] = { 2, 4, 3, 5, 6, 7, 8, 9 };
    fwrite(data2, 1, 8, f2);
    fclose(f2);

    // Тест для mask
    FILE* f3 = fopen("test_mask.bin", "wb");
    // Числа: 0x0F0F0F0F, 0x12345678, 0xF0F0F0F0
    unsigned char data3[] = { 0x0F, 0x0F, 0x0F, 0x0F,  // 0x0F0F0F0F
                             0x78, 0x56, 0x34, 0x12,  // 0x12345678
                             0xF0, 0xF0, 0xF0, 0xF0 }; // 0xF0F0F0F0
    fwrite(data3, 1, 12, f3);
    fclose(f3);

    printf("Test files created:\n");
    printf("  test_xor8.bin    - for xor8 flag\n");
    printf("  test_xorodd.bin  - for xorodd flag\n");
    printf("  test_mask.bin    - for mask flag\n");
}

int main(int argc, char* argv[]) {
    // Специальный режим для создания тестовых файлов
    if (argc == 2 && strcmp(argv[1], "--create-tests") == 0) {
        create_test_files();
        return 0;
    }

    // Проверка аргументов
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s <file> xor8\n", argv[0]);
        fprintf(stderr, "  %s <file> xorodd\n", argv[0]);
        fprintf(stderr, "  %s <file> mask <hex>\n", argv[0]);
        fprintf(stderr, "  %s --create-tests  (to create test files)\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    const char* flag = argv[2];

    if (strcmp(flag, "xor8") == 0) {
        flag_xor8(filename);
    }
    else if (strcmp(flag, "xorodd") == 0) {
        flag_xorodd(filename);
    }
    else if (strcmp(flag, "mask") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Error: mask flag requires a hex argument\n");
            fprintf(stderr, "Usage: %s <file> mask <hex>\n", argv[0]);
            return 1;
        }

        unsigned int mask_value;
        if (sscanf(argv[3], "%x", &mask_value) != 1) {
            fprintf(stderr, "Error: invalid hex value: %s\n", argv[3]);
            return 1;
        }

        flag_mask(filename, (uint32_t)mask_value);
    }
    else {
        fprintf(stderr, "Error: unknown flag '%s'\n", flag);
        fprintf(stderr, "Available flags: xor8, xorodd, mask\n");
        return 1;
    }

    return 0;
}