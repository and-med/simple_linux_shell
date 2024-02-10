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

void eval(char *cmdline);
int parseline(char *cbuf, char **argv);
int builtin_command(char **argv);
int handle_foreground_job(sh_job job);

void sigint_handler(int sig) {
    printf("\n");
}

void sigtstp_handler(int sig) {
    printf("SIGTSTP received!");
    fflush(stdout);
}

void sigttou_handler(int sig) {
    printf("SIGTTOU received!");
}

void sigchild_handler(int sig) {
    int olderrno = errno;
    sigset_t mask_all, prev_all;
    pid_t pid;

    sigfillset(&mask_all);
    while ((pid = reap_child(-1, WNOHANG)) > 0) {
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        record_job_end(pid);
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }

    errno = olderrno;
}

int main()
{
    char cmdline[MAXLINE];
    init_jobs();

    register_signal_handler(SIGINT, sigint_handler);
    register_signal_handler(SIGTSTP, sigint_handler);
    register_signal_handler(SIGTTOU, sigint_handler);
    register_signal_handler(SIGCHLD, sigchild_handler);


    while(1) {
        printf("andrii> ");
        Fgets(cmdline, MAXLINE, stdin);

        if (feof(stdin)) {
            exit(0);
        }

        DEBUG_LOG
        pid_t reaped_pid;
        while ((reaped_pid = reap_child(-1, WNOHANG)) > 0) {
            record_job_end(reaped_pid);
        }
        DEBUG_LOG
        eval(cmdline);
        DEBUG_LOG
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
        sigset_t mask_all, mask_one, prev_one;
        sigfillset(&mask_all);
        sigemptyset(&mask_one);
        sigaddset(&mask_one, SIGCHLD);

        sigprocmask(SIG_BLOCK, &mask_one, &prev_one);        
        if ((pid = Fork()) == 0) {
            setpgid(0, 0);
            
            sigprocmask(SIG_SETMASK, &prev_one, NULL);
            if (execve(argv[0], argv, __environ) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        sh_job job;
        if (record_job_start(pid, cmdline, bg ? TYPE_BACKGROUND: TYPE_FOREGROUND, &job) < 0) {
            app_error("error recording job start");
        }
        sigprocmask(SIG_SETMASK, &prev_one, NULL);

        if (!bg) {
            handle_foreground_job(job);
        }
        else {
            print_job(&job);
        }
    }
    return;
}

int handle_foreground_job(sh_job job) {
    if (tcsetpgrp(fileno(stdout), job.pid) < 0) {
        unix_error("error setting foreground process to a child");
    }
    pid_t reaped_pid = reap_child(job.pid, 0);
    DEBUG_LOG

    sigset_t mask, prev_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTTOU);

    sigprocmask(SIG_BLOCK, &mask, &prev_mask);
    if (tcsetpgrp(fileno(stdout), getpid()) < 0) {
        unix_error("error setting foreground process back to shell");
    }
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);

    if (reaped_pid > 0) {
        record_job_end(reaped_pid);
    }
    
    return 1;
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

        sh_job job;

        if (get_job_by_id(id, &job) < 0) {
            printf("job with id %d: not found.\n", id);
            return 1;
        }

        printf("%d %s", job.pid, *job.cmd);
        handle_foreground_job(job);
        
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