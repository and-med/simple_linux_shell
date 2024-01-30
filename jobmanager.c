#include"jobarray.h"
#include"utilities.h"
#define JOB_INIT_LENGTH 4

static jobarray* jobs;

void init_jobs() {
    jobs = ja_init(JOB_INIT_LENGTH);
}

int record_job_start(pid_t pid, char* cmd) {
    sh_job newJob = make_job(pid, STATE_RUNNING, cmd);
    ja_pushBack(jobs, newJob);

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
}

// returns result of operation
// 1 - success
// 0 - failure
int move_job_to_foreground(int id) {
    int job_idx = ja_getIndexById(jobs, id);

    if (job_idx < 0) {
        printf("Job with id %d: not found.\n", id);
        return 0;
    }

    sh_job job = ja_get(jobs, job_idx);
    
    printf("%d %s", job.pid, *job.cmd);
    pid_t reaped_pid = reap_child(job.pid, 0);

    if (reaped_pid > 0) {
        record_job_end(reaped_pid);
    }
    
    return 1;
}

void print_jobs() {
    for (int i = 0; i < jobs->used; i++) {
        sh_job job = ja_get(jobs, i);
        printf("[%d] %d %s %s", job.id, job.pid, job.st == STATE_RUNNING ? "RUNNING" : "STOPPED", *job.cmd);
    }
}