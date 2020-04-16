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

int cmpByreadyTime(const void *a, const void *b);
char schedulingPolicy[512];
int numberOfProcess;
Process* job;

int main(int argc, char* argv[]){

	//set parent's CPU to CPU_0
	AssignCpu(getpid(), 0);
	//set high priority
	WakeUp(getpid());

	//IO
	scanf("%s", schedulingPolicy);
	scanf("%d", &numberOfProcess);
	job = (Process*)malloc(sizeof(Process) * numberOfProcess);
	for (int i = 0; i < numberOfProcess; i++){
		scanf("%s%d%d", job[i].name, &job[i].readyTime, &job[i].execTime);
		job[i].pid = -1; //-1 for not started
		job[i].progress = 0;
	}
	
	//sort jobs by readyTime
	qsort(job, numberOfProcess, sizeof(Process), cmpByreadyTime);
	
	//check policy
	if (strncmp(schedulingPolicy, "FIFO", 32) == 0){
		//init values
		int time = 0;
		int numberOfDone = 0;
		int running = -1;
		int next = 0;
		while(numberOfDone < numberOfProcess){
			if(running == -1){ //nothing is running
				if(time >= job[next].readyTime){ // the next job is ok to run
					running = next; //find a task to run
					job[running].pid = newChild(job[running]); //fork a new child
				}
				else{;} //no job is ready
			}
			else{ //some thing is running
				if(job[running].progress >= job[running].execTime){ //the running job finished
					/*wait here*/
					int status;
					wait(&status);
					next = next + 1; //update the next task
					running = -1;
					numberOfDone++;
					continue;
				}
				else{;} //the running job keep running
			}
			busy();
			time++;
			job[running].progress++;
		}
	}
	else if (strncmp(schedulingPolicy, "SJF", 32) == 0){
		//init values
        int time = 0;
        int numberOfDone = 0;
        int running = -1;
        int next = 0;
        while(numberOfDone < numberOfProcess){
            if(running == -1){ //nothing is running
                if(time >= job[next].readyTime){ // the next job is ok to run
					//find a shortest task to run
					running = next;
					int requirement = job[next].execTime - job[next].progress;
					for(int i = next; i < numberOfProcess && time >= job[i].readyTime; i++){
						if(job[i].execTime - job[i].progress < requirement && job[i].execTime - job[i].progress != 0){
							running = i;
							requirement = job[i].execTime - job[i].progress;
						}
					}
					job[running].pid = newChild(job[running]); //fork a new child
                }
                else{;} //no job is ready
            }
            else{ //some thing is running
                if(job[running].progress >= job[running].execTime){ //the running job finished
					if(running == next){ //update the next task
						while(next < numberOfProcess && job[next].progress >= job[next].execTime){
							next = next + 1;
						}
					}
					/*wait here*/
					int status;
					wait(&status);
                    running = -1;
                    numberOfDone++;
                    continue;
                }
                else{;} //the running job keep running
            }
            busy();
            time++;
            job[running].progress++;
        }
	}


	else if (strncmp(schedulingPolicy, "PSJF", 32) == 0){
		//init values
        int time = 0;
        int numberOfDone = 0;
        int running = -1;
        int next = 0;
		int findAgain = 0;
		int previous;
        while(numberOfDone < numberOfProcess){
            if(running == -1 || findAgain == 1){ //nothing is running
                if(time >= job[next].readyTime || findAgain == 1){ // the next job is ok to run
					//find a shortest task to run
					running = next;
					int requirement = job[next].execTime - job[next].progress;
					for(int i = next; i < numberOfProcess && time >= job[i].readyTime; i++){
						if(job[i].execTime - job[i].progress < requirement && job[i].execTime - job[i].progress != 0){
							running = i;
							requirement = job[i].execTime - job[i].progress;
						}
					}
					if(findAgain == 1 && previous == running){;} //keep running
					else if(job[running].progress == 0){
						if(findAgain == 1) Block(job[previous].pid);
						job[running].pid = newChild(job[running]); //fork a new child
					}
					else{
						if(findAgain == 1) Block(job[previous].pid);
						WakeUp(job[running].pid); //Wake up the child
					}
					findAgain = 0;
				}
                else{;} //no job is ready
            }
            else{ //some thing is running
                if(job[running].progress >= job[running].execTime){ //the running job finished
					if(running == next){ //update the next task
						while(next < numberOfProcess && job[next].progress >= job[next].execTime){
							next = next + 1;
						}
					}
					/*wait here*/
					int status;
					wait(&status);
                    running = -1;
                    numberOfDone++;
                    continue;
                }
                else{ // find the shortest job again 
					findAgain = 1;
					previous = running;
					continue;
				}
			}
            busy();
            time++;
            job[running].progress++;
			previous = running;
        }
	}
	else if (strncmp(schedulingPolicy, "RR", 32) == 0){
		//init values
        int time = 0;
        int numberOfDone = 0;
        int running = -1;
        int next = 0;
		int usedTime = 0;
		int findAgain = 0;
		int previous;
        while(numberOfDone < numberOfProcess){
            if(running == -1 || findAgain == 1){ //nothing is running
                if(time >= job[next].readyTime || findAgain == 1){ // the next job is ok to run
					usedTime = 0;
					int found = 0;
					for(int i = running + 1; i < numberOfProcess && time >= job[i].readyTime; i++){
        	        	if(job[i].execTime - job[i].progress != 0){
            	        	running = i;
                	        found = 1;
							break;
                    	}
                    }
					if(found == 0){
						for(int i = 0; i < numberOfProcess && time >= job[i].readyTime; i++){
	        	            if(job[i].execTime - job[i].progress != 0){
    	        	            running = i;
        	                    found = 1;
								break;
                	   	    }
						}
					}
					if(found == 0){
						fprintf(stderr, "This should not happen...\n");
					}
					//wake it up or fork it
					if(findAgain == 1 && previous == running){;} //keep running
					else if(job[running].progress == 0){
						if(findAgain == 1) Block(job[previous].pid);
						job[running].pid = newChild(job[running]); //fork a new child
					}
					else{
						if(findAgain == 1) Block(job[previous].pid);
						WakeUp(job[running].pid); //Wake up the child
					}
					findAgain = 0;
                }
                else{;} //no job is ready
            }
            else{ //some thing is running
                if(job[running].progress >= job[running].execTime){ //the running job finished
					if(running == next){ //update the next task
						while(next < numberOfProcess && job[next].progress >= job[next].execTime){
							next = next + 1;
						}
					}
					/*wait here*/
					int status;
					wait(&status);
                    running = -1;
                    numberOfDone++;
                    continue;
                }
                else{ 
            		if(usedTime == 500){//context switch
						findAgain = 1;
						previous = running;
						continue;
					}
					else{;} // the running job keep running
				}
			}
            busy();
            time++;
			usedTime++;
            job[running].progress++;
			previous = running;
		}
	}
	else{
		fprintf(stderr, "Scheduling policy not found.\n");
		exit(1);
	}
	return 0;
}

int cmpByreadyTime(const void *a, const void *b){
	Process* A = (Process*)a;
	Process* B = (Process*)b;
	if (A->readyTime < B->readyTime) return -1;
	else if (A->readyTime > B->readyTime) return 1;
	else return 0;
}

