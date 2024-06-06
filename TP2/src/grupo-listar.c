#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>


void list_group_members(const char *group_name) {
    char command[256];
    snprintf(command, sizeof(command), "getent group %s", group_name);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    if (fgets(line, sizeof(line), fp) != NULL) {
        char *token = strtok(line, ":");
        int field_count = 0;
        while (token != NULL) {
            field_count++;
            if (field_count == 4) {
                // The fourth field contains the list of users
                if (strlen(token) > 0) {
                    printf("Members of group '%s': %s\n", group_name, token);
                } else {
                    printf("Group '%s' has no members.\n", group_name);
                }
                break;
            }
            token = strtok(NULL, ":");
        }
    } else {
        printf("Group '%s' not found.\n", group_name);
    }

    pclose(fp);
    
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <groupname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *groupname = argv[1];

    list_group_members(groupname);

    return 0;
}