## Parallel Merge Sort

* Authors: Jonathan Sawers, Shahid Hussain, Nuwin Sooriyaarachchi 
* Group name: Group 10 

## Overview

This project implements merge sort using threads. The input array is split into two parts recursively, with separate threads used for the branches until a cutoff level is reached. The remaining work is completed with the normal recursive merge sort, and the program reports the sorting time.

## Manifest

* `mergesort.c`: Implements merging, sequential merge sort, and parallel merge sort.
* `mergesort.h`: Declares the sorting functions, shared arrays, cutoff value, and thread arguments.
* `test-mergesort.c`: Generates test data, starts the sort, checks the result, and reports the time.
* `Makefile`: Compiles and links the program with pthread support.
* `README.md`: Provides the project documentation and instructions.

## Building the project

From the project directory, run:

```sh
make
```

To remove object files, dependency files, and the executable, run:

```sh
make clean
```

## Features and usage

The program supports:

* sequential merge sort when the cutoff is `0`;
* parallel sorting with a configurable number of thread levels;
* automatic fallback to sequential sorting if a thread cannot be created;
* checking that the final array is sorted; and
* measuring the elapsed sorting time.

Run the executable with three arguments:

```text
./test-mergesort <input size> <cutoff level> <seed>
```

For example:

```sh
./test-mergesort 10000 4 7
```

`input size` must be at least `2`. The cutoff level controls how many levels of the recursion are allowed to create threads, and the seed makes the random input repeatable.

## Testing

I built the project from a clean state with `make clean && make`. I then ran the following cases:

* `./test-mergesort 2 0 50`, which tested the smallest accepted input and the sequential base case
* `./test-mergesort 1000 1 100`, which tested a small parallel sort
* `./test-mergesort 10000 4 89`, which tested a larger input and several thread levels
* `./test-mergesort` without arguments, which confirmed that the usage message is displayed and the program exits with an error.

Each valid case reported that the requested number of elements was sorted. The test program also checks every adjacent pair in the final array, so a successful result means the array was in nondecreasing order.

For performance testing, the supplied program sorted 100,000,000 elements with seed `1234` on an ARM64 macOS machine, using Apple clang 21.0.0. Each cutoff was measured once:

| Cutoff | Sort time (seconds) | Speedup over serial |
| ---: | ---: | ---: |
| 0 (serial) | 14.37 | 1.00× |
| 1 | 7.25 | 1.98× |
| 2 | 3.87 | 3.71× |
| 3 | 2.60 | 5.53× |
| 4 | 2.48 | 5.79× |
| 5 | 2.63 | 5.46× |
| 6 | 3.11 | 4.62× |
| 7 | 2.85 | 5.04× |
| 8 | 2.60 | 5.53× |

Cutoff 4 was fastest in this run. These are local, single-run results; performance on Gradescope may differ.

## Known Bugs

The sorting functions use the global arrays `A` and `B`, so they are not re-entrant (can't be called again if they are already running). Two independent sorts running at the same time could overwrite each other's data.

Errors returned by `pthread_join` are ignored. If joining a thread fails, the parent could merge the arrays before that thread has finished.

The functions assume that their argument pointers and index ranges are valid. Passing a `NULL` argument or an invalid range could cause undefined behaviour.

The midpoint and copy-length calculations use signed integer arithmetic. An extremely large or invalid range could overflow these calculations.

## Reflection and Self Assessment

Jono: A problem I had was understanding how the existing code divides the array between threads without merging either half before its sorting thread has finished. I researched how `pthread_create` passes a `void *` argument and how `pthread_join` waits for a thread, then used this knowledge to understand why the code avoids sorting errors caused by merging before both halves are ready. The most challenging part was following the thread arguments and understanding where the code stops creating new threads, but it finally clicked when I saw that the two halves can be sorted independently and only need to synchronise before the merge. I checked the code when errors appeared and tested different input sizes, cutoff levels, and seeds to confirm that the final array was sorted correctly. The only errors encountered were issues with my Git tree, and a rebase fixed these. Overall, the testing process went well because of how simple the code was.

Nuwin: 
I implemented the structure of parallel_mergesort() by extracting the values from the argument structure and using level >= cutoff as the point where the program switches to the serial mergesort function. For recursive, calls, I created separate argument structures for the left and right halves using buildArgs(), checking that level + 1 was passed so the program eventually reaches the cutoff.Then using pthread_create(), I sorted both halves concurrently and pthread_join() to ensure both thread were completed before calling merge(). The most challenging part of understanding which argument structures should be freed: each parent call frees the child arguments it created after joining them, while the original argument is freed by the caller.  


Shahid: I implemented the merge step by comparing elements from the two sorted halves, writing the smaller elements to `B`, and copying the result back to `A`. The temporary array prevents unread values from being overwritten.

`buildArgs` initially returned `NULL`, causing a segmentation fault when `parallel_mergesort` accessed the argument. I fixed it by allocating and initialising the structure. The parent frees each child argument after joining the thread. I also added a base case for ranges with at most one element and a serial fallback if thread creation fails. I tested serial and parallel runs and compared the results against `qsort` for sizes 0–257 and cutoffs 0–6.

## Sources Used

* The Canvas modules and the supplied starter files.
* The `pthread_create` and `pthread_join` documentation for thread management.
