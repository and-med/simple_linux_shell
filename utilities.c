#include<errno.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include"utilities.h"

void unix_error(char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

void app_error(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(0);
}

char* Fgets(char *s, int size, FILE* stream) {
    char* response = fgets(s, size, stream);

    if (response == NULL && ferror(stream)) {
        app_error("fgets error");
    }

    return response;
}

pid_t Fork() {
    pid_t child_id = fork();

    if (child_id < 0) {
        unix_error("fork error");
    }

    return child_id;
}

pid_t reap_child(pid_t pid, int options) {
    int status;
    pid_t reaped_pid = waitpid(pid, &status, options);

    if (reaped_pid < 0) {
        if (errno != 10) {
            unix_error("waitfg: waitpid error");
        }

        #ifdef DEBUG
        printf("no processes to reap.\n");
        #endif
    }

    return reaped_pid;
}