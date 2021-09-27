#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char **argv) {

	// checking number of args
	if (argc != 3) {
		fprintf(stderr, "Error : Expected 3 args\nUsage : %s buf_size file_path\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	// parsing buf_size
	char *endptr;
	// need this because the only way to discriminate between a valid return and an error is to check errno
	// see man 3 strtoul
	errno = 0;
	long buf_size = strtol(argv[1], &endptr, 10);

	if (errno != 0) {
		perror("strtol");
		exit(EXIT_FAILURE);
	}

	if (endptr == argv[1]) {
		fprintf(stderr, "Error : could not parse a number\n");
		exit(EXIT_FAILURE);
	}

	if (*endptr != '\0') {
		fprintf(stderr, "Warning : unexpected characters while parsing `buf_size`. Skipping\n");
	}

	// checking that buf_size > 0
	if (buf_size <= 0) {
		fprintf(stderr, "Error : buf_size needs to be strictly positive\n");

	}

	// opening the file
	int fd = open(argv[2], O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	// allocating the buffer
	char *buf = malloc(buf_size);
	if (buf == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	bool end_of_file_reached = false;
	while (!end_of_file_reached) {
		ssize_t bytes_read = 0;
		while (bytes_read != buf_size) {
			ssize_t read_rc = read(fd, buf + bytes_read, buf_size - bytes_read);
			// check if read failed
			if (read_rc == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			// check if we reached the end of file
			if (read_rc == 0) {
				end_of_file_reached = true;
				break;
			}
			bytes_read += read_rc;
		}
		// buffer is full or we reached end of file
		ssize_t bytes_written = 0;
		while (bytes_written != bytes_read) {
			ssize_t write_rc = write(STDOUT_FILENO, buf + bytes_written, bytes_read - bytes_written);
			// check if write failed
			if (write_rc <= 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
			bytes_written += write_rc;
		}
	}

	close(fd);
	free(buf);

	exit(EXIT_SUCCESS);
}
