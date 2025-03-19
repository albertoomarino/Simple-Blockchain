#include "nodes.h"

int SO_USERS_NUM, SO_NODES_NUM, SO_SIM_SEC, SO_BUDGET_INIT, SO_MIN_TRANS_GEN_NSEC, SO_MAX_TRANS_GEN_NSEC,
		SO_REWARD, SO_RETRY, SO_TP_SIZE, SO_MIN_TRANS_PROC_NSEC, SO_MAX_TRANS_PROC_NSEC;
int msgqid_node, semid_led, semid_ip, quantity_reward, j;
bool isDeclared;
struct sembuf sops;
struct block *ledger;
struct transaction tr_reward;
struct block curr_block;
struct msqid_ds buf;
bool isDeclared;

/* Store macro values from file */
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

	/* Read values from macro.txt file */
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
		{
			isDeclared = true;
			SO_MIN_TRANS_PROC_NSEC = val;
		}
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
	/* Decrease the semaphore by 1 */
	sops.sem_num = 0;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	if (semop(semid_ip, &sops, 1) == -1)
	{
		fprintf(stderr, "ERROR semop ");
		TEST_ERROR;
	}

	/* Wait for zero */
	sops.sem_num = 0;
	sops.sem_op = 0;
	sops.sem_flg = 0;
	if (semop(semid_ip, &sops, 1) == -1)
	{
		fprintf(stderr, "ERROR semop ");
		TEST_ERROR;
	}
}

/* Reserve binary semaphore */
int reserveSem(int semid)
{
	sops.sem_num = 0;
	sops.sem_op = -1; /* Decrease the semaphore by 1 and reserve it */
	sops.sem_flg = 0;

	return semop(semid, &sops, 1);
}

/* Release binary semaphore */
int releaseSem(int semid)
{
	sops.sem_num = 0;
	sops.sem_op = 1; /* Increase the semaphore by 1 and release it */
	sops.sem_flg = 0;

	return semop(semid, &sops, 1);
}

/* Wait for a time interval between SO_MIN_TRANS_PROC_NSEC and SO_MAX_TRANS_PROC_NSEC */
void one_sec_waited_nodes()
{
	if (isDeclared)
	{ /* Check if SO_MIN_TRANS_PROC_NSEC and SO_MAX_TRANS_PROC_NSEC are present in macro.txt file */
		struct timespec time;
		int time_range;

		time_range = (rand() % (SO_MAX_TRANS_PROC_NSEC - SO_MIN_TRANS_PROC_NSEC + 1)) + SO_MIN_TRANS_PROC_NSEC;
		time.tv_sec = time_range / NSEC;
		time.tv_nsec = time_range % NSEC;
		if (clock_nanosleep(CLOCK_MONOTONIC, 0, &time, NULL) == -1)
		{
			fprintf(stderr, "ERROR clock_nanosleep ");
			TEST_ERROR;
		}
	}
}

/* Add reward transaction */
void add_reward_trans()
{
	set_timestamp();
	tr_reward.sender = NODE_SENDER;
	tr_reward.receiver = getpid();
	tr_reward.quantity = quantity_reward;
	tr_reward.reward = 0;
	curr_block.array_trans[j] = tr_reward; /* Adds the reward transaction at position j of the block */
}

/* Set the timestamp for the reward transaction */
void set_timestamp()
{
	struct timespec tp;
	int control;

	if ((control = clock_gettime(CLOCK_REALTIME, &tp)) == -1)
	{
		fprintf(stderr, "ERROR clock_gettime ");
		TEST_ERROR;
	}

	tr_reward.timestamp = tp.tv_sec * (NSEC) + tp.tv_nsec; /* Current seconds in nanoseconds + current second's nanoseconds */
}

/* Signal handling */
void handle_signal(int sig)
{
	switch (sig)
	{
	case SIGTERM:
		if ((msgctl(msgqid_node, IPC_STAT, &buf)) == -1)
		{
			fprintf(stderr, "ERROR msgctl ");
			TEST_ERROR;
		}
		msgctl(msgqid_node, IPC_RMID, NULL);
		shmdt(ledger);
		exit(EXIT_SUCCESS);

	case SIGINT: /* CTRL-C signal affects all caller processes */
		pause();	 /* The process is blocked until it receives a signal */
		break;

	default:
		break;
	}
}
