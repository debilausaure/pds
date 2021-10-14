#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ustar_header.h"

unsigned long arrondi512(unsigned long n);
void get_block(int fd, char *block);
bool is_block_null(char* block);
void parse_metadata_block(char *metadata_block, sane_ustar_header_t *sane_header);
void display_metadata(sane_ustar_header_t *header);
unsigned long parse_from_octal_str(char *octal_str, unsigned max_str_size);

unsigned long arrondi512(unsigned long n) {
	unsigned long arrondi = (n >> 9) << 9; // 9 = log2(512)
	if (n != arrondi) {
		arrondi+=512;
	}
	return arrondi;
}

void display_metadata(sane_ustar_header_t *header) {
	printf("File name : %s\n", header->complete_name);
	printf("Ownership : (%d/%s) (%d/%s)\n", header->uid, header->uname, header->gid, header->gname);
}

void get_block(int fd, char *buf) {
	ssize_t bytes_read = 0;
	ssize_t total_bytes_read = 0;
	while(total_bytes_read != 512) {
		bytes_read = read(fd, buf + total_bytes_read, 512 - total_bytes_read);
		if (bytes_read == 0) {
			fprintf(stderr, "Unexpected end of file while retrieving a block\n");
			exit(EXIT_FAILURE);
		}
		if (bytes_read == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		total_bytes_read += bytes_read;
	}
}

bool is_block_null(char* block) {
	bool null_block = true;
	for (char *byte_ptr = block; byte_ptr < block + 512; byte_ptr++) {
		if(*byte_ptr != '\0') {
			null_block = false;
			break;
		}
	}

	return null_block;
}

void parse_metadata_block(char *metadata_block, sane_ustar_header_t *sane_header) {

	// checking matching magic and version
	ustar_header_t *header = (ustar_header_t *) metadata_block;
	if (strncmp(header->magic, TMAGIC, TMAGLEN)) {
		fprintf(stderr, "Warning : got wrong magic while parsing the metadata block\n");
	}
	if (strncmp(header->version, TVERSION, TVERSLEN)) {
		fprintf(stderr, "Warning : got wrong version while parsing the metadata block\n");
	}

	// retrieving the complete path of the file
	char *name_buf = sane_header->complete_name;
	if (header->prefix[0] != '\0') {
		strncpy(sane_header->complete_name, header->prefix, 155);
		sane_header->complete_name[155] = '\0';
		unsigned int prefix_len = strlen(sane_header->complete_name);
		name_buf[prefix_len] = '/';
		name_buf = sane_header->complete_name + prefix_len + 1;
	}
	strncpy(name_buf, header->name, 100);
	name_buf[100] = '\0';

	sane_header->mode  = parse_from_octal_str(header->mode,  8);
	sane_header->uid   = parse_from_octal_str(header->uid ,  8);
	sane_header->gid   = parse_from_octal_str(header->gid ,  8);
	sane_header->size  = parse_from_octal_str(header->size, 12);
	sane_header->mtime = parse_from_octal_str(header->size, 12);

	sane_header->typeflag = header->typeflag;
	/** 
	 * linkname is only valid when typeflag==LNKTYPE. It doesn't use prefix;
	 * files that are links to pathnames >100 chars long can not be stored
	 * in a tar archive.
	 */
	if (sane_header->typeflag == LNKTYPE) {
		strncpy(sane_header->linkname, header->linkname, 100);
		sane_header->linkname[100] = '\0';
	} else {
		sane_header->linkname[0] = '\0';
	}

	strncpy(sane_header->uname, header->uname, 31);
	sane_header->uname[31] = '\0';
	strncpy(sane_header->gname, header->gname, 31);
	sane_header->gname[31] = '\0';

	/**
	 * devmajor and devminor are only valid for typeflag=={BLKTYPE,CHRTYPE}.
	 */
	if (header->typeflag == BLKTYPE || header->typeflag == CHRTYPE) {
		sane_header->devmajor = parse_from_octal_str(header->devmajor, 8);
		sane_header->devminor = parse_from_octal_str(header->devminor, 8);
	}

	/**
	 * If typeflag=={LNKTYPE,SYMTYPE,DIRTYPE} then size must be 0.
	 */
	if (sane_header->typeflag == LNKTYPE ||
	    sane_header->typeflag == SYMTYPE ||
	    sane_header->typeflag == DIRTYPE) {
		if (sane_header->size != 0) {
			fprintf(stderr, "Warning : inconsistency found in archive\n"
					"'%s' : file type is inconsistent with its size\n",
					sane_header->complete_name);
		}
	}

	/**
	 * chksum contains the sum of all 512 bytes in the header block,
	 * treating each byte as an 8-bit unsigned value and treating the
	 * 8 bytes of chksum as blank characters.
	 */
	// TODO
}

unsigned long parse_from_octal_str(char *octal_str, unsigned max_str_size) {

	char *endptr;
	char *char_buffer = malloc(max_str_size + 1);
	if (char_buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	strncpy(char_buffer, octal_str, max_str_size);
	char_buffer[max_str_size] = '\0';

	errno = 0;
	unsigned long parsed_number = strtoul(char_buffer, &endptr, 8);

	if (errno != 0) {
		perror("strtoul");
		exit(EXIT_FAILURE);
	}

	if (endptr == char_buffer) {
		fprintf(stderr, "Error : Could not parse a size in field size\n");
		exit(EXIT_FAILURE);
	}

	if (*endptr != '\0')
		printf("Warning : Further characters after number in field size: \"%s\"\n", endptr);
	free(char_buffer);
	return parsed_number;
}

int main(int argc, char *argv[]) {
	if (argc > 2) {
		fprintf(stderr, "Error : Expected at most 1 argument\n"
				"Usage : \"%s file.tar\" or \"<tar stream> | %s\"\n",
			argv[0], argv[0]);
	}

	int fd;
	bool fd_is_seekable;
	if (argc == 2) {
		fd = open(argv[1], O_RDONLY);
		if (fd == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}
		fd_is_seekable = true; //probably
	} else {
		fd = STDIN_FILENO;
		fd_is_seekable = false;
	}

	/**
	 * A tar archive consists of 512-byte blocks. 
	 * Each file in the archive has a header block followed by 0+ data blocks.
	 * Two blocks of NUL bytes indicate the end of the archive.
	 */
	char block[512];
	sane_ustar_header_t sane_header;
	bool null_metadata_block_flag = false;
	while(true) {
		get_block(fd, block);
		if (is_block_null(block)) {
			if (null_metadata_block_flag) {
				// second null metadata block, end of file
				break;
			}
			null_metadata_block_flag = true;
			continue;
		}
		null_metadata_block_flag = false;
		// parse the octal ascii structure non-sense into a real structure
		parse_metadata_block(block, &sane_header);
		// display the metadata infos
		display_metadata(&sane_header);
		// skip data blocks
		unsigned bytes_to_read = arrondi512(sane_header.size);
		//if (fd_is_seekable) {
			//lseek
		//	continue;
		//}
		for (unsigned block_n = bytes_to_read / 512; block_n > 0; block_n--) {
			get_block(fd, block);
		}
	}
	exit(EXIT_SUCCESS);
}
