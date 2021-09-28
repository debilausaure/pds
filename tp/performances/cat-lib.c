#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char **argv) {

	// checking number of args
	if (argc != 2) {
		fprintf(stderr, "Error : Expected 1 arg\nUsage : %s file_path\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// opening the file
	FILE *stream = fopen(argv[1], "r");
	if (stream == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	int c;
	while ((c = fgetc(stream)) != EOF) {
		fputc(c, stdout);
	}

	fclose(stream);

	exit(EXIT_SUCCESS);
}
