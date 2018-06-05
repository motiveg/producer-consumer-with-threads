# producer-consumer-with-threads

* Last update to program: April 15, 2018

# Problem assigned by Instructor John Baldwin for Operating System Principles (CSC 415), Spring 2018
Implement a multi-threaded producer-consumer program using a bounded buffer. Use two condition variables to coordinate access to the buffer in conjunction with a mutex. The first condition variable (ready_to_read) should be used by consumers to wait for data if the buffer is empty. The second condition variable (ready_to_write) should be used by producers to wait for an available slot if the buffer is full. The buffer should have 16 slots total and should be managed as a ring buffer. You will need to use separate variables to track the read index (the next item available to read) and the write index (the next available slot for writing).

The number of producers, the number of consumers, and the number of items each producer produces should be specified by their binary log as command-line parameters. Thus, the following command should generate 8 producers, 16 consumers, and 64 items produced by each producer:

./producer-consumer 3 4 6

The consumers need to each consume the same number of items before exiting. Your code should calculate the number of items each consumer needs to consume so that every item produced gets consumed. The main thread should parse all of the command-line parameters, print them in a message, initialize the synchronization objects, spawn all of the threads, wait for them to complete, and then print a final message. The items produced by the producer threads should be integers, obtained using the following expression:

thread_number * num_produced + counter

where thread_number is an integer passed to each thread ranging from 0 to num_threads - 1, num_produced is the number of items produced by each producer, and counter is a local variable in
each thread that gets initialized to 0 and incremented every time a new item is produced. The consumer threads should consume these items by simply printing them. (Printing is the only required "consumption".)

Use the pthread mutex and condition variable APIs for synchronization.
