#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <syslog.h>  // Incluir biblioteca para syslog

#define MAIL_SPAWN_FIFO "/var/tmp/mail_spawn_fifo"

void write_message(const char *remetente, const char *destinatario, const char *message, int id, const char *filepath) {
    syslog(LOG_INFO, "Entrando na função write_message");
    syslog(LOG_INFO, "remetente: %s, destinatario: %s, message: %s, id: %d, filepath: %s", remetente, destinatario, message, id, filepath);
    FILE *file = fopen(filepath, "a");
    if (!file) {
        syslog(LOG_ERR, "Erro ao abrir arquivo: %s", strerror(errno));
        return;
    }
    fprintf(file, "%s;%s;%s\n", remetente, destinatario, message);
    fclose(file);
    syslog(LOG_INFO, "Mensagem escrita com sucesso");
}

int get_next_id(const char *dir_path) {
    syslog(LOG_INFO, "Entrando na função get_next_id");
    int max_id = 0;
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        syslog(LOG_ERR, "Falha ao abrir diretório: %s", strerror(errno));
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            int id;
            if (sscanf(entry->d_name, "%d.txt", &id) == 1) {
                if (id > max_id) {
                    max_id = id;
                }
            }
        }
    }

    closedir(dir);
    syslog(LOG_INFO, "Próximo ID: %d", max_id + 1);
    return max_id + 1;
}

int main() {
    openlog("mail-lspawn", LOG_PID | LOG_CONS, LOG_MAIL);
    syslog(LOG_INFO, "Iniciando mail-lspawn");
    char buffer[1024];

    while (1) {
        syslog(LOG_INFO, "Abrindo FIFO");
        int fd = open(MAIL_SPAWN_FIFO, O_RDONLY);
        if (fd == -1) {
            syslog(LOG_ERR, "Falha ao abrir FIFO: %s", strerror(errno));
            continue;
        }

        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Ensure the string is null-terminated
            syslog(LOG_INFO, "Mensagem recebida: %s", buffer);

            char *type = strtok(buffer, ";");
            char *remetente = strtok(NULL, ";");
            char *destinatario = strtok(NULL, ";");
            char *message = strtok(NULL, "");

            if (type && remetente && destinatario && message) {
                syslog(LOG_INFO, "Processando mensagem, Tipo: %s, Remetente: %s, Destinatário: %s", type, remetente, destinatario);
                char filepath[512];
                if (strcmp(type, "u") == 0) {
                    snprintf(filepath, sizeof(filepath), "/home/%s/%sMail", destinatario, destinatario);
                } else if (strcmp(type, "g") == 0) {
                    snprintf(filepath, sizeof(filepath), "/home/concordia/Grupos/%s", destinatario);
                }
                syslog(LOG_INFO, "Caminho do arquivo: %s", filepath);
                
                int id = get_next_id(filepath);
                snprintf(filepath + strlen(filepath), sizeof(filepath) - strlen(filepath), "/%d.txt", id);
                syslog(LOG_INFO, "Caminho completo do arquivo: %s", filepath);
                write_message(remetente, destinatario, message, id, filepath);
            } else {
                syslog(LOG_ERR, "Formato de mensagem inválido");
            }
        } else {
            syslog(LOG_ERR, "Falha ao ler do FIFO: %s", strerror(errno));
        }

        close(fd);
    }

    closelog();
    return 0;
}
