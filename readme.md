# Multi-Lookup: Multithreaded Producer–Consumer DNS Resolver

A C program implementing the producer/consumer pattern using POSIX threads to resolve hostnames concurrently. The project focuses on safe multithreaded programming, synchronization, and performance tradeoffs.

---

## Overview

This project implements a bounded-buffer producer/consumer system where multiple requester threads read hostnames from input files and multiple resolver threads perform DNS lookups concurrently.

The primary goal of this project was to gain hands-on experience with multithreading in C, including mutexes, condition variables, and avoiding common concurrency issues such as race conditions and deadlocks.

---

## Design

### Thread Roles

- **Requester threads (producers)**  
  Read hostnames from one or more input files and place them into a shared bounded buffer.

- **Resolver threads (consumers)**  
  Remove hostnames from the buffer and perform DNS lookups, writing results to an output file.

---

### Shared Data Structures

- A bounded, thread-safe array used as the producer/consumer buffer
- Shared output files protected by mutexes to prevent concurrent write corruption

---

### Concurrency & Synchronization

- Implemented using **POSIX threads (pthreads)**
- **Mutexes** protect critical sections involving shared data
- **Condition variables** block producer threads when the buffer is full and consumer threads when the buffer is empty
- Threads exit cleanly once all input files have been processed and the buffer is drained
- Avoids busy waiting and minimizes contention between threads

---

## Build & Run

### Build
run these commands
make clean
make 

### Run
run this commands
./multi-lookup <# requester thread> <# resolver threads> <serviced.txt> <results.txt> <input/filename(s)>


You can run all files with input/filename*.txt, you can use anywhere from 1-10 threads.
example run with 5 resolver & 5 requester threads(Can copy/paste if desired):
 ./multi-lookup 5 10 serviced.txt results.txt input/*.txt


 ## Performance

Measured using 30 input files containing 618 hostnames.

| Configuration | Total Time |
|---------------|------------|
| 1 requester / 1 resolver | ~198 seconds |
| 10 requesters / 10 resolvers | ~31 seconds |

Parallel execution achieved approximately a **6.5× reduction in total runtime**.
Resolver threads complete in waves due to blocking DNS resolution calls; the remaining execution time is dominated by external DNS resolver latency rather than synchronization or contention within the program.
