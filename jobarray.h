#ifndef JOBARRAY_H
#define JOBARRAY_H
#include<sys/types.h>

typedef short jobtype;
typedef short jobstate;

#define STATE_STOPPED 0
#define STATE_RUNNING 1
#define STATE_DONE 2

#define TYPE_FOREGROUND 0
#define TYPE_BACKGROUND 1

typedef struct {
    int id;
    pid_t pid;
    short st;
    short type;
    char** cmd;
} sh_job;

typedef struct {
    sh_job* array;
    size_t used;
    size_t size;
} jobarray;

sh_job make_job(pid_t pid, jobstate st, jobtype type, char* cmd);
jobarray* ja_init(size_t size);
void ja_pushBack(jobarray *arr, sh_job j);
sh_job ja_get(jobarray *arr, size_t idx);
void ja_remove(jobarray *arr, size_t idx);
int ja_getIndexById(jobarray *arr, int id);
int ja_getIndexByPid(jobarray *arr, pid_t pid);

#endif