#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <syslog.h>

#define MAX_MESSAGE_SIZE 1024
#define MESSAGE_DIR "/home/concordia/queue"
#define GROUPS_DIR "/home/concordia/queue"
#define MESSAGE_FILE_PREFIX "message"

int main(int argc, char *argv[]) {
    printf("entrei no mail-queue\n");
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (pw == NULL) {
        syslog(LOG_ERR, "Failed to get passwd struct: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    char full_message[MAX_MESSAGE_SIZE + 256], filename[256];
    // Open a connection to the syslog

    printf("", pw->pw_name);


    if (argc < 2 || (argc == 3 && strcmp(argv[1], "-g") != 0)) {
        printf("Usage: %s [-g <groupname>] <remetente>", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Username (getpwuid): %s", pw->pw_name);
    char *remetente = pw->pw_name;
    int is_group = 0;
    char *dest = NULL;
    if(argc == 2) {
        dest = strdup(argv[1]);
        is_group = 0;
    } else if(argc == 3 && strcmp(argv[1], "-g") == 0) {
        is_group = 1;
        dest = strdup(argv[2]);
    } else {
        printf("Usage: %s [-g <groupname>] <username>", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(dest == NULL) {
        printf("Failed to duplicate destination: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char message[MAX_MESSAGE_SIZE];
    if (fgets(message, MAX_MESSAGE_SIZE, stdin) == NULL) {
        printf("Failed to read message from stdin: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    size_t len = strlen(message);
    if (len > 0 && message[len - 1] == '\n') {
        message[len - 1] = '\0';
    }
    
    if(is_group) { // no caso de ser para um grupo
        struct stat st;
        if (stat(GROUPS_DIR, &st) == -1) {
            if (errno == ENOENT) {
                // Directory does not exist, try to create it
                if (mkdir(GROUPS_DIR, 0700) == -1) {
                    printf("Failed to create directory: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                printf("Directory created: %s", GROUPS_DIR);
            } else {
                printf("Failed to stat directory: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
        } else if (!S_ISDIR(st.st_mode)) {
            printf("Error: %s is not a directory", GROUPS_DIR);
            exit(EXIT_FAILURE);
        } else if ((st.st_mode & S_IWUSR) == 0) {
            printf("Error: No write permission for %s", GROUPS_DIR);
            exit(EXIT_FAILURE);
        }
        snprintf(full_message, sizeof(full_message), "g;%s;%s;%s", remetente, dest, message);
        snprintf(filename, sizeof(filename), "%s/%s%d", GROUPS_DIR, MESSAGE_FILE_PREFIX, getpid());
        printf("Creating file: %s", filename);
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd == -1) {
            printf("Failed to open file: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (write(fd, full_message, strlen(full_message)) == -1) {
            printf("Failed to write to file: %s", strerror(errno));
            close(fd);
            exit(EXIT_FAILURE);
        }
        close(fd);
        printf("Message saved to: %s", filename);
    } else { // no caso de ser para um utilizador
        struct stat st;
        if (stat(MESSAGE_DIR, &st) == -1) {
            if (errno == ENOENT) {
                // Directory does not exist, try to create it
                if (mkdir(MESSAGE_DIR, 0700) == -1) {
                    printf("Failed to create directory: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                printf("Directory created: %s\n", MESSAGE_DIR);
            } else {
                printf("Failed to stat directory: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        } else if (!S_ISDIR(st.st_mode)) {
            printf("Error: %s is not a directory\n", MESSAGE_DIR);
            exit(EXIT_FAILURE);
        } else if ((st.st_mode & S_IWUSR) == 0) {
            printf("Error: No write permission for %s\n", MESSAGE_DIR);
            exit(EXIT_FAILURE);
        }

        snprintf(full_message, sizeof(full_message), "u;%s;%s;%s", remetente, dest, message);
        snprintf(filename, sizeof(filename), "%s/%s%d", MESSAGE_DIR, MESSAGE_FILE_PREFIX, getpid());
        printf("full_message: %s\n", full_message);
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd == -1) {
            printf("Failed to open file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("writing in filename %s the content: %s\n", filename, full_message);
        if (write(fd, full_message, strlen(full_message)) == -1) {
            printf("Failed to write to file: %s\n", strerror(errno));
            close(fd);
            exit(EXIT_FAILURE);
        }
        close(fd);
        printf("Message saved to: %s\n", filename);
    }

    free(remetente);

    return 0;
}