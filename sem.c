/*
 * sem.c 10/12/2021
 * P3 CMPSCI 4760 Prof. Bhatia
 * 
 * This code has been modified from the site: https://stevens.netmeister.org/631/semdemo.c
 */

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "sem.h"

int initsem(key_t key) {
	int i;
	union semun arg;
	struct semid_ds buf;
	struct sembuf sb;
	int semid;

	arg.buf = &buf;

	semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
	if (semid >= 0) {       /* we got it first */
		sb.sem_num = 0; /* operate on the first semaphore in the set */
		sb.sem_op  = 1; /* "free" the semaphore */
		sb.sem_flg = 0; /* no flags are set */

		// Free the semaphore
		printf("removing sem\n");
		if (semop(semid, &sb, 1) == -1) {
			int e = errno;
			if (semctl(semid, 0, IPC_RMID) < 0) {
				perror("sem: Error: semctl rm ");
			}
			errno = e;
			return -1;
		}

	} else if (errno == EEXIST) { /* someone else got it first */
		int ready = 0;
		int e;

		semid = semget(key, 1, 0); /* get the id */
		if (semid < 0)
			return semid;

		/* ...would see sem_otime as '0', sleep for a second, and
		 * attempt again up to MAX_TRIES to give the
		 * semaphore-creating process above a chance to free it.
		 * Without this check, the race condition above could
		 * otherwise lead to a deadlock.
		 */
		for(i = 0; i < MAX_RETRIES && !ready; i++) {
			if ((e = semctl(semid, 0, IPC_STAT, arg)) < 0) {
				perror("sem: Error: semctl stat ");
				return -1;
			}

			if (arg.buf->sem_otime != 0) {
				ready = 1;
			} else {
				sleep(1);
			}
		}

		if (!ready) {
			errno = ETIME;
			return -1;
		}
	} else {
		return semid;
	}

	return semid;
}
