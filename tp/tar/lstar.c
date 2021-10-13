#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ustar_header.h"

unsigned long arrondi512(unsigned long n) {
	unsigned long arrondi = (n >> 9) << 9; // 9 = log2(512)
	if (n != arrondi) {
		arrondi+=512;
	}
	return arrondi;
}

ssize_t parse_metadata(int fd, char *buf) {
	ssize_t bytes_read = 0;
	ssize_t total_bytes_read = 0;
	while(total_bytes_read != 512) {
		bytes_read = read(fd, buf + total_bytes_read, 512 - total_bytes_read);
		if (bytes_read == 0) {
			fprintf(stderr, "Unexpected end of file while parsing header\n");
			exit(EXIT_FAILURE);
		}
		if (bytes_read == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		total_bytes_read += bytes_read;
	}

	bool empty_metadata = true;
	for (char *byte_ptr = buf; byte_ptr < buf + 512; byte_ptr++) {
		if(*byte_ptr != '\0') {
			empty_metadata = false;
			break;
		}
	}

	if (empty_metadata) {
		return -1;
	}

	tar_header_t *header = (tar_header_t *) buf;

	if (strncmp(header->magic, TMAGIC, TMAGLEN)) {
		fprintf(stderr, "Error : got wrong magic while parsing an archive header\n");
		exit(EXIT_FAILURE);
	}

	if (strncmp(header->version, TVERSION, TVERSLEN)) {
		fprintf(stderr, "Error : got wrong magic while parsing an archive header\n");
		exit(EXIT_FAILURE);
	}

	char *endptr;
	errno = 0;
	unsigned long file_size = strtoul(header->size, &endptr, 8);

	if (errno != 0) {
		perror("strtoul");
		exit(EXIT_FAILURE);
	}

	if (endptr == header->size) {
		fprintf(stderr, "Error : Could not parse a size in field size\n");
		exit(EXIT_FAILURE);
	}

	if (*endptr != '\0')
		printf("Warning : Further characters after number in field size: \"%s\"\n", endptr);

	printf("File size : %d\n", file_size);
	return arrondi512(file_size);
}

void skip_file_data(int fd, char *buf, ssize_t bytes_to_skip) {
	while(bytes_to_skip != 0) {
		ssize_t bytes_read = read(fd, buf, bytes_to_skip < 512 ? bytes_to_skip : 512);
		if (bytes_read == 0) {
			fprintf(stderr, "Unexpected end of file while skipping file data\n");
			exit(EXIT_FAILURE);
		}
		if (bytes_read == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		bytes_to_skip -= bytes_read;
	}
}

int main(int argc, char *argv[]) {

	if (argc > 2) {
		fprintf(stderr, "Error : Expected at most 1 argument\n"
				"Usage : %s [file.tar]\n", argv[0]);
	}

	int fd = STDIN_FILENO;
	if (argc == 2) {
		fd = open(argv[1], O_RDONLY);
		if (fd == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}
	}

	char buf[512];

	while(true) {
		ssize_t bytes_to_skip = parse_metadata(fd, buf);
		if (bytes_to_skip == -1) {
			break;
		}
		skip_file_data(fd, buf, bytes_to_skip);
	}

	exit(EXIT_SUCCESS);
}
