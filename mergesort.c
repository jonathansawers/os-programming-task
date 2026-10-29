/**
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include "mergesort.h"

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){
	int i = leftstart, j = rightstart, k = leftstart;

	while (i <= leftend && j <= rightend) {
		B[k++] = A[i] <= A[j] ? A[i++] : A[j++];
	}
	while (i <= leftend) {
		B[k++] = A[i++];
	}
	while (j <= rightend) {
		B[k++] = A[j++];
	}
	memcpy(A + leftstart, B + leftstart,
	       (size_t)(rightend - leftstart + 1) * sizeof(*A));
}

/* this function will be called by parallel_mergesort() as its base case. */
void my_mergesort(int left, int right){
	int middle;

	// base case
	if (left >= right) {
		return;
	}

	// sort both halves before merging them
	middle = left + (right - left) / 2;
	my_mergesort(left, middle);
	my_mergesort(middle + 1, right);
	merge(left, middle, middle + 1, right);
}

/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){
	struct argument *args;
	struct argument *left_args;
	struct argument *right_args;
	pthread_t left_thread;
	pthread_t right_thread;
	int middle, left_created, right_created;

	args = (struct argument *) arg;

	if (args->left >= args->right) {
		return NULL;
	}
	// stop creating threads once branch reaches cutoff lvl
	if (args->level >= cutoff) {
		my_mergesort(args->left, args->right);
		return NULL;
	}

	middle = args->left + (args->right - args->left) / 2;
	left_args = buildArgs(args->left, middle, args->level + 1);
	right_args = buildArgs(middle + 1, args->right, args->level + 1);

	left_created = pthread_create(&left_thread, NULL, parallel_mergesort, left_args) == 0;
	right_created = pthread_create(&right_thread, NULL, parallel_mergesort, right_args) == 0;

	if (!left_created) {
		my_mergesort(left_args->left, left_args->right);
	}
	if (!right_created) {
		my_mergesort(right_args->left, right_args->right);
	}
	if (left_created) {
		pthread_join(left_thread, NULL);
	}
	if (right_created) {
		pthread_join(right_thread, NULL);
	}

	merge(args->left, middle, middle + 1, args->right);

	free(left_args);
	free(right_args);
	return NULL;
}

/* we build the argument for the parallel_mergesort function. */
struct argument * buildArgs(int left, int right, int level){
	struct argument *args = malloc(sizeof(*args));
	if (args == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	args->left = left;
	args->right = right;
	args->level = level;
	return args;
}

