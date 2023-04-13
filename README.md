# SO - Operating Systems
Laboratory of Operating Systems, Computer Science Course - A.Y. 2021/2022  
Department of Computer Science, University of Turin  
Teachers: Enrico Bini

## Lab project
We intend to simulate a ledger containing the data of monetary transactions between different users.

Inside the folder there is a macro.txt file which contains all the parameters to be read at run time. They are read by the read_macro(argv[1]) function present in the master.c, users.c and nodes.c files. The header.h file includes all the libraries needed for the correct execution of the program, the keys for semaphores and shared memories are defined, the parameters to be read at compile time are defined and the following structures are defined:
• transaction: structure of a transaction that contains the timestamp, sender, receiver, quantity and reward fields.
• p_info: structure that allows you to have information about a process and contains the fields proc_pid (PID of the process), proc_balance (balance of the process), pos (position of the process in the shared memory), term (process terminated or not)
• block: structure of a block of the ledger which contains the fields array_trans (array of transactions of size SO_BLOCK_SIZE) and id (numeric identifier of the block)
• message: structure of the message to be sent to the message queues which contains the fields mtype (type of the message) and mtext (text of the message)
In the master.h, users.h and nodes.h files the method signatures are declared, used in the respective files.
The master_functions.c file implements the methods that are used in master.c, the users_functions.c file implements the methods that are used in users.c, and the nodes_functions.c file implements the methods that are used in nodes.c.
