#include<errno.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<signal.h>
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

    DEBUG_LOG

    if (reaped_pid < 0) {
        if (errno != 10) {
            unix_error("waitfg: waitpid error");
        }

        #ifdef DEBUG
        printf("no processes to reap.\n");
        #endif
    }

    DEBUG_LOG

    return reaped_pid;
}

handler_t *register_signal_handler(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    if (sigaction(signum, &action, &old_action) < 0) {
        unix_error("signal registration error");
    }

    return old_action.sa_handler;
}