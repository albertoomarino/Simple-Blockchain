#include "nodes.h"

int main(int argc, char *argv[])
{
	/* Used also in nodes_functions.c */
	extern int SO_TP_SIZE;
	extern int msgqid_node, semid_led, semid_ip, quantity_reward, j;
	extern bool isDeclared;
	extern struct block *ledger;
	extern struct block curr_block;

	/* Used in nodes.c */
	int shmid_block, shmid_ledger, *block;
	struct message mymsg;
	struct msqid_ds buf;
	struct sigaction sa;
	sigset_t my_mask;

	/* True if SO_MIN_TRANS_PROC_NSEC and SO_MAX_TRANS_PROC_NSEC are present in macro.txt, false otherwise */
	isDeclared = false;

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
	sigaction(SIGINT, &sa, NULL); /* By default, it breaks the process */

	/* Create the message queue and assign it to msgqid_node */
	if ((msgqid_node = msgget(getpid(), IPC_CREAT | 0666)) == -1)
	{
		fprintf(stderr, "ERROR msgget ");
		TEST_ERROR;
	}

	/* Copy the msqid_ds structure into the buffer pointed by buf (initialize it) */
	if ((msgctl(msgqid_node, IPC_STAT, &buf)) == -1)
	{
		fprintf(stderr, "ERROR msgctl ");
		TEST_ERROR;
	}

	/* Set the maximum byte size for the message queue */
	buf.msg_qbytes = SO_TP_SIZE * sizeof(struct transaction);

	/* Set the message queue with the maximum byte size */
	if ((msgctl(msgqid_node, IPC_SET, &buf)) == -1)
	{
		fprintf(stderr, "ERROR msgctl ");
		TEST_ERROR;
	}

	/* Get the memory identifier for the block counter in the ledger */
	if ((shmid_block = shmget(SHMID_BLOCK, sizeof(int), 0)) == -1)
	{
		fprintf(stderr, "ERROR shmget ");
		TEST_ERROR;
	}

	/* Attach to the block memory address and convert it to an integer */
	if ((block = (int *)shmat(shmid_block, NULL, 0)) == (void *)-1)
	{
		fprintf(stderr, "ERROR shmat ");
		TEST_ERROR;
	}

	/* Get the memory identifier for the ledger */
	if ((shmid_ledger = shmget(SHMID_LEDGER, sizeof(struct block) * (SO_REGISTRY_SIZE), 0)) == -1)
	{
		fprintf(stderr, "ERROR shmget ");
		TEST_ERROR;
	}

	/* Attach to the ledger memory address and convert it to a struct of blocks */
	if ((ledger = (struct block *)shmat(shmid_ledger, NULL, 0)) == (void *)-1)
	{
		fprintf(stderr, "ERROR shmat ");
		TEST_ERROR;
	}

	/* Get the semaphore identifier for the SEMKEY_LEDGER set */
	if ((semid_led = semget(SEMKEY_LEDGER, 0, 0)) == -1)
	{
		fprintf(stderr, "ERROR semget ");
		TEST_ERROR;
	}

	/* Get the semaphore identifier for the SEMKEY_P_INFO set */
	if ((semid_ip = semget(SEMKEY_P_INFO, 0, 0)) == -1)
	{
		fprintf(stderr, "ERROR semget ");
		TEST_ERROR;
	}

	/* Wait for the creation of processes */
	wait_for_zero();

	while (1)
	{
		quantity_reward = 0;
		if ((msgctl(msgqid_node, IPC_STAT, &buf)) == -1)
		{ /* Update buf to see how many messages are in the queue */
			fprintf(stderr, "ERROR msgctl ");
			TEST_ERROR;
		}

		if (buf.msg_qnum >= SO_BLOCK_SIZE - 1)
		{ /* If there are enough messages, create the block (SO_BLOCK_SIZE - 1 transactions received from users) */
			for (j = 0; j < SO_BLOCK_SIZE - 1; j++)
			{ /* Read SO_BLOCK_SIZE - 1 transactions from the queue and insert them into the block */
				if (msgrcv(msgqid_node, &mymsg, sizeof(struct transaction), 0, 0) == -1)
				{
					fprintf(stderr, "ERROR msgrcv ");
					TEST_ERROR;
				} /* Fill the block with transactions */

				quantity_reward += mymsg.mtext.reward;	 /* Add the reward amount by accessing the reward field of the transaction mtext */
				curr_block.array_trans[j] = mymsg.mtext; /* Adds the transaction just read from the queue at position j of the block */
			}

			/* In the last position of the block, add the reward transaction (which is the last to enter the block from the message queue) */
			add_reward_trans();

			/* No one else can write to the ledger from now on */
			if (reserveSem(semid_led) == -1)
			{ /* Entry section */
				fprintf(stderr, "ERROR reserveSem ");
				TEST_ERROR;
			}

			if (block[0] < SO_REGISTRY_SIZE)
			{																 /* Critical section: writing the transaction block into the ledger */
				curr_block.id = block[0];			 /* Update block id with the current number of blocks */
				one_sec_waited_nodes();				 /* Simulate block processing wait */
				ledger[block[0]] = curr_block; /* Write the entire block into the ledger */
				block[0]++;										 /* Increment the actual number of blocks written into the ledger */

				if (block[0] == SO_REGISTRY_SIZE)
				{
					kill(getppid(), SIGUSR1); /* Space is exhausted, so notify the master */
					pause();									/* Pause the process and wait for SIGTERM signal */
				}
			}

			/* Release the semaphore, other processes can write to the ledger */
			if (releaseSem(semid_led) == -1)
			{ /* Exit section */
				fprintf(stderr, "ERROR releaseSem ");
				TEST_ERROR;
			}
		}
	}
}
