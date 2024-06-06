#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_GROUP_NAME 100

int criar_user_grupo(char* groupname, char* username) {
    printf("Creating group: %s\n", groupname); // Debug print
    pid_t pid = fork();

    if (pid == 0) {
        char *create[] = {"groupadd", groupname, NULL};

        if (execvp("groupadd", create) == -1) {
            perror("Falha ao executar execvp");
            return 1;
        }  
    } 
    else if (pid > 0) {
        wait(NULL);
        printf("Group creation completed\n"); // Debug print

    
        char folder_path[256];
        snprintf(folder_path, sizeof(folder_path), "/home/concordia/Grupos/%s", groupname);

        mode_t mode = S_IRWXU; // Set permissions for the owner (user)

       if (mkdir(folder_path, mode) != 0) {
        perror("mkdir");
        exit(EXIT_FAILURE);
        } else {
            printf("Diretoria %s criada com sucesso nos grupos!\n", groupname);

            char *args[] = {"sudo","gpasswd", "-a", username, groupname, NULL};
            
            
            pid = fork();
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

        }

    }
    return 0;
}

int main(int argc, char* argv[]) {

    char *groupname = argv[1];
    char *username = argv[2];

    if (strlen(groupname) > MAX_GROUP_NAME) {
        fprintf(stderr, "Group name too long. Maximum length is %d.\n", MAX_GROUP_NAME);
        exit(EXIT_FAILURE);
    }
    
    printf("Creating group...\n");

    printf("Groupname: %s\n", groupname); // Debug print

    criar_user_grupo(groupname, username);

    return 0;
}