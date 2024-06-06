#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>

#define MAX_FILENAME_SIZE 256
#define MAX_CONTENT_SIZE 1024

void print_file_info(const char *filepath, const char *filename) {
    char content[MAX_CONTENT_SIZE];
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("fopen");
        return;
    }

    if (fgets(content, sizeof(content), file) == NULL) {
        perror("fgets");
        fclose(file);
        return;
    }
    fclose(file);

    size_t len = strlen(content);
    if (len > 0 && content[len - 1] == '\n') {
        content[len - 1] = '\0';
    }

    // Extrair remetente e destinatário
    char *first_separator = strchr(content, ';');
    char *second_separator = first_separator ? strchr(first_separator + 1, ';') : NULL;

    if (!first_separator || !second_separator) {
        fprintf(stderr, "Invalid message format: %s\n", content);
        return;
    }

    char remetente[MAX_FILENAME_SIZE];
    char destinatario[MAX_FILENAME_SIZE];

    size_t remetente_len = first_separator - content;
    size_t destinatario_len = second_separator - first_separator - 1;

    strncpy(remetente, content, remetente_len);
    remetente[remetente_len] = '\0';
    strncpy(destinatario, first_separator + 1, destinatario_len);
    destinatario[destinatario_len] = '\0';

    struct stat file_stat;
    if (stat(filepath, &file_stat) == -1) {
        perror("stat");
        return;
    }

    char date_str[256];
    strftime(date_str, sizeof(date_str), "%d-%m-%Y %H:%M:%S", localtime(&file_stat.st_mtime));

    char id[256];
    strncpy(id, filename, strlen(filename) - 4);
    id[strlen(filename) - 4] = '\0';

    printf("%s | %s | %s | %s | %s\n", id, filename, date_str, remetente, destinatario);
}

int main() {
    char *username = getlogin();
    if (!username) {
        perror("getlogin");
        exit(EXIT_FAILURE);
    }

    char user_dir[MAX_FILENAME_SIZE];
    snprintf(user_dir, sizeof(user_dir), "/home/%s/%sMail", username, username);

    DIR *dir = opendir(user_dir);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    printf("id | nome do ficheiro | data de receção | remetente | destinatário\n");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char filepath[MAX_FILENAME_SIZE];
            snprintf(filepath, sizeof(filepath), "%s/%s", user_dir, entry->d_name);

            print_file_info(filepath, entry->d_name);
        }
    }

    closedir(dir);
    return 0;
}