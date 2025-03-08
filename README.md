# Queue-using-mutex-lock

This is a thread-safe queue implementation in C, designed for concurrent access by multiple threads. The queue supports basic operations like enqueue, dequeue, and tryDequeue, along with additional features such as waiting threads management and access tracking.

## Features

- **Thread Safety**: Uses mutexes (`mtx_t`) and condition variables (`cnd_t`) to ensure thread-safe operations.
- **Dynamic Memory Management**: Allocates and frees memory dynamically for queue nodes and thread nodes.
- **Waiting Threads Management**: Tracks and manages threads waiting to dequeue items when the queue is empty.
- **Access Tracking**: Tracks the number of items accessed (`visited`) and the current size of the queue (`size`).

## Code Overview

The implementation consists of the following components:

1. **Data Structures**:
   - `struct Node`: Represents a node in the main queue. It contains a value (`void* val`) and a pointer to the next node.
   - `struct Queue`: Represents the main queue. It contains pointers to the head and tail nodes, along with atomic counters for the queue length and access count.
   - `struct TNode`: Represents a node in the thread queue. It contains a condition variable (`cnd_t TCond`) and a pointer to the next node.
   - `struct TQueue`: Represents the thread queue. It contains pointers to the head and tail nodes, along with an atomic counter for the queue length.

2. **Global Variables**:
   - `mtx_t pLock`: Mutex for protecting the main queue.
   - `mtx_t sLock`: Mutex for protecting the thread queue.
   - `struct Queue pQueue`: The main queue.
   - `struct TQueue threadsQueue`: The thread queue.
   - `int isInit`: Flag to track whether the queues have been initialized.

3. **Functions**:
   - `initQueue`: Initializes the queues and mutexes.
   - `destroyQueue`: Destroys the queues and frees allocated memory.
   - `waiting`: Returns the number of threads waiting to dequeue items.
   - `enqueue`: Adds an item to the main queue and signals waiting threads if any.
   - `size`: Returns the current size of the main queue.
   - `visited`: Returns the number of items accessed from the queue.
   - `dequeue`: Removes and returns an item from the main queue. If the queue is empty, the calling thread waits until an item is available.
   - `tryDequeue`: Attempts to remove and return an item from the main queue without blocking. Returns `false` if the queue is empty or another thread is waiting.

## Usage
