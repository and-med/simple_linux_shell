#ifndef JOBMANAGER_H
#define JOBMANAGER_H
#include<sys/types.h>

void init_jobs();
int record_job_start(pid_t pid, char* cmd);
void record_job_end(pid_t pid);
void print_jobs();
int move_job_to_foreground(int id);

#endif