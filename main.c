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

#define MAXARGS 128
#define MAXLINE 256
#define JOB_INIT_LENGTH 4
#define DEBUG
#undef DEBUG

void eval(char *cmdline);
int parseline(char *cbuf, char **argv);
int builtin_command(char **argv);
void print_jobs();
pid_t reap_child(pid_t pid, int options);
int record_job_start(pid_t pid, char* cmd);
void record_job_end(pid_t pid);

jobarray* jobs;

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
    jobs = ja_init(JOB_INIT_LENGTH);

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

int record_job_start(pid_t pid, char* cmd) {
    sh_job newJob = make_job(pid, STATE_RUNNING, cmd);
    ja_pushBack(jobs, newJob);

    #ifdef DEBUG
    printf("pid %d started.\n", pid);
    #endif

    return newJob.id;
}

void record_job_end(pid_t pid) {
    int toRemove = ja_getIndexByPid(jobs, pid);

    if (toRemove < 0) {
        app_error("pid is not found in list of jobs.");
    }

    sh_job job = ja_get(jobs, toRemove);

    printf("[%d] %d Done %s", job.id, job.pid, *job.cmd);

    ja_remove(jobs, toRemove);

    #ifdef DEBUG
    printf("pid %d reaped.\n", pid);
    #endif
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
        int val = strtol(argv[1], &endptr, 10);

        if (endptr == argv[1]) {
            printf("no digits were found.\n");
            return 1;
        }

        int job_idx = ja_getIndexById(jobs, val);

        if (job_idx < 0) {
            printf("Job with id %d: not found.\n", val);
            return 1;
        }

        sh_job job = ja_get(jobs, job_idx);
        
        printf("%d %s", job.pid, *job.cmd);
        pid_t reaped_pid = reap_child(job.pid, 0);

        if (reaped_pid > 0) {
            record_job_end(reaped_pid);
        }
        
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

void print_jobs() {
    for (int i = 0; i < jobs->used; i++) {
        sh_job job = ja_get(jobs, i);
        printf("[%d] %d %s %s", job.id, job.pid, job.st == STATE_RUNNING ? "RUNNING" : "STOPPED", *job.cmd);
    }
}