# Simple Blockchain

## Introduction

The goal is to simulate a ledger containing monetary transaction data between different users. For this purpose, the following processes are present:

- A master process that manages the simulation, the creation of other processes, etc;
- `SO_USERS_NUM` user processes that can send money to other users through a transaction;
- `SO_NODES_NUM` node processes that process transactions for a fee.

### Transactions

A transaction is characterized by the following information:

- Transaction timestamp with nanosecond resolution (see `clock_gettime(...)` function);
- Sender (implicit, as it is the user who generated the transaction);
- Receiver, the user receiving the amount;
- Amount of money sent;
- Reward, money paid by the sender to the node that processes the transaction.

The transaction is sent by the user process that generates it to one of the node processes, chosen at random.

### User Processes

User processes are responsible for creating and sending monetary transactions to node processes. Each user process is assigned an initial budget `SO_BUDGET_INIT`. During its lifecycle, a user process iteratively performs the following operations:

1. Calculates the current balance based on the initial budget, summing the income and expenses recorded in the transactions in the ledger, subtracting the amounts of transactions sent but not yet recorded in the ledger.

   - If the balance is greater than or equal to 2, the process randomly selects:
     - Another user process as the recipient of the money;
     - A node to send the transaction for processing;
     - An integer value between 2 and its balance, divided as follows:
       - The transaction reward, which is a percentage `SO_REWARD` of the selected value, rounded, with a minimum of 1;
       - The transaction amount will be the selected value minus the reward.
   
   Example: the user has a balance of 100. Randomly selecting a number between 2 and 100, they pick 50. If `SO_REWARD` is 20 (indicating a 20% reward), then by executing the transaction, the user will transfer 40 to the recipient user and 10 to the node that successfully processed the transaction.
   
   - If the balance is less than 2, the process does not send any transactions.

2. Sends the transaction to the selected node and waits for a randomly extracted time interval (in nanoseconds) between `SO_MIN_TRANS_GEN_NSEC` and `SO_MAX_TRANS_GEN_NSEC`.

Additionally, a user process must generate a transaction in response to a received signal (the choice of the signal is at the developers’ discretion).

If a process fails to send any transaction for `SO_RETRY` consecutive times, it terminates its execution, notifying the master process.

### Node Processes

Each node process privately stores the list of received transactions to be processed, called the transaction pool, which can contain a maximum of `SO_TP_SIZE` transactions, with `SO_TP_SIZE > SO_BLOCK_SIZE`. If the node’s transaction pool is full, any additional transaction is discarded and thus not executed. In this case, the sender of the discarded transaction must be informed.

Transactions are processed by a node in blocks. Each block contains exactly `SO_BLOCK_SIZE` transactions to be processed, of which `SO_BLOCK_SIZE−1` are transactions received from users and one is a processing payment transaction.

The lifecycle of a node can be defined as follows:

- Creation of a candidate block:
  - Extracting `SO_BLOCK_SIZE−1` transactions from the transaction pool that are not yet in the ledger.
  - The node adds a reward transaction to the block with the following characteristics:
    - `timestamp`: the current value of `clock_gettime(...)`;
    - `sender`: -1 (define a MACRO...);
    - `receiver`: the identifier of the current node;
    - `amount`: the sum of all the rewards of the transactions included in the block;
    - `reward`: 0.

- Simulating block processing by waiting for a random time interval in nanoseconds between `SO_MIN_TRANS_PROC_NSEC` and `SO_MAX_TRANS_PROC_NSEC`;

- After completing block processing, the node writes the newly processed block into the ledger and removes the successfully executed transactions from the transaction pool.

### Ledger

The ledger is the shared structure among all nodes and users and is responsible for storing executed transactions. A transaction is considered confirmed only when it becomes part of the ledger. More specifically, the ledger consists of a sequence of up to `SO_REGISTRY_SIZE` consecutive blocks. Each block contains exactly `SO_BLOCK_SIZE` transactions. Each block is identified by a sequential integer ID, starting at 0.

A transaction is uniquely identified by the triplet `(timestamp, sender, receiver)`. The node that adds a new block to the ledger is also responsible for updating the block’s identifier.

### Printing

Every second, the master process prints:

- The number of active user and node processes;
- The current budget of each user process and node process, as recorded in the ledger (including terminated user processes). If the number of processes is too large to display, only the most significant processes are printed: those with the highest and lowest budgets.

### Simulation Termination

The simulation will terminate in one of the following cases:

- `SO_SIM_SEC` seconds have elapsed;
- The ledger capacity is exhausted (the ledger can contain a maximum of `SO_REGISTRY_SIZE` blocks);
- All user processes have terminated.

Upon termination, the master process forces all node and user processes to terminate and prints a summary of the simulation, containing at least the following information:

- Reason for simulation termination;
- Balance of each user process, including those that terminated prematurely;
- Balance of each node process;
- Number of prematurely terminated user processes;
- Number of blocks in the ledger;
- For each node process, the number of transactions still present in the transaction pool.

## Configuration

The following parameters are read at runtime, from a file, from environment variables, or from stdin (at the students' discretion):

- `SO_USERS_NUM`: number of user processes;
- `SO_NODES_NUM`: number of node processes;
- `SO_BUDGET_INIT`: initial budget of each user process;
- `SO_REWARD`: the percentage of reward paid by each user for transaction processing;
- `SO_MIN_TRANS_GEN_NSEC`, `SO_MAX_TRANS_GEN_NSEC`: minimum and maximum value of the time (expressed in nanoseconds) that elapses between the generation of one transaction and the next by a user;
- `SO_RETRY`: maximum number of consecutive failures in transaction generation after which a user process terminates;
- `SO_TP_SIZE`: maximum number of transactions in the transaction pool of node processes;
- `SO_MIN_TRANS_PROC_NSEC`, `SO_MAX_TRANS_PROC_NSEC`: minimum and maximum simulated processing time (expressed in nanoseconds) of a block by a node;
- `SO_SIM_SEC`: duration of the simulation (in seconds).

A change in the above parameters should not require recompilation of the source code.
Instead, the following parameters are read at compile time:

- `SO_REGISTRY_SIZE`: maximum number of blocks in the ledger;
- `SO_BLOCK_SIZE`: number of transactions contained in a block.

The following table lists values for some example configurations to be tested. Note that the project must be able to function with other parameters as well.

| Parameter | Read at | Conf#1 | Conf#2 | Conf#3 |
|-----------|---------|--------|--------|--------|
| `SO_USERS_NUM` | Run time | 100 | 1000 | 20 |
| `SO_NODES_NUM` | Run time | 10 | 10 | 10 |
| `SO_BUDGET_INIT` | Run time | 1000 | 1000 | 10000 |
| `SO_REWARD [0–100]` | Run time | 1 | 20 | 1 |
| `SO_MIN_TRANS_GEN_NSEC [nsec]` | Run time | 100000000 | 10000000 | 10000000 |
| `SO_MAX_TRANS_GEN_NSEC [nsec]` | Run time | 200000000 | 10000000 | 20000000 |
| `SO_RETRY` | Run time | 20 | 2 | 10 |
| `SO_TP_SIZE` | Run time | 1000 | 20 | 100 |
| `SO_BLOCK_SIZE` | Compile time | 100 | 10 | 10 |
| `SO_MIN_TRANS_PROC_NSEC [nsec]` | Run time | 10000000 | 1000000 | - |
| `SO_MAX_TRANS_PROC_NSEC [nsec]` | Run time | 20000000 | 1000000 | - |
| `SO_REGISTRY_SIZE` | Compile time | 1000 | 10000 | 1000 |
| `SO_SIM_SEC [sec]` | Run time | 10 | 20 | 20 |
| `SO_NUM_FRIENDS` | Run time | 3 | 5 | 3 |
| `SO_HOPS` | Run time | 10 | 2 | 10 |

## Implementation Requirements

The project (both in its “minimal” and “normal” versions) must:

- Be implemented using modular coding techniques;
- Be compiled using the `make` utility;
- Maximize concurrency between processes;
- Deallocate IPC resources allocated by processes at the end of execution;
- Be compiled with at least the following compilation options:
  ```bash
  gcc -std=c89 -pedantic
  ```
- Be able to run correctly on a machine (virtual or physical) that supports parallelism (two or more processors).

For the reasons introduced in the lectures, remember to define the `GNU_SOURCE` macro or compile the project with the `-D_GNU_SOURCE` flag.

## Running the Project

Once you have installed the project and launched it, you are ready to execute the code. Below are the essential commands, as specified in the `Makefile`:

- **Compile the project**: generates three binaries (`master`, `users`, and `nodes`), each corresponding to a specific system component.
  ```bash
  make all
  ```

- **Run the main application**: starts the `master` program with the argument `"macro.txt"`, which serves as a specific input file or configuration.
  ```bash
  make run
  ```

- **Clean the project**: removes all object files (`.o`) and the generated binaries to maintain a clean directory.
  ```bash
  make clean
  ```

## License

This project is distributed under the [MIT License](https://github.com/albertoomarino/operating-systems/blob/main/LICENSE), granting permission to use, modify, and distribute the code with proper attribution.
