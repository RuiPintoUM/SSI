#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <libgen.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <mid>\n", argv[0]);
        return 1;
    }

    char *id = strdup(argv[1]);
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);

    if (!pw) {
        fprintf(stderr, "Error: getpwuid failed\n");
        free(id);
        return 1;
    }

    char path[1024];
    snprintf(path, sizeof(path), "/home/%s/%sMail/%s.txt", pw->pw_name, pw->pw_name, id);

    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o ficheiro: %s\n", path);
        free(id);
        return 1;
    }

    char line[256];
    if (fgets(line, sizeof(line), file)) {
        char *sender = strtok(line, ";");
        if (sender) {
            char *recipient = strtok(NULL, ";");
            if (recipient) {
                printf("sender: %s\n", sender);

                char exe_path[1024];
                ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
                if (len == -1) {
                    perror("readlink");
                    fclose(file);
                    free(id);
                    return 1;
                }
                exe_path[len] = '\0';
                char *dir = dirname(exe_path);

                char enviar_path[1024];
                snprintf(enviar_path, sizeof(enviar_path), "%s/concordia-enviar", dir);
                printf("enviar_path: %s\n", enviar_path);

                pid_t pid = fork();
                if (pid == 0) {
                    fclose(file);
                    execl(enviar_path, "concordia-enviar", sender, (char *)NULL);
                    perror("execl");
                    exit(1);
                } else if (pid > 0) {
                    int status;
                    waitpid(pid, &status, 0);
                    if (WIFEXITED(status)) {
                        printf("concordia-enviar exited with status %d\n", WEXITSTATUS(status));
                    } else {
                        fprintf(stderr, "concordia-enviar did not exit normally\n");
                    }
                } else {
                    perror("fork");
                    fclose(file);
                    free(id);
                    return 1;
                }
            } else {
                fprintf(stderr, "Formato inválido no ficheiro: falta destinatário\n");
            }
        } else {
            fprintf(stderr, "Formato inválido no ficheiro: falta remetente\n");
        }
    } else {
        fprintf(stderr, "Erro ao ler o ficheiro: %s\n", path);
    }

    fclose(file);
    free(id);
    return 0;
}
