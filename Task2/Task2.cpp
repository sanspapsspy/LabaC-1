#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_LOGIN_LEN 6
#define MAX_PIN 100000
#define CONFIRM_VALUE 52

typedef struct {
    char login[MAX_LOGIN_LEN + 1];
    int pin;
    int request_limit;
    int requests_made;
} User;

typedef struct {
    User* users;
    size_t count;
    size_t capacity;
} UserDatabase;

static int is_valid_login(const char* login) {
    if (login == NULL) return 0;
    size_t len = strlen(login);
    if (len == 0 || len > MAX_LOGIN_LEN) return 0;

    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)login[i])) {
            return 0;
        }
    }
    return 1;
}

static int is_valid_pin(int pin) {
    return pin >= 0 && pin <= MAX_PIN;
}

static UserDatabase* db_init(void) {
    UserDatabase* db = (UserDatabase*)malloc(sizeof(UserDatabase));
    if (db == NULL) return NULL;

    db->users = NULL;
    db->count = 0;
    db->capacity = 0;
    return db;
}

static void db_free(UserDatabase* db) {
    if (db != NULL) {
        free(db->users);
        free(db);
    }
}

static int db_add_user(UserDatabase* db, const char* login, int pin) {
    if (db == NULL || !is_valid_login(login) || !is_valid_pin(pin)) {
        return 0;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->users[i].login, login) == 0) {
            return 0;
        }
    }

    if (db->count >= db->capacity) {
        size_t new_capacity = db->capacity == 0 ? 4 : db->capacity * 2;
        User* new_users = (User*)realloc(db->users, new_capacity * sizeof(User));
        if (new_users == NULL) return 0;

        db->users = new_users;
        db->capacity = new_capacity;
    }

    User* new_user = &db->users[db->count];
    strncpy(new_user->login, login, MAX_LOGIN_LEN);
    new_user->login[MAX_LOGIN_LEN] = '\0';
    new_user->pin = pin;
    new_user->request_limit = -1;
    new_user->requests_made = 0;

    db->count++;
    return 1;
}

static User* db_find_user(UserDatabase* db, const char* login) {
    if (db == NULL || login == NULL) return NULL;

    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->users[i].login, login) == 0) {
            return &db->users[i];
        }
    }
    return NULL;
}

static int authenticate(UserDatabase* db, char* login, size_t login_size) {
    printf("Login: ");
    if (fgets(login, login_size, stdin) == NULL) return 0;

    login[strcspn(login, "\n")] = '\0';

    if (!is_valid_login(login)) {
        printf("Invalid login format.\n");
        return 0;
    }

    User* user = db_find_user(db, login);
    if (user == NULL) {
        printf("User not found.\n");
        return 0;
    }

    int pin;
    printf("PIN: ");
    if (scanf("%d", &pin) != 1) {
        while (getchar() != '\n');
        printf("Invalid PIN format.\n");
        return 0;
    }
    while (getchar() != '\n');

    if (pin != user->pin) {
        printf("Invalid PIN.\n");
        return 0;
    }

    return 1;
}

static int register_user(UserDatabase* db) {
    char login[MAX_LOGIN_LEN + 1];
    int pin;

    printf("Enter login (max %d chars, letters and digits only): ", MAX_LOGIN_LEN);
    if (fgets(login, sizeof(login), stdin) == NULL) return 0;
    login[strcspn(login, "\n")] = '\0';

    if (!is_valid_login(login)) {
        printf("Invalid login format.\n");
        return 0;
    }

    if (db_find_user(db, login) != NULL) {
        printf("User already exists.\n");
        return 0;
    }

    printf("Enter PIN (0-%d): ", MAX_PIN);
    if (scanf("%d", &pin) != 1) {
        while (getchar() != '\n');
        printf("Invalid PIN format.\n");
        return 0;
    }
    while (getchar() != '\n');

    if (!is_valid_pin(pin)) {
        printf("PIN out of range.\n");
        return 0;
    }

    return db_add_user(db, login, pin);
}

static void print_time(void) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    printf("%02d:%02d:%02d\n", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
}

static void print_date(void) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    printf("%02d:%02d:%04d\n", tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
}

static int parse_datetime(const char* str, struct tm* tm_out) {
    int day, month, year, hour, min, sec;
    if (sscanf(str, "%d:%d:%d %d:%d:%d", &day, &month, &year, &hour, &min, &sec) != 6) {
        return 0;
    }

    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 0 ||
        hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return 0;
    }

    tm_out->tm_mday = day;
    tm_out->tm_mon = month - 1;
    tm_out->tm_year = year - 1900;
    tm_out->tm_hour = hour;
    tm_out->tm_min = min;
    tm_out->tm_sec = sec;
    tm_out->tm_isdst = -1;

    return 1;
}

static void howmuch(const char* datetime_str, const char* flag) {
    struct tm tm_time;
    if (!parse_datetime(datetime_str, &tm_time)) {
        printf("Invalid datetime format. Use: dd:MM:yyyy hh:mm:ss\n");
        printf("Example: Howmuch 30:03:2026 12:00:00 -s\n");
        return;
    }

    time_t past = mktime(&tm_time);
    if (past == (time_t)-1) {
        printf("Invalid datetime.\n");
        return;
    }

    time_t now = time(NULL);
    double diff = difftime(now, past);

    if (diff < 0) {
        printf("The specified time is in the future.\n");
        return;
    }

    if (strcmp(flag, "-s") == 0) {
        printf("%.0f seconds\n", diff);
    }
    else if (strcmp(flag, "-m") == 0) {
        printf("%.0f minutes\n", diff / 60);
    }
    else if (strcmp(flag, "-h") == 0) {
        printf("%.2f hours\n", diff / 3600);
    }
    else if (strcmp(flag, "-y") == 0) {
        printf("%.4f years\n", diff / (365.25 * 24 * 3600));
    }
    else {
        printf("Invalid flag. Use -s, -m, -h, or -y\n");
        printf("Example: Howmuch 30:03:2026 12:00:00 -s\n");
    }
}

static int sanctions(UserDatabase* db, const char* username) {
    User* user = db_find_user(db, username);
    if (user == NULL) {
        printf("User '%s' not found.\n", username);
        return 0;
    }

    int confirm;
    printf("Enter confirmation value (52) to apply sanctions to '%s': ", username);
    if (scanf("%d", &confirm) != 1) {
        while (getchar() != '\n');
        printf("Invalid input.\n");
        return 0;
    }
    while (getchar() != '\n');

    if (confirm != CONFIRM_VALUE) {
        printf("Confirmation failed. Expected 52.\n");
        return 0;
    }

    int limit;
    printf("Enter request limit for user %s: ", username);
    if (scanf("%d", &limit) != 1) {
        while (getchar() != '\n');
        printf("Invalid input.\n");
        return 0;
    }
    while (getchar() != '\n');

    if (limit < 0) {
        printf("Limit must be non-negative.\n");
        return 0;
    }

    user->request_limit = limit;
    user->requests_made = 0;
    printf("Sanctions applied successfully. User '%s' can now make only %d requests per session.\n",
        username, limit);
    return 1;
}

static int check_request_limit(User* user) {
    if (user == NULL) return 0;

    if (user->request_limit >= 0 && user->requests_made >= user->request_limit) {
        printf("\n!!! WARNING: Request limit exceeded (%d/%d). Logging out !!!\n",
            user->requests_made, user->request_limit);
        return 0;
    }

    user->requests_made++;
    return 1;
}

static void print_help(void) {
    printf("\n=== Available Commands ===\n");
    printf("Time                      - Display current time (HH:MM:SS)\n");
    printf("Date                      - Display current date (DD:MM:YYYY)\n");
    printf("Howmuch <datetime> <flag> - Calculate time elapsed since datetime\n");
    printf("                           Format: DD:MM:YYYY HH:MM:SS\n");
    printf("                           Flags: -s (seconds), -m (minutes), -h (hours), -y (years)\n");
    printf("Sanctions <username>      - Set request limit for user (requires confirmation 52)\n");
    printf("Logout                    - Logout and return to login menu\n");
    printf("Help                      - Show this help message\n");
    printf("==========================\n");
}

static void shell_loop(UserDatabase* db, User* current_user) {
    char command[256];

    printf("\nWelcome %s! Type 'Help' for available commands.\n", current_user->login);
    printf("Request limit: %s\n",
        current_user->request_limit >= 0 ?
        (printf("%d", current_user->request_limit), "set") : "not set");

    while (1) {
        printf("\n%s@shell> ", current_user->login);
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        // Удаляем символ новой строки
        command[strcspn(command, "\n")] = '\0';

        // Пропускаем пустые команды
        if (strlen(command) == 0) continue;

        // Обработка команд
        if (strcmp(command, "Time") == 0 || strcmp(command, "TIME") == 0 || strcmp(command, "time") == 0) {
            if (!check_request_limit(current_user)) break;
            print_time();
        }
        else if (strcmp(command, "Date") == 0 || strcmp(command, "DATE") == 0 || strcmp(command, "date") == 0) {
            if (!check_request_limit(current_user)) break;
            print_date();
        }
        else if (strcmp(command, "Help") == 0 || strcmp(command, "HELP") == 0 || strcmp(command, "help") == 0) {
            if (!check_request_limit(current_user)) break;
            print_help();
        }
        else if (strcmp(command, "Logout") == 0 || strcmp(command, "LOGOUT") == 0 || strcmp(command, "logout") == 0) {
            printf("Logging out...\n");
            break;
        }
        else if (strncmp(command, "Howmuch ", 8) == 0 || strncmp(command, "howmuch ", 8) == 0) {
            if (!check_request_limit(current_user)) break;

            // Пропускаем префикс "Howmuch " (регистронезависимо)
            const char* args = command + 8;
            char datetime[64];
            char flag[8];

            if (sscanf(args, "%63s %7s", datetime, flag) == 2) {
                howmuch(datetime, flag);
            }
            else {
                printf("Usage: Howmuch <datetime> <flag>\n");
                printf("Example: Howmuch 30:03:2026 12:00:00 -s\n");
                printf("Flags: -s (seconds), -m (minutes), -h (hours), -y (years)\n");
            }
        }
        else if (strncmp(command, "Sanctions ", 10) == 0 || strncmp(command, "sanctions ", 10) == 0) {
            if (!check_request_limit(current_user)) break;

            // Пропускаем префикс "Sanctions " (регистронезависимо)
            const char* username = command + 10;

            if (strlen(username) > 0) {
                // Обрезаем пробелы в конце
                char clean_username[MAX_LOGIN_LEN + 1];
                strncpy(clean_username, username, MAX_LOGIN_LEN);
                clean_username[MAX_LOGIN_LEN] = '\0';

                // Удаляем trailing whitespace
                size_t len = strlen(clean_username);
                while (len > 0 && isspace((unsigned char)clean_username[len - 1])) {
                    clean_username[--len] = '\0';
                }

                if (len > 0) {
                    sanctions(db, clean_username);
                }
                else {
                    printf("Usage: Sanctions <username>\n");
                    printf("Example: Sanctions john\n");
                }
            }
            else {
                printf("Usage: Sanctions <username>\n");
                printf("Example: Sanctions john\n");
            }
        }
        else {
            printf("Unknown command: '%s'\n", command);
            printf("Type 'Help' for available commands.\n");
        }
    }
}

int main(void) {
    UserDatabase* db = db_init();
    if (db == NULL) {
        fprintf(stderr, "Failed to initialize database.\n");
        return EXIT_FAILURE;
    }

    // Добавляем тестовых пользователей
    db_add_user(db, "admin", 1234);
    db_add_user(db, "qwerty", 12345);
    db_add_user(db, "user1", 1111);

    while (1) {
        printf("\n========================================\n");
        printf("       Shell Login System\n");
        printf("========================================\n");
        printf("1. Login\n");
        printf("2. Register\n");
        printf("3. Exit\n");
        printf("----------------------------------------\n");
        printf("Choice: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input. Please enter a number (1-3).\n");
            continue;
        }
        while (getchar() != '\n');

        if (choice == 3) {
            printf("Goodbye!\n");
            break;
        }

        char login[MAX_LOGIN_LEN + 1];
        int authenticated = 0;

        if (choice == 1) {
            authenticated = authenticate(db, login, sizeof(login));
        }
        else if (choice == 2) {
            if (register_user(db)) {
                printf("\n✓ Registration successful! Please login.\n");
            }
            else {
                printf("\n✗ Registration failed.\n");
            }
            continue;
        }
        else {
            printf("Invalid choice. Please enter 1, 2, or 3.\n");
            continue;
        }

        if (authenticated) {
            User* user = db_find_user(db, login);
            if (user != NULL) {
                user->requests_made = 0;
                shell_loop(db, user);
            }
        }
    }

    db_free(db);
    return EXIT_SUCCESS;
}