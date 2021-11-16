#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uintptr_t fib(uintptr_t n);
typedef void *(thread_signature) (void *);

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Error : expected one argument.\nUsage : %s n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	errno = 0;    /* To distinguish success/failure after call */
	char *endptr;
	long int n = strtol(argv[1], &endptr, 10);
	
	/* Check for various possible errors. */
	if (errno != 0) {
		perror("strtol");
		exit(EXIT_FAILURE);
	}

	if (endptr == argv[1]) {
		fprintf(stderr, "Unable to parse an integer\n");
		exit(EXIT_FAILURE);
	}

	int res = fib(n);

	printf("fib(%ld)=%d\n", n, res);
	exit(EXIT_SUCCESS);
}

uintptr_t fib(uintptr_t n) {
	if (n < 2) {
		return n;
	}

	pthread_t thread1;
	int create_th1_rc = pthread_create(&thread1, NULL, (thread_signature *) &fib, (void *) n-1);
	if (create_th1_rc != 0) {
		fprintf(stderr, "pthread_create failed with error code :%d\n", create_th1_rc);
		exit(EXIT_FAILURE);
	}

	pthread_t thread2;
	int create_th2_rc = pthread_create(&thread2, NULL, (thread_signature *) &fib, (void *) n-2);
	if (create_th2_rc != 0) {
		fprintf(stderr, "pthread_create failed with error code :%d\n", create_th1_rc);
		exit(EXIT_FAILURE);
	}

	uintptr_t x;
	uintptr_t y;
	int join_th1_rc = pthread_join(thread1, (void **) &x);
	if (join_th1_rc != 0) {
		fprintf(stderr, "pthread_join failed with error code :%d\n", create_th1_rc);
		exit(EXIT_FAILURE);
	}
	int join_th2_rc = pthread_join(thread2, (void **) &y);
	if (join_th2_rc != 0) {
		fprintf(stderr, "pthread_join failed with error code :%d\n", create_th1_rc);
		exit(EXIT_FAILURE);
	}
	return x + y;
}
