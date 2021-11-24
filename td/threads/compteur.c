#include <stdio.h>
#include <pthread.h>

#define n_threads 32

int unique() {
    static int count = 0; /* variable globale à visibilité locale */
    count++;
    return count;
}

int main() {

	pthread_t thread[n_threads];
	int count[n_threads];
	for (unsigned i = 0; i < n_threads; i++)
		pthread_create(&thread[i], NULL, &unique, NULL);
	for (unsigned i = 0; i < n_threads; i++)
		pthread_join(thread[i], &count[i]);
	for (unsigned i = 0; i < n_threads; i++)
		printf("Count for thread %d : %d\n", i, count[i]);
}
