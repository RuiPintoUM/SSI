#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>

void save_to_file(const char *username, const char *folder_path) {
    FILE *fp = fopen("users.txt", "a");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%s:%s\n", username, folder_path);
    fclose(fp);
}

void remove_from_file(const char *username) {
    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    FILE *tmp_fp = fopen("users.tmp", "w");
    if (tmp_fp == NULL) {
        perror("fopen");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    char line[256];
    char *token;
    while (fgets(line, sizeof(line), fp) != NULL) {
        token = strtok(line, ":");
        if (strcmp(token, username) != 0) {
            fputs(line, tmp_fp);
        }
    }

    fclose(fp);
    fclose(tmp_fp);

    if (rename("users.tmp", "users.txt") != 0) {
        perror("rename");
        exit(EXIT_FAILURE);
    }
}

void remove_directory(const char *folder_path) {
    DIR *d = opendir(folder_path);
    if (!d) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", folder_path);
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", folder_path, dir->d_name);
            if (remove(path) != 0) {
                perror("remove");
                exit(EXIT_FAILURE);
            }
        }
    }
    closedir(d);

    if (remove(folder_path) != 0) {
        perror("remove");
        exit(EXIT_FAILURE);
    } else {
        printf("Directory %s removed successfully!\n", folder_path);
    }
}

int main() {
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw == NULL) {
        perror("getpwuid");
        exit(EXIT_FAILURE);
    }

    char *username = pw->pw_name;
    char folder_name[100];
    snprintf(folder_name, sizeof(folder_name), "%sMail", username);

    char folder_path[256];
    snprintf(folder_path, sizeof(folder_path), "/home/%s/%s", username, folder_name);

    remove_directory(folder_path);

    return 0;
}
