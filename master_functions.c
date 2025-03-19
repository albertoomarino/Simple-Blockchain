#include "master.h"

int SO_USERS_NUM, SO_NODES_NUM, SO_SIM_SEC, SO_BUDGET_INIT;
int users_term, shmid_block, shmid_ip, shmid_ledger, semid_ip, semid_led, msgqid, *block;
struct p_info *ip;
struct block *ledger;
struct msqid_ds buf;

/* Storing macros from files */
void read_macro(char *argv)
{
	char macro[30];
	FILE *apriFile;
	int val;

	/* Opening macro.txt file */
	if ((apriFile = fopen(argv, "r")) == NULL)
	{
		fprintf(stderr, "ERROR fopen ");
		TEST_ERROR;
	}

	/* Reading values ​​from macro.txt file */
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
		else if (strcmp(macro, "SO_SIM_SEC") == 0)
			SO_SIM_SEC = val;
	}

	/* Closing macro.txt file */
	if (fclose(apriFile))
	{
		fprintf(stderr, "ERROR fclose ");
		TEST_ERROR;
	}
}

/* Waiting for a second */
void one_sec_waited_master()
{
	struct timespec time;

	time.tv_sec = 1 / NSEC;
	time.tv_nsec = 999999999;
	if (clock_nanosleep(CLOCK_MONOTONIC, 0, &time, NULL) == -1)
	{
		fprintf(stderr, "ERROR clock_nanosleep ");
		TEST_ERROR;
	}
}

/* IPC Resource Removal */
void clear()
{
	/* Shared Memory Deallocation */
	shmdt(ip);
	shmctl(shmid_ip, IPC_RMID, NULL);
	shmctl(shmid_ledger, IPC_RMID, NULL);
	shmctl(shmid_block, IPC_RMID, NULL);

	/* Traffic light deallocation */
	semctl(semid_ip, 0, IPC_RMID);
	semctl(semid_led, 0, IPC_RMID);
}

/* Print only major and minor users */
void print_max_min_users(struct p_info *ptr)
{
	int min, max, pos_min, pos_max, i;

	min = ptr[0].proc_balance;
	max = ptr[0].proc_balance;
	pos_min = 0;
	pos_max = 0;

	for (i = 1; i < SO_USERS_NUM; i++)
	{
		if (ptr[i].proc_balance < min)
		{
			pos_min = ptr[i].pos;
			min = ptr[i].proc_balance;
		}
		if (ptr[i].proc_balance > max)
		{
			pos_max = ptr[i].pos;
			max = ptr[i].proc_balance;
		}
	}

	if (ptr[pos_min].term == true)
	{
		printf("User | PID: %d --> MIN Balance: %d [terminated]\n", ptr[pos_min].proc_pid, min);
	}
	else
	{
		printf("User | PID: %d --> MIN Balance: %d \n", ptr[pos_min].proc_pid, min);
	}

	if (ptr[pos_max].term == true)
	{
		printf("User | PID: %d --> MAX Balance: %d [terminated]\n", ptr[pos_max].proc_pid, max);
	}
	else
	{
		printf("User | PID: %d --> MAX Balance: %d \n", ptr[pos_max].proc_pid, max);
	}
}

/* Print only major and minor nodes */
void print_max_min_nodes(struct p_info *ptr)
{
	int min, max, pos_min, pos_max, i;

	min = ptr[SO_USERS_NUM].proc_balance;
	max = ptr[SO_USERS_NUM].proc_balance;
	pos_min = SO_USERS_NUM;
	pos_max = SO_USERS_NUM;

	for (i = SO_USERS_NUM + 1; i < SO_USERS_NUM + SO_NODES_NUM; i++)
	{
		if (ptr[i].proc_balance < min)
		{
			pos_min = ptr[i].pos;
			min = ptr[i].proc_balance;
		}
		if (ptr[i].proc_balance > max)
		{
			pos_max = ptr[i].pos;
			max = ptr[i].proc_balance;
		}
	}

	printf("Node | PID: %d --> MIN Balance: %d \n", ptr[pos_min].proc_pid, min);
	printf("Node | PID: %d --> MAX Balance: %d \n", ptr[pos_max].proc_pid, max);
}

/* Calculate process balances by reading the ledger */
void read_balance_ledger()
{ /* Run every second: update the proc_balance of each process (see p_indo structure in the header) */
	int i, j, k;

	/* block[0]: counter of how many blocks in the ledger are written */
	for (i = 0; i < SO_USERS_NUM; i++)
	{ /* Scans all user processes */
		ip[i].proc_balance = SO_BUDGET_INIT;
		for (j = 0; j < block[0]; j++)
		{ /* Scans every block of the ledger */
			for (k = 0; k < SO_BLOCK_SIZE; k++)
			{ /* Scans every single transaction */
				if (ip[i].proc_pid == ledger[j].array_trans[k].sender)
				{																													 /* If process i is the sender of the transaction */
					ip[i].proc_balance -= ledger[j].array_trans[k].quantity; /* Balance of i minus quantity of the sent transaction */
				}
				if (ip[i].proc_pid == ledger[j].array_trans[k].receiver)
				{																													 /* If process i is the receiver of the transaction */
					ip[i].proc_balance += ledger[j].array_trans[k].quantity; /* Balance of the most quantity of the received transaction */
				}
			}
		}
	}

	/* Update the nodes balance with the latest transaction of the block (reward) */
	for (i = SO_USERS_NUM; i < SO_USERS_NUM + SO_NODES_NUM; i++)
	{ /* Scans all nodes */
		ip[i].proc_balance = 0;
		for (j = 0; j < block[0]; j++)
		{ /* Scans every block of the ledger */
			if (ip[i].proc_pid == ledger[j].array_trans[SO_BLOCK_SIZE - 1].receiver)
			{																																					 /* The last transaction in each block of the ledger */
				ip[i].proc_balance += ledger[j].array_trans[SO_BLOCK_SIZE - 1].quantity; /* Balance of the most quantity of the received transaction */
			}
		}
	}
}

/* Print processes */
void print_proc(struct p_info *ip)
{
	int i;

	read_balance_ledger(); /* I read the ledger and update p_info */

	if (SO_USERS_NUM > MAX_PROC)
	{
		if (SO_NODES_NUM > MAX_PROC)
		{
			/* CASE 1: Print user processes and nodes with higher and lower budgets */
			print_max_min_users(ip);
			print_max_min_nodes(ip);
		}
		else if (SO_NODES_NUM <= MAX_PROC)
		{
			/* CASE 2: Print user processes with higher and lower budget and all nodes */
			print_max_min_users(ip);

			for (i = SO_USERS_NUM; i < SO_USERS_NUM + SO_NODES_NUM; i++)
			{
				printf("Node | PID: %d -->     Balance: %d \n", ip[i].proc_pid, ip[i].proc_balance);
			}
		}
	}
	else if (SO_USERS_NUM <= MAX_PROC)
	{
		if (SO_NODES_NUM > MAX_PROC)
		{
			/* CASE 3: Print node processes with higher and lower budget and all users */
			for (i = 0; i < SO_USERS_NUM; i++)
			{
				if (ip[i].term == true)
				{
					printf("User | PID: %d -->     Balance: %d [terminated]\n", ip[i].proc_pid, ip[i].proc_balance);
				}
				else
				{
					printf("User | PID: %d -->     Balance: %d \n", ip[i].proc_pid, ip[i].proc_balance);
				}
			}
			print_max_min_nodes(ip);
		}
		else if (SO_NODES_NUM <= MAX_PROC)
		{
			/* CASE 4: Print all user and node processes */
			for (i = 0; i < SO_USERS_NUM; i++)
			{
				if (ip[i].term == true)
				{
					printf("User | PID: %d -->     Balance: %d [terminated]\n", ip[i].proc_pid, ip[i].proc_balance);
				}
				else
				{
					printf("User | PID: %d -->     Balance: %d \n", ip[i].proc_pid, ip[i].proc_balance);
				}
			}

			for (i = SO_USERS_NUM; i < SO_USERS_NUM + SO_NODES_NUM; i++)
			{
				printf("Node | PID: %d -->     Balance: %d \n", ip[i].proc_pid, ip[i].proc_balance);
			}
		}
	}
}

/* Print the end of the simulation */
void print_end(char *string)
{
	int j;

	printf("\n-*-*-*-*-*- THE SIMULATION ENDS NOW -*-*-*-*-*-\n\n");
	printf("TERMINATION REASON: %s\n", string); /* String: termination reason */
	print_proc(ip);
	printf("\nUsers processes terminated: %d\n", users_term);

	if (block[0] == SO_REGISTRY_SIZE)
	{
		printf("Maximum capacity of %d blocks reached in the ledger\n\n", SO_REGISTRY_SIZE);
	}
	else
	{
		printf("Occupied blocks in the ledger: %d\n\n", block[0]);
	}

	printf("\n\nNUMBER OF TRANSACTIONS IN THE TRANSACTION POOL:\n\n"); /* List of transactions received to be processed */

	for (j = SO_USERS_NUM; j < SO_USERS_NUM + SO_NODES_NUM; j++)
	{ /* Loop through all nodes to print remaining messages in TP */
		if ((msgqid = msgget(ip[j].proc_pid, 0)) == -1)
		{ /* I put in msgqid the message queue attack of process j */
			fprintf(stderr, "ERROR msgget ");
			TEST_ERROR;
		}
		if ((msgctl(msgqid, IPC_STAT, &buf)) == -1)
		{ /* I refresh buf to see how many transactions are left at node j */
			fprintf(stderr, "ERROR msgctl ");
			TEST_ERROR;
		}
		printf("Node | PID -->  %d    Remaining transaction --> %ld \n", ip[j].proc_pid, buf.msg_qnum); /* Print the number of messages related to node j */
	}
	printf("\n\n");
}

/* Signal Management */
void handle_signal(int sig)
{
	switch (sig)
	{
	case SIGALRM: /* Alarm signal: time up */
		print_end("Seconds of the simulation passed!\n");
		kill(0, SIGTERM);
		break;

	case SIGUSR1: /* Signal received from nodes when the ledger is full */
		print_end("The maximum ledger size has been reached!\n");
		kill(0, SIGTERM);
		break;

	case SIGINT: /* User-sent termination signal (CTRL-C) */
		print_end("Signal CTRL-C received!\n");
		kill(0, SIGTERM);
		break;

	case SIGTERM:					 /* End of simulation signal */
		waitpid(0, NULL, 0); /* Waiting for death all processes */
		clear();
		exit(EXIT_SUCCESS);
		break;

	default:
		break;
	}
}
