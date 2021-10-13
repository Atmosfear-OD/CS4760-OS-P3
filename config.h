/*
 * config.h by Pascal Odijk 10/3/2021
 * P3 CS4760 Prof. Bhatia
 *
 * Header file for runsim and testsim -- contains function prototypes for license obj.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/mman.h>

#define SEM_NAME "/semName"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INIT_VALUE 1

extern int shmid;
extern struct nLicenses *shm;

struct nLicenses {
	int availLicenses;	// Total number of licenses
	int childProc;		// Number of child processes
	int processes;		// Number of processes currently running
	int block; 		// Flag for if all available liceses are being used
};

void addTime(FILE *ofptr);
int childCheck(pid_t child[], int size);
void signalHandler();
void sigintHandler(int sig_num);
void killProcesses();

extern int getlicense();
extern int returnlicense();
extern int initlicense();
extern void addtolicenses(int n);
extern void removelicenses(int n);
extern void logmsg(char* pid, char *c, char *repeat);

#endif
