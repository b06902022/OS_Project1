#include <sys/types.h>

typedef struct process{
    char name[32];
    int readyTime;
    int execTime;
    int progress;
    pid_t pid;
}Process;

int AssignCpu(pid_t pid, int core);

int WakeUp(pid_t pid);

int newChild(Process p);

void busy();
