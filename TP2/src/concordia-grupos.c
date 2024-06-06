#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>

int main(int argc, char* argv[]) {
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);

    if (pw == NULL) {
        perror("getpwuid");
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1], "c") == 0){
        pid_t pid = fork();
        if (pid == 0) {
            printf("grupo-criar %s\n", argv[2]);
            execl("grupo-criar", "grupo-criar", argv[2], pw->pw_name, NULL);
            perror("Falha ao executar grupo-criar");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("Falha ao criar processo para grupo-criar");
            exit(EXIT_FAILURE);
        }
    }

    if(strcmp(argv[1], "d") == 0){
        char *username = pw->pw_name;

        pid_t pid = fork();
        if (pid == 0) {
            printf("grupo-remover %s %s\n", username, argv[2]);
            execl("grupo-remover", "grupo-remover", username, argv[2], NULL);
            perror("Falha ao executar grupo-remover");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("Falha ao criar processo para grupo-remover");
            exit(EXIT_FAILURE);
        }
    }

    if(strcmp(argv[1], "l") == 0){
        pid_t pid = fork();
        if (pid == 0) {
            execl("grupo-listar", "grupo-listar", argv[2], NULL);
            perror("Falha ao executar grupo-listar");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("Falha ao criar processo para grupo-criar");
            exit(EXIT_FAILURE);
        }
    }

    if(strcmp(argv[1], "a") == 0){
        char *username = pw->pw_name;

        pid_t pid = fork();
        if (pid == 0) {
            execl("grupo-user-adicionar", "grupo-user-adicionar", argv[2], argv[3], NULL);
            perror("Falha ao executar grupo-adicionar");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("Falha ao criar processo para grupo-adicionar");
            exit(EXIT_FAILURE);
        }
    }

    if(strcmp(argv[1], "r") == 0){
        char *username = pw->pw_name;

        pid_t pid = fork();
        if (pid == 0) {
            execl("grupo-user-remover", "grupo-user-remover", argv[2], argv[3], NULL);
            perror("Falha ao executar grupo-remover");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("Falha ao criar processo para grupo-remover");
            exit(EXIT_FAILURE);
        }
    }

    if(strcmp(argv[1], "r"))
    
    return 0;
}