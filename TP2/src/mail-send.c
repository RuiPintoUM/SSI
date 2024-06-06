#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <syslog.h>

#define MAIL_QUEUE_DIR "/home/concordia/queue/" // Alterado para diretório relativo
#define MAIL_SPAWN_FIFO "/var/tmp/mail_spawn_fifo"

void send_message(const char *filename) {
    char filepath[PATH_MAX];
    printf("[mail-send] argument filename: %s\n", filename);
    snprintf(filepath, sizeof(filepath), "%s%s", MAIL_QUEUE_DIR, filename);

    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("(mail-send)fopen");
        return;
    }

    char buffer[1024];
    fread(buffer, sizeof(char), sizeof(buffer), file);
    fclose(file);
    syslog(LOG_INFO, "Sending message to lspawn: %s", buffer);
    printf("[mail-send] Sending message to lspawn: %s\n", buffer);

    int fd = open(MAIL_SPAWN_FIFO, O_WRONLY);
    if (fd == -1) {
        perror("[mail-send] send_message function: open fifo");
        return;
    }

    write(fd, buffer, strlen(buffer) + 1);
    close(fd);

    remove(filepath);
}

int main() {
    pid_t pid = fork();
    if(pid == 0) {
        // Processo filho
        printf("processo filho\n");
        syslog(LOG_INFO, "Processo filho");
        char exe_path[PATH_MAX];
        syslog(LOG_INFO, "before_readlink");
        if(readlink("/proc/self/exe", exe_path, PATH_MAX) == -1) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }
        printf("exe_path: %s\n", exe_path);
        syslog(LOG_INFO, "exe_path: %s", exe_path);
        
        char *slash_ptr = strrchr(exe_path, '/');
        if(slash_ptr != NULL) {
            *slash_ptr = '\0';
        }
        syslog(LOG_INFO, "slash_ptr: %s", slash_ptr);
        syslog(LOG_INFO, "exe_path: %s", exe_path);
        char mail_lspawn_path[PATH_MAX];
        snprintf(mail_lspawn_path, sizeof(mail_lspawn_path), "%s/mail-lspawn", exe_path);
        syslog(LOG_INFO, "mail-lspawn path: %s", mail_lspawn_path);
        printf("mail-lspawn path: %s\n", mail_lspawn_path);
        execl(mail_lspawn_path, "mail-lspawn", NULL);
        perror("execl mail-lspawn");
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    printf("Iniciando monitoramento do diretório...\n");

    while (1) {
        DIR *dir = opendir(MAIL_QUEUE_DIR);
        if (!dir) {
            perror("opendir dentro do mail send");
            exit(EXIT_FAILURE);
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                send_message(entry->d_name);
                syslog(LOG_INFO, "Message sent: %s", entry->d_name);
            }
        }

        closedir(dir);
        sleep(1);
    }

    return 0;
}
