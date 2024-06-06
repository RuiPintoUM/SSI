#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <mid>\n", argv[0]);
        return 1;
    }

    char *id = strdup(argv[1]);
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);

    if (!pw) {
        fprintf(stderr, "Error: getpwuid\n");
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
    while (fgets(line, sizeof(line), file)) {
        char *first_semicolon = strchr(line, ';');
        if (first_semicolon) {
            char *second_semicolon = strchr(first_semicolon + 1, ';');
            if (second_semicolon) {
                // A mensagem inicia após a segunda ";"
                char *message = second_semicolon + 1;
                // Imprime a mensagem
                printf("%s", message);
            } else {
                fprintf(stderr, "Formato inválido na linha: %s\n", line);
            }
        } else {
            fprintf(stderr, "Formato inválido na linha: %s\n", line);
        }
    }

    fclose(file);
    free(id);
    return 0;
}
