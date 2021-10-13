/*
 * runsim.c by Pascal Odijk 10/3/2021
 * P3 CMPSCI 4760 Prof. Bhatia
 *
 * This program takes in the command line arg and checks if it is of valid type and value. Shared memory is then allocated with the number of licenses stated by user arg.
 * The invocation method is:
 * 	./runsim n < testing.data -- where n is an integer number of licenses
 * 	./runsim -t sec n < testing.data -- where -t sec is an integer program runtime
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "config.h"
#include "sem.h"

#define BUFFERSIZE 100

// Shared memory globals
int shmid;
struct nLicenses *shm;
int semid;

int main(int argc, char *argv[]) {

	int numLicenses;
	int childCount = 0;
	int terminationTime = 100;  // Runtime for program -- default is 100 seconds

	signal(SIGINT, signalHandler);
	signal(SIGINT, sigintHandler);

	// Semaphore info
        key_t semKey;
        struct sembuf sb;
        sb.sem_num = 0;         // Only operate on the first element in the semaphore set
        sb.sem_op = -1;         // Set to allocate resource
        sb.sem_flg = SEM_UNDO;  // Increment adjust on exit value by abs(sem_op)

        // Initialize semaphore
        if((semKey = ftok("sem.c", 'J')) == -1) {
                perror("testsim: Error: ftok ");
                exit(1);
        }

	if((semid = initsem(semKey)) == -1) {
                perror("testsim: Error: initsem ");
                exit(1);
        }

	// Error checking for arguments
	if(argc > 4) {	// Check for too many args
		printf("runsim: Error: Too many arguments\n");
		printf("Usage: ./runsim n < testing.data -- where n is an integer greater than or equal to 1\n");
		printf("or     ./runsim -t sec n < testing.data -- where -t is the opt and sec is an integer greater than or equal to 0\n");

		return 0;
	} else if(argc == 1) {	// Check for no arg
		printf("runsim: Error: No argument given\n");
		printf("Usage: ./runsim n < testing.data -- where n is an integer greater than or equal to 1\n");
		printf("or     ./runsim -t sec n < testing.data -- where -t is the opt and sec is an integer greater than or equal to 0\n");

		return 0;
	}

	if(argc == 2 && (strcmp(argv[1], "-t") != 0)) {
		if(strspn(argv[1], "0123456789") == strlen(argv[1])) {  // Check is n is a number
			numLicenses = atoi(argv[1]);
			if(numLicenses <= 0) {
				printf("runsim: Error: n must be an integer greater than or equal to 1\n");
				printf("Usage: ./runsim n < testing.data -- where n is an integer greater than or equal to 1\n");
				printf("or     ./runsim -t sec n < testing.data -- where -t is the opt and sec is an integer greater than or equal to 0\n");

				return 0;
			}
		} else {
			printf("runsim: Error: Invalid argument: %s\n", argv[1]);
			printf("Usage: ./runsim n < testing.data -- where n is an integer greater than or equal to 1\n");
			printf("or     ./runsim -t sec n < testing.data -- where -t is the opt and sec is an integer greater than or equal to 0\n");

			return 0;
		}	
	} else if((argc == 3) && (strcmp(argv[1], "-t") == 0)) {
		printf("runsim: Error: -t specified but no value given\n");
		printf("Usage: ./runsim n < testing.data -- where n is an integer greater than or equal to 1\n");
                printf("or     ./runsim -t sec n < testing.data -- where -t is the opt and sec is an integer greater than or equal to 0\n");

		return 0;
	} else if(argc == 4) {
		if(strspn(argv[2], "0123456789") == strlen(argv[2])) { 
			numLicenses = atoi(argv[3]);
			terminationTime = atoi(argv[2]);
			if(terminationTime < 0) {
				printf("runsim: Error: -t sec must be an integer greater than or equal to 0\n");
                        	printf("Usage: ./runsim n < testing.data -- where n is an integer greater than or equal to 1\n");
                        	printf("or     ./runsim -t sec n < testing.data -- where -t is the opt and sec is an integer greater than or equal to 0\n");

                        	return 0;	
			}
		} else {
			printf("runsim: Error: -t sec must be an integer greater than or equal to 0\n");
                        printf("Usage: ./runsim n < testing.data -- where n is an integer greater than or equal to 1\n");
                        printf("or     ./runsim -t sec n < testing.data -- where -t is the opt and sec is an integer greater than or equal to 0\n");

                        return 0;	
		}
	}

	// Implement shared memory
	key_t key = 5678;
	
	// Create shared memory id
	if((shmid = shmget(key, sizeof(struct nLicenses) * 2, IPC_CREAT | 0666)) < 0) {
		perror("runsim: Error: shmget ");
		exit(1);
	}

	// Attach shared memory
	if((shm = (struct nLicenses *)shmat(shmid, NULL, 0)) == (struct nLicenses *) -1) {
		perror("runsim: Error: shmat ");
		exit(1);
	}

	// Init data to shared memory
	shm->availLicenses = numLicenses;
	shm->processes++;
	initlicense();

	// Get strings from stdin/file redirection
	char buffer[BUFFERSIZE];
	char lines[BUFFERSIZE][BUFFERSIZE];
	int i = 0, j = 0;
	char progName[50] = "./";
	char a2[50], a3[50];

	while(fgets(buffer, BUFFERSIZE, stdin) != NULL) {
		strcpy(lines[childCount], buffer);
		childCount++;
	}
	
	shm->childProc = childCount;
	int runningProcesses = 0;
	int index = 0;
	pid_t pid, child[childCount];

	while(terminationTime > 0) {
		if(runningProcesses < 20) {
			while(getlicense() == 1) {
				if(numLicenses == 1) {
					terminationTime--;
					sleep(1);

					if(terminationTime < 0) {
						printf("runsim: Error: Did not complete all processes before alloted time limit -- terminating remaining child processes\n");
						signalHandler();
						exit(1);
					}		
				}
			}	

			if(index < childCount){
        			// Get program name from lines array
        			for(i ; lines[index][i] != ' ' ; i++) {
               				progName[i + 2] = lines[index][i];
       				}			

        			i++;
        			// Get sleep time
        			for(i ; lines[index][i] != ' ' ; i++) {
                			a2[j] = lines[index][i];
                			j++;
        			}	

        			j = 0;
        			i++;

        			// Get repeat factor
       			 	for(i ; i < strlen(lines[index])-1 ; i++) {
               				a3[j] = lines[index][i];
                			j++;
        			}
				i = 0;
				j = 0;
				
				// About to alter license object -- obtaining semaphore
        			if(semop(semid, &sb, 1) == -1) {
                			perror("runsim: Error: semop ");
                			exit(1);
        			}			

        			removelicenses(1);

        			// Finished altering license object -- releasing semaphore
        			sb.sem_op = 1;
        			if(semop(semid, &sb, 1) == -1) {
                			perror("testsim: Error: semop ");
                			exit(1);
			        }
	
				pid = fork();
				child[index] = pid;
				runningProcesses++;
				shm->processes++;
				printf("running proc: %i\n", shm->processes);
				index++;
				
			}		

			if(pid == -1) {
				printf("runsim: Error: Fork error -- terminating program\n");
				killProcesses();
				exit(1);
			} else if(pid == 0) {
				char ch[50];
				sprintf(ch, "%d", index);
				//execl(PATH, argv[0], argv[1], argv[2]);
				printf("About to call testsim with arg: %s %s %s %s\n", progName, a2, a3, ch);
				execl(progName, "testsim", a2, a3, ch,(char *)NULL);
				perror("runsim: Error: ");
			}	
		} else {
			printf("runsim: Error: Too many processes running -- killing all then terminating\n");
			signalHandler();
			exit(1);
		}			
		
		terminationTime--;
		if(terminationTime == 0) {
			printf("runsim: Error: Did not complete all processes before alloted time limit -- terminating remaining child processes\n");
			signalHandler();
			exit(1);
		}
			
		shm->processes--;	
		int check = childCheck(child, childCount);
		if(check == 1) {
			break;
		}

		sleep(1);
	}
	
	// Check if there are any remaining child processes runnning
	if((wait(NULL) > 0) && (shm->childProc != 0)) {
		printf("runsim: Error: Did not complete all processes before alloted time limit -- terminating remaining child processes\n");
		signalHandler();
	} else {
		printf("Completed all %d processes successfully -- terminating program\n", childCount);
		killProcesses();
	}	
	
	// Remove semaphore
	semctl(semid, 0, IPC_RMID);	

	return 0;
}

// Check if any children processes are still runnning
int childCheck(pid_t child[], int size) {
	int c, status;

	for(c = 0 ; c < size ; c++) {
		pid_t wid;
		wid = waitpid(child[c], &status, WNOHANG);

		if(wid != 0) {
			child[c] = 0;
		}
	}

	for(c = 0 ; c < size ; c++) {
		if(child[c] == 0) {
			continue;
		} else {
			return 0;
		}
	}

	return 1;
}

void signalHandler() {	
	pid_t id = getpgrp();
	killProcesses();
	killpg(id, SIGINT);
	exit(0);
}

// Catch if user ends program by CTRL+C
void sigintHandler(int sig_num) {
	signal(SIGINT, sigintHandler);
	fflush(stdout);
	signalHandler();
}

// Terminate all processes
void killProcesses() {
	logmsg("terminate", "", "");
	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);
	shmdt(shm);
}
