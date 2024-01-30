#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<signal.h>
#include"utilities.h"
#include"jobarray.h"
#include"jobmanager.h"

#define MAXARGS 128
#define MAXLINE 256
#define DEBUG
#undef DEBUG

void eval(char *cmdline);
int parseline(char *cbuf, char **argv);
int builtin_command(char **argv);

void sigint_handler(int sig) {
    printf("SIGINT received!");
}

void sigtstp(int sig) {
    printf("SIGTSTP received!");
    fflush(stdout);
}

int main()
{
    char cmdline[MAXLINE];
    init_jobs();

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        unix_error("error registering SIGINT handler");
    }

    if (signal(SIGTSTP, sigint_handler) == SIG_ERR) {
        unix_error("error registering SIGSTOP handler");
    }

    while(1) {
        printf("andrii> ");
        Fgets(cmdline, MAXLINE, stdin);

        if (feof(stdin)) {
            exit(0);
        }

        pid_t reaped_pid;
        while ((reaped_pid = reap_child(-1, WNOHANG)) > 0) {
            record_job_end(reaped_pid);
        }
        eval(cmdline);
    }
}

void eval(char *cmdline) {
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL) {
        return;
    }

    if (!builtin_command(argv)) {
        if ((pid = Fork()) == 0) {
            pid = getpid();
            setpgid(pid,pid);
            
            if (execve(argv[0], argv, __environ) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        if (!bg) {
            reap_child(pid, 0);
        }
        else {
            int id = record_job_start(pid, cmdline);
            printf("[%d] %d %s", id, pid, cmdline);
        }
    }
    return;
}

int builtin_command(char **argv) {
    if (strcmp(argv[0], "quit") == 0)
        exit(0);
    if (strcmp(argv[0], "&") == 0)
        return 1;
    if (strcmp(argv[0], "jobs") == 0) {
        print_jobs();
        return 1;
    }
    if (strcmp(argv[0], "fg") == 0) {
        if (argv[1] == NULL) {
            printf("Invalid syntax: expected job id.\n");
            return 1;
        }

        char* endptr;
        int id = strtol(argv[1], &endptr, 10);

        if (endptr == argv[1]) {
            printf("no digits were found.\n");
            return 1;
        }

        move_job_to_foreground(id);
        
        return 1;
    }

    return 0;
}

int parseline(char *buf, char **argv) {
    char *delim;
    int argc;
    int bg;

    // Replace trailing '\n' with whitespace
    buf[strlen(buf) - 1] = ' ';
    // Ignore leading space
    while (*buf && (*buf == ' '))
        buf++;

    argc = 0;
    while((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        // Ignore spaces
        while (*buf && (*buf == ' '))
            buf++;
    }
    argv[argc] = NULL;

    // Ignore blank lines
    if (argc == 0)
        return 1;

    if ((bg = (*argv[argc - 1] == '&')) != 0) {
        argv[--argc] = NULL;
    }

    return bg;
}