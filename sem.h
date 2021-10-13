/*
 * sem.h by Pascal Odijk 10/12/2021
 * P3 CMPSCI 4760 Prof. Bhatia
 *
 * This is the header file for sem.c.
 */

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifndef SEM_H

#define MAX_RETRIES 10

union semun {
        int val;
        struct semid_ds *buf;
        ushort *array;
};

int initsem(key_t key);

#endif

