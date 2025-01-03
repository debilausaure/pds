#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ustar_header.h"

unsigned long arrondi512(unsigned long n);
void get_block(int fd, char *block);
bool is_block_null(char* block);
void parse_metadata_block(char *metadata_block, sane_ustar_header_t *sane_header);
void display_metadata(sane_ustar_header_t *header);
unsigned long parse_from_octal_str(char *octal_str, unsigned max_str_size);
void print_type(char file_type);

unsigned long arrondi512(unsigned long n) {
	unsigned long arrondi = (n >> 9) << 9; // 9 = log2(512)
	if (n != arrondi) {
		arrondi+=512;
	}
	return arrondi;
}

void print_access_rights(unsigned int mode, char typeflag) {
	switch(typeflag) {
		case DIRTYPE :
			printf("d");
			break;
		case LNKTYPE :
			printf("h");
			break;
		case SYMTYPE :
			printf("l");
			break;
		default:
			printf("-");
			break;
	}
	printf("%c%c%c%c%c%c%c%c%c",
        	(mode & TUREAD )      ? 'r' : '-', // file is owner readable
        	(mode & TUWRITE)      ? 'w' : '-', // file is owner writable
        	(mode & TUEXEC )      ? 'x' : '-', // file is owner executable
        	(mode & TGREAD )      ? 'r' : '-', // file is group readable
        	(mode & TGWRITE)      ? 'w' : '-', // file is group writable
        	(mode & TGEXEC )      ? 'x' : '-', // file is group executable
        	(mode & TOREAD )      ? 'r' : '-', // file is readable by others
        	(mode & TOWRITE)      ? 'w' : '-', // file is writable by others
        	(mode & TOEXEC )      ? 'x' : '-'  // file is executable by others
	);
}

void print_type(char file_type) {
	switch(file_type) {
		case REGTYPE:
		case AREGTYPE:
			printf("Regular file");
			break;
		case LNKTYPE:
			printf("Hard Link");
			break;
		case SYMTYPE:
			printf("Symbolic Link");
			break;
		case CHRTYPE:
			printf("Character special");
			break;
		case BLKTYPE:
			printf("Block special");
			break;
		case DIRTYPE:
			printf("Directory");
			break;
		case FIFOTYPE:
			printf("Named pipe");
			break;
		case CONTTYPE:
			printf("Contiguous file");
			break;
		default:
			printf("Unknown type");
	}
}

void print_time(time_t mtime) {
	char time_str[17];
	struct tm *stm = localtime(&mtime);
	if (stm == NULL) {
		perror("localtime");
		exit(EXIT_FAILURE);
	}

	if (strftime(time_str, sizeof(time_str), "%F %R", stm) == 0) {
		fprintf(stderr, "Error : could not format file modification date\n");
		exit(EXIT_FAILURE);
	}
	printf("%s", time_str);
}


void display_metadata(sane_ustar_header_t *header) {
	print_access_rights(header->mode, header->typeflag);
	printf(" %d/%d %s/%s", header->uid, header->gid, header->uname, header->gname);
	printf(" %3ld ", header->size);
	print_time(header->mtime);
	if (header->typeflag == SYMTYPE || header->typeflag == LNKTYPE)
		printf(" %s -> %s (", header->complete_name, header->linkname);
	else
		printf(" %s (", header->complete_name);
	print_type(header->typeflag);printf(")\n");
}



void get_block(int fd, char *buf) {
	ssize_t bytes_read = 0;
	ssize_t total_bytes_read = 0;
	while(total_bytes_read != 512) {
		bytes_read = read(fd, buf + total_bytes_read, 512 - total_bytes_read);
		if (bytes_read == 0) {
			fprintf(stderr, "Error : Unexpected end of file while retrieving a block\n");
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

	sane_header->mode  = parse_from_octal_str(header->mode,   8);
	sane_header->uid   = parse_from_octal_str(header->uid ,   8);
	sane_header->gid   = parse_from_octal_str(header->gid ,   8);
	sane_header->size  = parse_from_octal_str(header->size,  12);
	sane_header->mtime = parse_from_octal_str(header->mtime, 12);

	sane_header->typeflag = header->typeflag;
	/** 
	 * linkname is only valid when typeflag==LNKTYPE. (Seems likes SYMTYPE
	 * works too) It doesn't use prefix; files that are links to pathnames
	 * >100 chars long can not be stored in a tar archive.
	 */
	if (sane_header->typeflag == SYMTYPE || sane_header->typeflag == LNKTYPE) {
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
	unsigned long computed_chksum = 0;
	unsigned byte_n;
	for (byte_n = 0; byte_n < 148; byte_n++) {
		computed_chksum += ((unsigned char *)metadata_block)[byte_n];
	}
	for (/*      */; byte_n < 156; byte_n++) {
		computed_chksum += ' ';
	}
	for (/*      */; byte_n < sizeof(ustar_header_t); byte_n++) {
		computed_chksum += ((unsigned char *)metadata_block)[byte_n];
	}
	unsigned long header_chksum = parse_from_octal_str(header->chksum, 6);
	if (header_chksum != computed_chksum) {
		fprintf(stderr, "Warning : computed checksum does not match archive checksum.\n");
	}
	if (header->chksum[6] != '\0' || header->chksum[7] != ' ') {
		fprintf(stderr, "Warning : archive checksum matches but is not padded with the expected bytes.\n");
	}
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
		fprintf(stderr, "Error : Could not parse number from %s\n", char_buffer);
		exit(EXIT_FAILURE);
	}

	if (*endptr != '\0')
		fprintf(stderr, "Warning : Further unparsable characters after parsed number %ld \"%s\"\n", parsed_number, endptr);
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
		unsigned bytes_to_skip = arrondi512(sane_header.size);
		if (fd_is_seekable) {
			off_t offset = lseek(fd, bytes_to_skip, SEEK_CUR);
			// if lseek succeeded, continue parsing
			if (offset != -1)
				continue;
			// if lseek failed
			// check if it is because the file is not seekable
			if (errno == ESPIPE)
				// prevent the program from trying lseek next loops
				fd_is_seekable = false;
			else {
				perror("lseek");
				exit(EXIT_FAILURE);
			}
		}
		// fallback whenever file is not seekable
		for (unsigned block_n = bytes_to_skip / 512; block_n > 0; block_n--) {
			get_block(fd, block);
		}
	}
	exit(EXIT_SUCCESS);
}
