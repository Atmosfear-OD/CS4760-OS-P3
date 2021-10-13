/*
 * testsim.c by Pascal Odijk 10/3/2021
 * P3 CS4760 Prof. Bhatia
 *
 * This file retrieves the shared memory initiated in runsim and then calls logmsg to print the messages the number of times stated in the repeat factor. The program also sleeps in between
 * each message loop as stated in the sleep time. Semaphores are utlizied to handle the multiple processes created by fork in runsim.  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "config.h"
#include "sem.h"

int shmid;
struct nLicenses *shm;

int main(int argc, char* argv[]) {
	signal(SIGINT, signalHandler);
	
	int sleepT = atoi(argv[1]);
	int repeatF = atoi(argv[2]);
	int i = atoi(argv[3]);
	key_t key = 5678;
	
	// Semaphore info
	key_t semKey;
	int semid;
	struct sembuf sb;
	sb.sem_num = 0;		// Only operate on the first element in the semaphore set
	sb.sem_op = -1;		// Set to allocate resource
	sb.sem_flg = SEM_UNDO;  // Increment adjust on exit value by abs(sem_op)
	
	// Open semaphore
	if((semKey = ftok("sem.c", 'J')) == -1) {
		perror("testsim: Error: ftok ");
		exit(1);
	}

	if((semid = initsem(semKey)) == -1) {
		perror("testsim: Error: initsem ");
		exit(1);
	}
	
	// Get shared memory id from parent
	if((shmid = shmget(key, sizeof(struct nLicenses) * 2, 0666)) < 0) {
		perror("testsim: Error: shmget ");
		exit(1);
	}

	// Attach shared memory to child
	if((shm = (struct nLicenses *)shmat(shmid, NULL, 0)) == (struct nLicenses *) -1) {
		perror("testsim: Error: shmat ");
		exit(1);
	}

	int c, j, k, n = shm->childProc;

	for(c = 0 ; c < repeatF ; c++) {
		// Will block if another process is in the critical section
		if(semop(semid, &sb, 1) == -1) {
			perror("testsim: Error: semop ");
			exit(1);
		}

		printf("PID %d acquired semaphore\n", getpid());

		// Enter critical section -- 
		pid_t id = getpid();
		char pid[50];
		char num[50];
	
		sprintf(pid, "%d", id);
		sprintf(num, "%d", (c + 1));

		printf("Printing msg to file: %s %s of %s\n", pid, num, argv[2]);

		// Print to output fie using logmsg
		logmsg(pid, num, argv[2]);

		//sleep(sleepT);
		
		// Exit critical section and release semaphore -- 
		sb.sem_op = 1;
		if(semop(semid, &sb, 1) == -1) {
			perror("testsim: Error: semop ");
			exit(1);
		}

		// Sleep before looping again
		sleep(sleepT);
	}
	
	// About to alter license object -- obtaining semaphore
	if(semop(semid, &sb, 1) == -1) {
        	perror("testsim: Error: semop ");
                exit(1);
        }

	addtolicenses(1);
	
	// Finished altering license object -- releasing semaphore
        sb.sem_op = 1;
        if(semop(semid, &sb, 1) == -1) {
        	perror("testsim: Error: semop ");
                exit(1);
        }
	
	killProcesses();
	return 0;
}

void signalHandler() {
	pid_t id = getpid();
	killProcesses();
	killpg(id, SIGINT);
	exit(1);
}

void killProcesses() {
	shmdt(shm);
}
