#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "process.h"

int AssignCpu(pid_t pid, int core){
    if (core > sizeof(cpu_set_t)) {
        fprintf(stderr, "Core index too large.\n");
        return -1;
    }
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);
    int tmp = sched_setaffinity(pid, sizeof(mask), &mask);
    if(tmp < 0) {
        fprintf(stderr, "Failed to sched_setaffinity().\n");
        exit(1);
    }
    return 0;
}
int WakeUp(pid_t pid){
    struct sched_param param;
    param.sched_priority = 0;
    int tmp = sched_setscheduler(pid, SCHED_OTHER, &param); //set CHED_OTHER's priority to 0
    if (tmp < 0) {
        fprintf(stderr, "Failed to sched_setscheduler().\n");
        return -1;
    }
    return tmp;
}

int newChild(Process p){
    pid_t tmp = fork();
    if(tmp < 0){
        fprintf(stderr, "Failed to fork().\n");
    }
    else if(tmp > 0){ //parent
        //set high priority
        WakeUp(tmp);
        return tmp;
    }
    else{ //child
        //set child's CPU to CPU_1
        p.pid = getpid();
        printf("%s %d\n", p.name, p.pid);
        AssignCpu(p.pid, 1);
        struct timespec start, end;
        clock_gettime(CLOCK_REALTIME, &start);
        //child starts to run
        for(int epoch = 0; epoch < p.execTime; epoch++){
            busy();
        }
        clock_gettime(CLOCK_REALTIME, &end);
        syscall(345, getpid(), start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec);
        exit(0);
    }
    fprintf(stderr, "This should not happen...\n");
    return -1;
}

void busy(){
    volatile unsigned long i;
    for(i = 0; i<1000000UL; i++);
    return;
}

