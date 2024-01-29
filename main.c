#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"utilities.h"
#include"jobarray.h"

#define MAXARGS 128
#define MAXLINE 256
#define JOB_INIT_LENGTH 4

void eval(char *cmdline);
int parseline(char *cbuf, char **argv);
int builtin_command(char **argv);
void print_jobs();
void reap_child();

jobarray* jobs;
int currJobId = 1;

int generateNextJobId() {
    return currJobId++;
}

int main()
{
    char cmdline[MAXLINE];
    jobs = ja_init(JOB_INIT_LENGTH);

    while(1) {
        printf("andrii> ");
        Fgets(cmdline, MAXLINE, stdin);

        if (feof(stdin)) {
            exit(0);
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

        sh_job newJob;
        newJob.id = generateNextJobId();
        newJob.pid = pid;
        ja_pushBack(jobs, newJob);

        if (!bg) {
            int status;

            pid_t wait_result = waitpid(pid, &status, 0);
            if (wait_result < 0) {
                unix_error("waitfg: waitpid error");
            } else {
                int toRemove = ja_getIndexByPid(jobs, pid);

                if (toRemove < 0) {
                    app_error("pid is not found in list of jobs.");
                }

                printf("pid %d finished.\n", pid);

                ja_remove(jobs, toRemove);
            }
        }
        else {
            printf("%d %s", pid, cmdline);
        }
    }
    return;
}

void reap_child() {
    int status;
    pid_t reaped_pid = waitpid(0, &status, 0);
    if (reaped_pid < 0) {
        unix_error("waitfg: waitpid error");
    } else {
        int toRemove = ja_getIndexByPid(jobs, reaped_pid);

        if (toRemove < 0) {
            app_error("pid is not found in list of jobs.");
        }

        printf("pid %d finished.\n", reaped_pid);

        ja_remove(jobs, toRemove);
    }
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
        printf("[%d] %d\n", jobs->array[i].id, jobs->array[i].pid);
    }
}