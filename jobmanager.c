#include"jobarray.h"
#include"utilities.h"
#include<signal.h>
#include<unistd.h>
#define JOB_INIT_LENGTH 4

static jobarray* jobs;

void init_jobs() {
    jobs = ja_init(JOB_INIT_LENGTH);
}

int record_job_start(pid_t pid, char* cmd, jobtype type, sh_job *job) {
    sh_job newJob = make_job(pid, STATE_RUNNING, type, cmd);
    ja_pushBack(jobs, newJob);

    *job = newJob;
    return 0;
}

int get_job_by_pid(pid_t pid, sh_job *job) {
    int job_idx = ja_getIndexByPid(jobs, pid);

    if (job_idx < 0) {
        return -1;
    }

    *job = ja_get(jobs, job_idx);
    return 0;
}

int get_job_by_id(int id, sh_job *job) {
    int job_idx = ja_getIndexById(jobs, id);

    if (job_idx < 0) {
        return -1;
    }

    *job = ja_get(jobs, job_idx);
    return 0;
}

void record_job_end(pid_t pid) {
    int toRemove = ja_getIndexByPid(jobs, pid);

    if (toRemove < 0) {
        app_error("pid is not found in list of jobs.");
    }

    sh_job job = ja_get(jobs, toRemove);

    if (job.type != TYPE_FOREGROUND) {
        printf("[%d] %d Done %s", job.id, job.pid, *job.cmd);
    }

    ja_remove(jobs, toRemove);
}

int reap_job(pid_t pid, int options) {
    sh_job job;

    if (get_job_by_pid(pid, &job) < 0) {
        return -1;
    }

    reap_child(pid, options);
}

void print_jobs() {
    for (int i = 0; i < jobs->used; i++) {
        sh_job job = ja_get(jobs, i);
        printf("[%d] %d %s %s", job.id, job.pid, job.st == STATE_RUNNING ? "RUNNING" : "STOPPED", *job.cmd);
    }
}

void print_job(sh_job *job) {
    printf("[%d] %d %s %s", job->id, job->pid, job->st == STATE_RUNNING ? "RUNNING" : "STOPPED", *job->cmd);
}