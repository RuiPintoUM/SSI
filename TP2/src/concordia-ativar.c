#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <sys/types.h>

void save_to_file(const char *username, const char *folder_path) {
    FILE *fp = fopen("data/users.txt", "a");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%s:%s\n", username, folder_path);
    fclose(fp);
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

    mode_t mode = S_IRWXU; // Set permissions for the owner (user)

    if (mkdir(folder_path, mode) != 0) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    } else {
        printf("Directory %s created successfully!\n", folder_name);
        save_to_file(username, folder_path);
    }

    return 0;
}
