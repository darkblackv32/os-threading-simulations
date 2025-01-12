
# Threading Simulations

## Table of Contents
1. [Introduction](#introduction)
2. [Chef's Problem](#chefs-problem)
    - [Explanation](#explanation)
    - [Code Overview](#code-overview)
3. [Cinema Simulation](#cinema-simulation)
    - [Explanation](#explanation-1)
    - [Code Overview](#code-overview-1)
4. [Valgrind Tests](#valgrind-tests)
5. [How to Run](#how-to-run)
6. [Results](#results)

---

## Introduction
This project contains two main implementations using C with `pthread` for thread-based concurrency:
1. **Chef's Problem**: A synchronization problem involving multiple threads coordinating through files.
2. **Cinema Simulation**: A multiplex cinema model that demonstrates thread management and monitor implementation.

---

## Chef's Problem

### Explanation
The Chef's problem involves synchronizing threads where a **Master Chef** interacts with three chefs:
- **Master Chef**: Places two random ingredients on the table (managed as a file).
- **Chefs**: Each chef has one unique ingredient and attempts to collect the other two ingredients to prepare ramen.

**Techniques used:**
- Mutex (`pthread_mutex_t`) to manage file access.
- Condition variables (`pthread_cond_t`) to synchronize chef actions.

### Code Overview
The implementation consists of:
1. **Master Chef**: Randomly places two ingredients and notifies chefs.
2. **Chefs**: Each chef checks for matching ingredients, prepares ramen, and clears the table.

---

## Cinema Simulation

### Explanation
This program simulates a multiplex cinema with:
- **Rooms**: Three rooms with capacities of 4, 5, and 7 people.
- **Customers**: Attempt to enter randomly selected rooms.
- **Projections**: Simulated using threads, each room running its projection independently.

**Techniques used:**
- Monitor structure to manage room data (occupancy, projection status).
- Mutex and condition variables for thread-safe operations and customer waiting.

### Code Overview
The cinema monitor structure maintains:
- Room capacities.
- Current occupancy and projection status.

---

## Valgrind Tests
The code has been validated using `valgrind` for memory and concurrency issues:
1. **Memory Leak Summary**:
    - No memory leaks detected.
2. **Detailed Memory Check**:
    - Verified no errors related to memory allocation and deallocation.
3. **Thread Concurrency Check**:
    - Verified no race conditions or deadlocks using Helgrind.

Refer to the figures in the LaTeX document for visual summaries.

---

## How to Run
1. Clone the repository and navigate to the source code directory.
2. Compile the programs using:
   ```bash
   gcc -pthread -o chef chef.c && valgrind --tool=helgrind ./chef
   gcc -pthread -o cine cine.c && valgrind --tool=helgrind ./cinema
   ```
3. Run the programs:
   ```bash
   ./chefs or make chef
   ./cinema or make cinema
   ```

---

## Results
1. **Chef's Problem**:
    - Successfully simulated the synchronization between the Master Chef and three chefs over fixed rounds.
    - Verified no deadlocks or data corruption.

2. **Cinema Simulation**:
    - Successfully managed customer entry and room projections.
    - Ensured all threads safely interacted with shared data structures.

---
