# SO - Operating Systems

We intend to simulate a ledger containing the data of monetary transactions between different users.

Inside the folder there is a `macro.txt` file which contains all the parameters to be read at run time. They are read by the `read_macro(argv[1])` function present in the `master.c`, `users.c` and `nodes.c` files. The `header.h` file includes all the libraries needed for the correct execution of the program, the keys for semaphores and shared memories are defined, the parameters to be read at compile time are defined and the following structures are defined:
* **transaction**: structure of a transaction that contains the timestamp, sender, receiver, quantity and reward fields.
* **p_info**: structure that allows you to have information about a process and contains the fields `proc_pid` (PID of the process), `proc_balance` (balance of the process), `pos` (position of the process in the shared memory), `term` (process terminated or not)
* **block**: structure of a block of the ledger which contains the fields `array_trans` (array of transactions of size `SO_BLOCK_SIZE`) and `id` (numeric identifier of the block)
* **message**: structure of the message to be sent to the message queues which contains the fields `mtype` (type of the message) and `mtext` (text of the message)
In the `master.h`, `users.h` and `nodes.h` files the method signatures are declared, used in the respective files.
The `master_functions.c` file implements the methods that are used in `master.c`, the `users_functions.c` file implements the methods that are used in `users.c`, and the `nodes_functions.c` file implements the methods that are used in `nodes.c`.

### Master
Initially the parameters from the `macro.txt` file are read and values are assigned to the macros. It uses the `sigaction()` system call to handle signals and a `handle_signal(sig)` function is defined to handle signals that may occur during simulation. Creates shared memory for the block counter in the ledger, for the `p_info` structure with process information, and for the ledger. Create semaphore to write to ledger and create semaphore to wait for all processes to be created. Subsequently, through a loop of length `SO_USERS_NUM` + `SO_NODES_NUM` and a `fork()`, it creates the child processes users and nodes, places them in shared memory and calls `execvp()` for both users and nodes. Once the process creation wait is over, thanks to the semaphore of the `SEMKEY_P_INFO` set, it decreases the latter and the simulation starts. Set an `alarm()` of `SO_SIM_SEC` and enter the `while(1)` loop, where a second wait is set, for subsequent printouts, with the `one_sec_waited_master()` function. Update the number of terminated users, subtract them from the active ones and check if they are all terminated or if only one is left alive (who cannot send transactions to anyone else). (In these two cases, print the termination with the `print_end()` function and send a simulation end signal). With the `print_proc()` function, print the PID of the processes, their balance and whether they are finished or not. Finally, it prints the number of terminated users, active users and nodes, and blocks written to the ledger. After the simulation, deallocate all the IPC resources.

### Users
Initially the parameters from the `macro.txt` file are read and values are assigned to the macros. It uses the `sigaction()` system call to handle signals and a `handle_signal(sig)` function is defined to handle signals that may occur during simulation. Stores the memory identifiers for the block counter in the ledger, for the `p_info` structure with process information, and for the ledger in variables. It also saves the semaphore set identifier `SEMKEY_P_INFO`. Wait for the green light to run the simulation through semaphores, using the `wait_for_zero()` function. With a loop `for(j = 0; j < SO_USERS_NUM; j++)` it recovers its position in shared memory and initializes three variables: `balance`, `index_blocks` and `attempt`, respectively the balance of the process, the index of blocks read and the transaction sending attempts. Set an `alarm(rand() % SO_SIM_SEC + 1)` to generate the `SIGALRM` signal chosen by us to send a transaction. It enters the `while(attempt != SO_RETRY)` loop and exits when it has failed to send a transaction `SO_RETRY` consecutive times. Inside the `while(attempt != SO_RETRY)` it calculates the budget with `budget_calc()`, initializes the transaction with `trans_init()` and sets it up and sends it to the node via `trans_send()`. Exited the `while(attempt != SO_RETRY)` update the term field to true of the struct `p_info` and terminate.

### Nodes
Initially the parameters from the `macro.txt` file are read and values are assigned to the macros. It uses the `sigaction()` system call to handle signals and a `handle_signal(sig)` function is defined to handle signals that may occur during simulation. It creates the message queue, "initializes" it with `IPC_STAT`, sets the maximum byte size in the queue with `buf.msg_qbytes = SO_TP_SIZE * sizeof(struct transaction)` and updates the `msqid_ds` structure with the new maximum size. Therefore, the transaction pool is created by the node with the help of the message queue, which will have maximum `SO_TP_SIZE` queued transactions. Saves the memory identifiers for the block counter in the ledger and for the ledger in the variables. Also saves the semaphore set identifier `SEMKEY_P_INFO` and the semaphore set identifier `SEMKEY_LEDGER` in the variables. Wait for the green light to start simulating through semaphores, with the `wait_for_zero()` function. Enter the `while(1)` loop where, with `IPC_STAT`, update buf to see how many messages are queued and, if there are enough, create the block by reading `SO_BLOCK_SIZE – 1` transactions from the transaction pool with `msgrcv()`. In the last position of the block it adds the reward transaction with the `add_reward_trans()` function. Do `reserve_sem()` to allow the block to be written to the ledger. Check if there is space in the ledger, update the block id with the current number of blocks, simulate waiting for processing with the `one_sec_waited_nodes()` function, write the block to the ledger and increment the block counter of the ledger master. After writing the block it checks if the ledger is out of space and if so it sends a `SIGUSR1` signal to the master and pauses the process waiting for `SIGTERM`, which, when it arrives, deallocates the IPC resources and terminates. At the end of the `while(1)` does `release_sem()`.

## Table of Contents

- [Introduction](#introduction)
- [Characteristics](#characteristics)
- [Installation](#installation)
- [Usage](#usage)
- [License](#license)

## Introduction

The Web Technologies project was useful for learning the basics of web development. As mentioned above, different programming languages and markup were used to gain knowledge of how to work in the network environment.

## Characteristics

The repository consists of two folders:
- Folder **lab**: contains six laboratory exercises with the various deliverables and implementations;
- Folder **project**: contains the code of the actual website, the requirements provided by the professor, and the report describing the entire project in full.

## Installation

To install the following project, you must have installed a software application on your machine that provides a local development environment for creating and testing websites and web applications (I used **XAMPP**).

To clone the repository locally, simply run in your *Git Bash* the command `git clone https://github.com/albertoomarino/TWeb.git`.

## Usage

Once you have installed the project and launched it, you need to create a database called `getbook`. At this point, from the XAMPP web interface, you can import the complete database in the `getbook.sql` file into the `database` folder.

After registering to the site rather than logging in, it is possible to navigate within the platform to view all implemented functions. For more details on navigation and implementation, please refer to the `Report.pdf`.

## License

Copyright (c) [2023] [Alberto Marino]

All rights reserved. This software is protected by copyright laws and international license agreements.

### License Agreement

This License Agreement (hereinafter referred to as the "Agreement") is entered into between the Author or the Right Holder (hereinafter referred to as the "Licensor") and any individual or legal entity (hereinafter referred to as the "Licensee") intending to use the software called [Web-Technologies] (hereinafter referred to as the "Software").

#### 1. GRANT OF LICENSE

Licensor grants Licensee the non-exclusive, non-transferable, revocable right to use the Software in accordance with the terms and conditions of this Agreement.

#### 2. LIMITATIONS

Licensee may not:

a) Copy, distribute, or disclose the Software or any part thereof to any third party, whether free of charge or for any form of compensation.
b) Modify, adapt, reverse engineer, decompile, or disassemble the Software or parts thereof.
c) Remove or alter any copyright notices, proprietary legends, or other identifying symbols in the Software.

#### 3. DISCLAIMER

THE SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED. LICENSOR DISCLAIMS ALL WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT. THE LICENSOR SHALL NOT BE LIABLE FOR ANY DAMAGES ARISING OUT OF THE USE OF THE SOFTWARE.

#### 4. TERMINATION

This Agreement shall remain in force until terminated by either party. The Licensor may terminate this Agreement at any time if the Licensee breaches any of the terms set forth herein. Upon termination, Licensee shall cease all use of the Software and destroy all copies in its possession.

#### 5. APPLICABLE LAW

This Agreement shall be governed by and construed in accordance with the laws of [Jurisdiction]. Any disputes arising out of this Agreement shall be submitted to the exclusive jurisdiction of the courts of [Jurisdiction].

By using the Software, Licensee agrees to be bound by the terms and conditions of this Agreement.

