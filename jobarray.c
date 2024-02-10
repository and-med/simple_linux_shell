#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include"utilities.h"
#include"jobarray.h"

#define app_errorf(format, args...) \
    char error_message[100]; \
    sprintf(error_message, format, args); \
    app_error(error_message); \

int currJobId = 1;

int generateNextJobId() {
    return currJobId++;
}

sh_job make_job(pid_t pid, jobstate st, jobtype type, char* cmd) {
    sh_job job;
    job.id = generateNextJobId();
    job.pid = pid;
    job.st = st;
    job.type = type;
    char* cmd_copy = malloc(strlen(cmd) * sizeof(char));
    char** cmd_ptr = malloc(sizeof(char**));
    *cmd_ptr = cmd_copy;
    strcpy(cmd_copy, cmd);
    job.cmd = cmd_ptr;
    return job;
}

jobarray* ja_init(size_t size) {
    jobarray *arr = malloc(sizeof(jobarray));
    arr->array = malloc(size * sizeof(sh_job));
    arr->used = 0;
    arr->size = size;
    return arr;
}

void ja_pushBack(jobarray *arr, sh_job j) {
    if (arr->used == arr->size) {
        size_t newSize = arr->size * 2;
        sh_job *newArray = malloc(newSize * sizeof(sh_job));
        sh_job *oldArray = arr->array;

        memcpy(newArray, oldArray, arr->size * sizeof(sh_job));

        arr->array = newArray;
        arr->size = newSize;

        free(oldArray);
    }

    arr->array[arr->used++] = j;
}

sh_job ja_get(jobarray *arr, size_t idx) {
    if (idx >= arr->size) {
        app_errorf("get: index out of range: %d out of %d", idx, arr->size);
    }

    return arr->array[idx];
}

int ja_getIndexById(jobarray *arr, int id) {
    for (int i = 0; i < arr->used; i++) {
        if (arr->array[i].id == id) {
            return i;
        }
    }

    return -1;
}

int ja_getIndexByPid(jobarray *arr, pid_t pid) {
    for (int i = 0; i < arr->used; i++) {
        if (arr->array[i].pid == pid) {
            return i;
        }
    }

    return -1;
}

void ja_remove(jobarray *arr, size_t idx) {
    if (idx >= arr->size) {
        app_errorf("remove: index out of range: %d out of %d", idx, arr->size);
    }

    char** tofree = arr->array[idx].cmd;
    free(*tofree);
    free(tofree);

    arr->used--;
    for (size_t i = idx; i < arr->used; i++) {
        arr->array[i] = arr->array[i + 1];
    }
}
