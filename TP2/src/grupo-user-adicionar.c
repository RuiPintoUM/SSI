#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <group_name> <username>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *group_name = argv[2];
    const char *username_to_add = argv[1];

    char command[256];

    char *args[] = {"sudo","gpasswd", "-a", username_to_add, group_name, NULL};

    pid_t pid = fork();
            if (pid == 0) {
                if (execvp("sudo", args)) {
                    perror("Falha ao executar execvp");
                    exit(EXIT_FAILURE);
                }
            } else if (pid > 0) {
                wait(NULL);
            } else {
                perror("Falha ao criar processo para system");
                exit(EXIT_FAILURE);
            }
    return 0;
}