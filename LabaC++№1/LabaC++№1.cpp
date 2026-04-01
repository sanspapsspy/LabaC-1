#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <locale.h>
#include <string.h>

void print_file_state(FILE* fp, const char* stage) {
    if (!fp) return;

    printf("\n=== Состояние файла (%s) ===\n", stage);

    long pos = ftell(fp);
    if (pos != -1) {
        printf("Текущая позиция: %ld байт\n", pos);
    }
    else {
        perror("ftell");
    }

    printf("Дескриптор файла: %d\n", _fileno(fp));

    clearerr(fp);
    if (feof(fp)) {
        printf("Флаг EOF: УСТАНОВЛЕН\n");
    }
    else {
        printf("Флаг EOF: СБРОШЕН\n");
    }

    if (ferror(fp)) {
        printf("Флаг ошибки: УСТАНОВЛЕН\n");
    }
    else {
        printf("Флаг ошибки: СБРОШЕН\n");
    }

    printf("==========================\n");
}

void show_usage(const char* program_name) {
    printf("Использование:\n");
    printf("  %s <имя_файла>              - создать файл и выполнить все операции\n", program_name);
    printf("  %s rename <старое> <новое>  - переименовать файл\n", program_name);
    printf("  %s help                     - показать эту справку\n", program_name);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "help") == 0) {
        show_usage(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "rename") == 0) {
        if (argc != 4) {
            printf("Ошибка: для переименования нужно указать старое и новое имя файла\n");
            printf("Пример: %s rename oldfile.bin newfile.bin\n", argv[0]);
            return 1;
        }

        if (rename(argv[2], argv[3]) == 0) {
            printf("Файл успешно переименован\n");
            printf("Старое имя: %s\n", argv[2]);
            printf("Новое имя: %s\n", argv[3]);
        }
        else {
            perror("Ошибка переименования файла");
        }
        return 0;
    }

    char cwd[1024];
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("========================================\n");
        printf("Текущая рабочая папка: %s\n", cwd);
        printf("========================================\n\n");
    }
    else {
        perror("Ошибка получения текущей папки");
    }

    const char* filename = argv[1];
    printf("Файл будет создан/открыт по пути: %s\\%s\n\n", cwd, filename);

    unsigned char data[] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 };
    size_t data_size = sizeof(data);

    printf("========================================\n");
    printf("ЗАДАНИЕ 1: РАБОТА С FILE*, FSEEK, FREAD\n");
    printf("========================================\n\n");

    printf("ЧАСТЬ 1: Создание файла и запись данных\n");
    printf("----------------------------------------\n");

    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        perror("Ошибка открытия файла для записи");
        return 1;
    }

    printf("\nФайл успешно открыт: %s\n", filename);
    print_file_state(fp, "после открытия на запись");

    size_t written = fwrite(data, 1, data_size, fp);
    if (written != data_size) {
        perror("Ошибка записи в файл");
        fclose(fp);
        return 1;
    }

    printf("\nЗаписано %zu байт: ", written);
    for (size_t i = 0; i < data_size; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");

    print_file_state(fp, "после записи данных");

    fclose(fp);
    printf("\nФайл закрыт.\n");
    printf("Файл сохранен по пути: %s\\%s\n\n", cwd, filename);

    printf("ЧАСТЬ 2: Побайтовое чтение файла\n");
    printf("--------------------------------\n");

    fp = fopen(filename, "rb");
    if (!fp) {
        perror("Ошибка открытия файла для чтения");
        return 1;
    }

    printf("\nФайл успешно открыт для чтения: %s\n", filename);

    int ch;
    int byte_count = 0;

    while ((ch = fgetc(fp)) != EOF) {
        byte_count++;
        printf("\n--- Байт %d: %d ---\n", byte_count, ch);
        print_file_state(fp, "после чтения байта");
    }

    printf("\nВсего прочитано байт: %d\n", byte_count);
    fclose(fp);
    printf("\nФайл закрыт.\n\n");

    printf("ЧАСТЬ 3: Перемещение указателя и чтение 4 байт\n");
    printf("--------------------------------------------\n");

    fp = fopen(filename, "rb");
    if (!fp) {
        perror("Ошибка открытия файла для повторного чтения");
        return 1;
    }

    printf("\nФайл успешно открыт: %s\n", filename);
    print_file_state(fp, "после открытия");

    printf("\nВыполняется fseek(fp, 3, SEEK_SET)...\n");
    if (fseek(fp, 3, SEEK_SET) != 0) {
        perror("Ошибка fseek");
        fclose(fp);
        return 1;
    }

    print_file_state(fp, "после fseek(3, SEEK_SET)");

    unsigned char buffer[4];
    size_t bytes_read = fread(buffer, 1, 4, fp);

    printf("\nПосле fread:\n");
    print_file_state(fp, "после чтения 4 байт");

    printf("\nРезультат fread:\n");
    printf("Прочитано байт: %zu\n", bytes_read);
    printf("Содержимое буфера: ");
    for (size_t i = 0; i < bytes_read; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");

    fclose(fp);

    printf("\n========================================\n");
    printf("ОТВЕТ НА ВОПРОС ЗАДАНИЯ\n");
    printf("========================================\n");
    printf("После выполнения fseek(fp, 3, SEEK_SET) указатель чтения\n");
    printf("перемещается на 3 байта от начала файла (индекс 3).\n");
    printf("Затем fread считывает 4 байта, начиная с этой позиции.\n\n");

    printf("Содержимое файла по байтам (индексы):\n");
    printf("Индекс: 0   1   2   3   4   5   6   7   8   9   10\n");
    printf("Байт:   3   1   4   1   5   9   2   6   5   3   5\n");
    printf("                      ^\n");
    printf("                      |\n");
    printf("                 fseek(3) указывает сюда\n\n");

    printf("Буфер будет содержать байты с индексов 3, 4, 5, 6: ");
    printf("%d, %d, %d, %d\n", data[3], data[4], data[5], data[6]);
    printf("То есть: 1, 5, 9, 2\n");

    printf("\n========================================\n");
    printf("Файл находится по адресу: %s\\%s\n", cwd, filename);
    printf("========================================\n");

    printf("\n----------------------------------------\n");
    printf("ДЛЯ ПЕРЕИМЕНОВАНИЯ ФАЙЛА:\n");
    printf("Используйте команду: %s rename %s новое_имя\n", argv[0], filename);
    printf("----------------------------------------\n");

    return 0;
}