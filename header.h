#ifndef __HEADER_H__
#define __HEADER_H__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>

/* Error Structure Definition */
#define TEST_ERROR if (errno) { fprintf(stderr, "nel file: %s a linea: %d, con pid %d; ERRORE: %d (%s)\n", \
  __FILE__, __LINE__, getpid(), errno, strerror(errno)); exit(EXIT_FAILURE); }

/* Parameters defined at compile time */
#define SO_REGISTRY_SIZE 1000 /* Maximum number of blocks in the ledger */
#define SO_BLOCK_SIZE 10 /* Number of transactions contained in a block */

#define NSEC 1000000000

#define NODE_SENDER -1

#define MAX_PROC 15

/* Shared Memory */
#define SHMID_LEDGER 76438
#define SHMID_BLOCK 46320

/* Traffic lights */
#define SEMKEY_P_INFO 85674
#define SEMKEY_LEDGER 22886

/* Structure of a transaction */
struct transaction {
	long timestamp;
	pid_t sender;
	pid_t receiver;
	int quantity;
	int reward;
};

/* Structure to have information of each process */
struct p_info {
	pid_t proc_pid;
	int proc_balance;
	int pos;
	bool term;
};

/* Structure of a ledger block */
struct block {
	struct transaction array_trans[SO_BLOCK_SIZE]; /* Array of transactions in a single block. A block contains SO_BLOCK_SIZE transactions */
	int id;
};

/* Message structure */
struct message {
	long mtype;
	struct transaction mtext;
};

#endif
