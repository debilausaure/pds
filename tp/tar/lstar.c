#include <errno.h>
#include <fcntl.h>
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

int main(int argc, char *argv[]) {

	if (argc > 2) {
		fprintf(stderr, "Error : Expected at most 1 argument\nUsage : %s [file.tar]\n", argv[0]);
	}

	int fd = STDIN_FILENO;
	if (argc == 2) {
		fd = open(argv[1], O_RDONLY);
		if (fd == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}
	}

	tar_header_t header;
	ssize_t bytes_read = 0;
	ssize_t total_bytes_read = 0;
	while(total_bytes_read != sizeof(tar_header_t)) {
		bytes_read = read(fd, ((char *) &header) + total_bytes_read, sizeof(tar_header_t) - total_bytes_read);
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

	if (strncmp(header.magic, TMAGIC, TMAGLEN)) {
		fprintf(stderr, "Error : got wrong magic while parsing the tar header\n");
		exit(EXIT_FAILURE);
	}

	if (strncmp(header.version, TVERSION, TVERSLEN)) {
		fprintf(stderr, "Error : got wrong magic while parsing the tar header\n");
		exit(EXIT_FAILURE);
	}

	char *endptr;
	errno = 0;
	unsigned long file_size = strtoul(header.size, &endptr, 8);

	if (errno != 0) {
		perror("strtoul");
		exit(EXIT_FAILURE);
	}

	if (endptr == header.size) {
		fprintf(stderr, "Error : Could not parse a size in field size\n");
		exit(EXIT_FAILURE);
	}

	if (*endptr != '\0')
		printf("Warning : Further characters after number in field size: \"%s\"\n", endptr);

	printf("First file size : %d\n", file_size);
	return 0;
}