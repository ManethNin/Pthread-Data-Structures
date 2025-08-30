Concurrent Linked List Benchmark
A performance comparison study of different synchronization mechanisms in multi-threaded linked list implementations using POSIX threads (Pthreads).
Overview
This project implements and benchmarks three different approaches to concurrent linked list operations:

Serial Implementation: Single-threaded baseline
Mutex-based: Coarse-grained locking with one mutex for entire list
Read-Write Lock: Fine-grained locking allowing concurrent reads

Features

Thread-safe linked list with Member(), Insert(), and Delete() operations
Configurable workload distribution (read-heavy, write-heavy, balanced)
Statistical performance analysis with confidence intervals
Multi-core scalability testing (1, 2, 4, 8 threads)