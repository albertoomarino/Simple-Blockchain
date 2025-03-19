#include "users.h"

int SO_USERS_NUM, SO_NODES_NUM, SO_SIM_SEC, SO_BUDGET_INIT, SO_MIN_TRANS_GEN_NSEC, SO_MAX_TRANS_GEN_NSEC,
		SO_REWARD, SO_RETRY, SO_TP_SIZE, SO_MIN_TRANS_PROC_NSEC, SO_MAX_TRANS_PROC_NSEC;
int pos, attempt, balance, index_blocks, semid_ip, msgqid_user, r_node, *block;
struct transaction trans;
struct p_info *ip;
struct block *ledger;
struct message mymsg;
struct sembuf sops;

/* Store macros from file */
void read_macro(char *argv)
{
	char macro[30];
	FILE *apriFile;
	int val;

	/* Open macro.txt file */
	if ((apriFile = fopen(argv, "r")) == NULL)
	{
		fprintf(stderr, "ERROR fopen ");
		TEST_ERROR;
	}

	/* Read values from macro.txt */
	while (fscanf(apriFile, "%s"
													"%d",
								macro, &val) != EOF)
	{
		if (strcmp(macro, "SO_USERS_NUM") == 0)
			SO_USERS_NUM = val;
		else if (strcmp(macro, "SO_NODES_NUM") == 0)
			SO_NODES_NUM = val;
		else if (strcmp(macro, "SO_BUDGET_INIT") == 0)
			SO_BUDGET_INIT = val;
		else if (strcmp(macro, "SO_REWARD") == 0)
			SO_REWARD = val;
		else if (strcmp(macro, "SO_MIN_TRANS_GEN_NSEC") == 0)
			SO_MIN_TRANS_GEN_NSEC = val;
		else if (strcmp(macro, "SO_MAX_TRANS_GEN_NSEC") == 0)
			SO_MAX_TRANS_GEN_NSEC = val;
		else if (strcmp(macro, "SO_RETRY") == 0)
			SO_RETRY = val;
		else if (strcmp(macro, "SO_TP_SIZE") == 0)
			SO_TP_SIZE = val;
		else if (strcmp(macro, "SO_MIN_TRANS_PROC_NSEC") == 0)
			SO_MIN_TRANS_PROC_NSEC = val;
		else if (strcmp(macro, "SO_MAX_TRANS_PROC_NSEC") == 0)
			SO_MAX_TRANS_PROC_NSEC = val;
		else if (strcmp(macro, "SO_SIM_SEC") == 0)
			SO_SIM_SEC = val;
	}

	/* Close macro.txt file */
	if (fclose(apriFile))
	{
		fprintf(stderr, "ERROR fclose ");
		TEST_ERROR;
	}
}

/* Wait for the creation of processes */
void wait_for_zero()
{
	/* Decrement the semaphore by 1 */
	sops.sem_flg = 0;
	sops.sem_num = 0;
	sops.sem_op = -1;
	if (semop(semid_ip, &sops, 1) == -1)
	{
		fprintf(stderr, "ERROR semop ");
		TEST_ERROR;
	}

	/* Wait for zero */
	sops.sem_flg = 0;
	sops.sem_num = 0;
	sops.sem_op = 0;
	if (semop(semid_ip, &sops, 1) == -1)
	{
		fprintf(stderr, "ERROR semop ");
		TEST_ERROR;
	}
}

/* Calculate the user's balance by reading from the ledger */
void budget_calc()
{
	int i;

	if (block[0] > 0)
	{ /* If the number of blocks written is > 0, proceed */
		while (index_blocks < block[0])
		{ /* Loop through each block written in the ledger */
			for (i = 0; i < SO_BLOCK_SIZE - 1; i++)
			{ /* Loop through all transactions in the block index_blocks */
				if (ledger[index_blocks].array_trans[i].receiver == getpid())
				{																													 /* If the transaction i in the block index_blocks is the receiver, then: */
					balance += ledger[index_blocks].array_trans[i].quantity; /* Add the received amount to the user's balance */
				}
			}
			index_blocks++; /* Update the block index */
		}
	}
}

/* Initialize the transaction to send */
void trans_init()
{
	trans.timestamp = 0;
	trans.sender = getpid();
	trans.receiver = 0;
	trans.quantity = 0;
	trans.reward = 0;
}

/* Send transaction */
void trans_send()
{
	if (balance >= 2)
	{																																/* Set the transaction */
		set_users_timestamp();																				/* Set the transaction timestamp */
		set_users_receiver();																					/* Another user process receiver to send the money to */
		r_node = ip[(rand() % SO_NODES_NUM) + SO_USERS_NUM].proc_pid; /* Another node process to send the transaction for processing */
		set_users_quantity();																					/* Integer between 2 and the user's balance */

		/* Set msgqid_user to the message queue address to send the transaction */
		if ((msgqid_user = msgget(r_node, 0)) == -1)
		{
			fprintf(stderr, "ERROR msgget ");
			TEST_ERROR;
		}

		mymsg.mtype = 1;		 /* Message type > 0 */
		mymsg.mtext = trans; /* Message content */
		if (msgsnd(msgqid_user, &mymsg, sizeof(struct transaction), IPC_NOWAIT) == -1)
		{ /* Msgsnd failed */
			if (errno != EAGAIN)
			{ /* Error */
				fprintf(stderr, "ERROR msgsnd ");
				TEST_ERROR;
			}
			/* If errno == EAGAIN, the queue is full */
		}
		else
		{ /* Msgsnd succeeded */
			balance -= trans.quantity;
			one_sec_waited_users(); /* Wait for a random interval between SO_MAX_TRANS_GEN_NSEC and SO_MIN_TRANS_GEN_NSEC */
			attempt = 0;
		}
	}
	else
	{ /* If balance < 2, the process does not send any transaction */
		attempt++;
	}
}

/* Wait for a time interval between SO_MIN_TRANS_GEN_NSEC and SO_MAX_TRANS_GEN_NSEC */
void one_sec_waited_users()
{
	struct timespec time;
	int random;

	/* Random number between SO_MIN_TRANS_GEN_NSEC and SO_MAX_TRANS_GEN_NSEC */
	random = (rand() % (SO_MAX_TRANS_GEN_NSEC - SO_MIN_TRANS_GEN_NSEC + 1)) + SO_MIN_TRANS_GEN_NSEC;
	time.tv_sec = random / NSEC;
	time.tv_nsec = random % NSEC;
	if (clock_nanosleep(CLOCK_MONOTONIC, 0, &time, NULL) == -1)
	{
		fprintf(stderr, "ERROR clock_nanosleep ");
		TEST_ERROR;
	}
}

/* Set the transaction timestamp */
void set_users_timestamp()
{
	struct timespec tp;
	int control;

	if ((control = clock_gettime(CLOCK_REALTIME, &tp)) == -1)
	{
		fprintf(stderr, "ERROR clock_gettime ");
		TEST_ERROR;
	}

	trans.timestamp = tp.tv_sec * (NSEC) + tp.tv_nsec; /* Current seconds in nanoseconds + nanoseconds in the current second */
}

/* Set the transaction receiver */
void set_users_receiver()
{
	int pos_random;

	pos_random = rand() % SO_USERS_NUM; /* Random number between 0 and SO_USERS_NUM */

	/* Loop until finding a random position different from itself and not terminated */
	while (pos_random == pos || ip[pos_random].term == true)
	{
		pos_random = rand() % SO_USERS_NUM;
	}

	trans.receiver = ip[pos_random].proc_pid; /* Assign the receiver */
}

/* Set the amount of money to send to the user and the reward to the node */
void set_users_quantity()
{
	int random;

	random = (rand() % (balance - 1)) + 2; /* Random number between 2 and balance. rand() % (max - min + 1) + min */

	if (((SO_REWARD * random) / 100) < 1)
	{ /* If the reward is less than 1, assign 1 */
		trans.reward = 1;
	}
	else
	{
		trans.reward = (SO_REWARD * random) / 100; /* Assign the reward percentage */
	}

	trans.quantity = random - trans.reward; /* Assign the quantity to send to the user */
}

/* Handle signals */
void handle_signal(int sig)
{
	switch (sig)
	{
	case SIGTERM:
		shmdt(ip);		 /* Detach ip from shared memory */
		shmdt(ledger); /* Detach ledger from shared memory */
		exit(EXIT_SUCCESS);

	case SIGALRM: /* At the start of the simulation, after rand() % SO_SIM_SEC + 1 seconds, SIGALRM is captured and a transaction is sent */
		budget_calc();
		trans_send();
		break;

	case SIGINT: /* CTRL-C signal goes to all calling process */
		pause();	 /* Process is blocked until it receives a signal */
		break;

	default:
		break;
	}
}
