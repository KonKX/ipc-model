# ipc-model

IPC model (one feeder - multiple consumer processes using shared memory)

The program implements an IPC architecture that includes a feeder (producer) process, a number of ns from the consumer processes (their number is given by the command line) and a shared memory sector accessible to both feeder and consumers.

The feeder holds a matrix of random integers (0,100) of magnitude M (given by the command line) and uses the shared memory domain to replicate that table with exactly the same composition in n consumer processes. The feeder writes to memory from one integer at a time along with its time stamp and then waits until this integer is read from all consumer processes to reactivate and write a new value.

Consumers also maintain an M-sized table to replicate the feeder's integer table. They use the shared memory domain to read the integer that is registered by the feeder and simultaneously calculate a moving average of time delay each time they derive information from the shared memory.

Shared memory is of a specific size and can store up to an integer and a time stamp. This time stamp is set at the time the feeder registers a value from its table in the shared memory domain. In doing so, consumers are asked one by one to read the shared memory information by storing the integer and counting each time the new average delay time.

The above procedure is repeated until the feeder process table is reproduced in the same order in all consumer processes, and the last completed user process prints its table and the average delay time to an output file that is common to all processes.

compile (make) and run: ./project1 <number of values> <number of processes> with number of values > 3000
  
Multiple executions of the program were performed for different n and M values with an increase in average timeouts when the number of processes is large.
