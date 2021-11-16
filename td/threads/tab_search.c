#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

intptr_t search(int *array, unsigned int size, int v);
void *search_wrapper(void *search_args);

struct search_args {
	int *array;
	unsigned int size;
	int v;
};


int int_array[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

int main(int argc, char *argv[]) {
	if(argc != 2) {
		fprintf(stderr, "Error : expected exactly one argument.\nUsage : %s n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	errno = 0;
	char *endptr;
	long v = strtol(argv[1], &endptr, 10);

	if (errno != 0) {
		perror("strtol");
		exit(EXIT_FAILURE);
	}

	if (endptr == argv[1]) {
		fprintf(stderr, "Unable to parse a number\n");
		exit(EXIT_FAILURE);
	}

	intptr_t index = search(int_array, 32, v);

	printf("Value located in int_array at index : %ld\n", index);
}


void *search_wrapper(void *search_args) {
	struct search_args *args = (struct search_args *) search_args;
	intptr_t index = search(args->array, args->size, args->v);
	return (void *) index;
}

intptr_t search(int *array, unsigned int size, int v) {
	//printf("search(array + %ld, %d, %d)\n", array - int_array, size, v);
	if (size == 1) {
		if (*array == v)
			return 0;
		return -1;
	}
	else {
		struct search_args thread_args;
		thread_args.array = array;
		thread_args.size = size / 2;
		thread_args.v = v;

		pthread_t half_array_thread;
		int create_rc = pthread_create(&half_array_thread, NULL, &search_wrapper, &thread_args);
		if (create_rc != 0) {
			fprintf(stderr, "pthread_create failed with error : %d\n", create_rc);
			exit(EXIT_FAILURE);
		}

		intptr_t index_self = search(array + size / 2, size - size / 2, v);

		intptr_t index_thread;
		int join_rc = pthread_join(half_array_thread, (void **) &index_thread);
		if (join_rc != 0) {
			fprintf(stderr, "pthread_join failed with error : %d\n", join_rc);
			exit(EXIT_FAILURE);
		}

		if (index_thread != -1) {
			return index_thread;
		}
		if (index_self != -1) {
			return index_self + size / 2;
		}
		return -1;
	}
}
