#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>

#define MAX_MESSAGE_SIZE 1024

char* get_current_username() {
    __uid_t current_uid = getuid();
    struct passwd *pw = getpwuid(current_uid);
    if(pw == NULL) {
        perror("getpwuid");
        exit(EXIT_FAILURE);
    }
    return strdup(pw->pw_name);
}

int main(int argc, char *argv[]) {
    int pipefd[2];
    char *dest = NULL;
    char is_group = 0;

    if(argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: %s [-g <groupname>] <username>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(argc == 2) {
        dest = strdup(argv[1]);
    } else if (argc == 3 && strcmp(argv[1], "-g") == 0) {
        is_group = 1;
        dest = strdup(argv[2]);
    } else {
        fprintf("stderr", "Usage: %s [-g <groupname>] <username>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char exec_path[PATH_MAX];
    if(readlink("/proc/self/exe", exec_path, PATH_MAX) == -1) {
        perror("readlink");
        exit(EXIT_FAILURE);
    }

    char *slash_ptr = strrchr(exec_path, '/');
    if(slash_ptr != NULL) {
        *slash_ptr = '\0';
    }
    char mail_queue_path[PATH_MAX];
    snprintf(mail_queue_path, sizeof(mail_queue_path), "%s/mail-queue", exec_path);

    printf("mail-queue_path: %s\n", mail_queue_path);

    if(pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if(pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(pid == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);

        if(is_group) {
            execl(mail_queue_path, "mail-queue", "-g", dest, (char *)NULL);
        } else {
            execl(mail_queue_path, "mail-queue", dest, (char *)NULL);
        }
        perror("execl(mail-queue)");
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[0]);
        char message[MAX_MESSAGE_SIZE];
        printf("Enter your email message:\n");
        fgets(message, MAX_MESSAGE_SIZE, stdin);

        if(write(pipefd[1], message, strlen(message)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]);

        wait(NULL);
    }

    free(dest);
    return 0;
}