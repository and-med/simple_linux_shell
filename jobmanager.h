#ifndef JOBMANAGER_H
#define JOBMANAGER_H
#include<sys/types.h>
#include"jobarray.h"

void init_jobs();
int record_job_start(pid_t pid, char* cmd, jobtype type, sh_job *job);
void record_job_end(pid_t pid);
int get_job_by_pid(pid_t pid, sh_job *job);
int get_job_by_id(int id, sh_job *job);
void print_job(sh_job *job);
void print_jobs();

#endif