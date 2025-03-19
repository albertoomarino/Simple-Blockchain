#include "users.h"

int main(int argc, char *argv[])
{
	/* Used in users_functions.c */
	extern int SO_USERS_NUM, SO_NODES_NUM, SO_SIM_SEC, SO_BUDGET_INIT, SO_RETRY;
	extern int pos, balance, index_blocks, attempt, semid_ip, *block;
	extern struct p_info *ip;
	extern struct block *ledger;

	/* Used in users.c */
	int shmid_block, shmid_ip_user, shmid_ledger, j;
	struct sigaction sa;
	sigset_t my_mask;

	/* Seed for rand() */
	srand(getpid());

	/* Store macros from file */
	read_macro(argv[1]);

	/* Signals */
	sa.sa_handler = handle_signal;
	sa.sa_flags = 0;
	sigemptyset(&my_mask);
	sa.sa_mask = my_mask;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL); /* By default it breaks */

	/* Get the identifier for the memory of the ledger block counter */
	if ((shmid_block = shmget(SHMID_BLOCK, sizeof(int), 0)) == -1)
	{
		fprintf(stderr, "ERROR shmget ");
		TEST_ERROR;
	}

	/* Attach to block memory and convert to integer */
	if ((block = (int *)shmat(shmid_block, NULL, 0)) == (void *)-1)
	{
		fprintf(stderr, "ERROR shmat ");
		TEST_ERROR;
	}

	/* Get the identifier for the p_info struct memory */
	if ((shmid_ip_user = shmget(getppid(), sizeof(struct p_info) * (SO_USERS_NUM + SO_NODES_NUM), 0)) == -1)
	{
		fprintf(stderr, "ERROR shmget ");
		TEST_ERROR;
	}

	/* Attach to ip memory and convert to p_info struct */
	if ((ip = (struct p_info *)shmat(shmid_ip_user, NULL, 0)) == (void *)-1)
	{
		fprintf(stderr, "ERROR shmat ");
		TEST_ERROR;
	}

	/* Get the identifier for the ledger memory */
	if ((shmid_ledger = shmget(SHMID_LEDGER, sizeof(struct block) * (SO_REGISTRY_SIZE), 0)) == -1)
	{
		fprintf(stderr, "ERROR shmget ");
		TEST_ERROR;
	}

	/* Attach to ledger memory and convert to block struct */
	if ((ledger = (struct block *)shmat(shmid_ledger, NULL, 0)) == (void *)-1)
	{
		fprintf(stderr, "ERROR shmat ");
		TEST_ERROR;
	}

	/* Attach to semid_ip with the identifier of SEMKEY_P_INFO */
	if ((semid_ip = semget(SEMKEY_P_INFO, 0, 0)) == -1)
	{
		fprintf(stderr, "ERROR semget ");
		TEST_ERROR;
	}

	wait_for_zero(); /* Wait for process creation */

	/* Read the process position in p_info */
	for (j = 0; j < SO_USERS_NUM; j++)
	{
		if (ip[j].proc_pid == getpid())
			pos = j;
	}

	balance = SO_BUDGET_INIT; /* Assign the initial budget to the variable */
	index_blocks = 0;					/* Where I have read in the ledger (counter) */
	attempt = 0;							/* Number of transaction send attempts */

	alarm(rand() % SO_SIM_SEC + 1); /* After rand() % SO_SIM_SEC + 1 seconds from the start of the simulation, an alarm signal is triggered */

	while (attempt != SO_RETRY)
	{								 /* Loop until attempt is not equal to SO_RETRY, otherwise exit */
		budget_calc(); /* Read from the ledger to calculate the balance */
		trans_init();	 /* Initialize the transaction */
		trans_send();	 /* Send the transaction */
	}

	ip[pos].term = true; /* Update term in p_info */
	exit(EXIT_FAILURE);
}
