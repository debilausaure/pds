/* 
 * La plupart du code de ce fichier a été honteusement dérobé
 * à la page de manuel de stat (man 2 stat)
 */

/* 
 * Extrait de la page de manuel de getpwuid (man 3 getpwuid)
 *
 * The getpwuid() function returns a pointer to a structure containing the broken-out
 * fields of the record in the password database that matches the user ID uid.
 * 
 *  struct passwd {
 *      char   *pw_name;       // username
 *      char   *pw_passwd;     // user password
 *      uid_t   pw_uid;        // user ID
 *      gid_t   pw_gid;        // group ID
 *      char   *pw_gecos;      // user information
 *      char   *pw_dir;        // home directory
 *      char   *pw_shell;      // shell program
 *  };
 */

/* 
 * Extrait de la page de manuel de getgrgid (man 3 getgrgid)
 *
 * The getgrgid() function returns a pointer to a structure containing the broken-out
 * fields of the record in the group database that matches the group ID gid.
 *
 *  struct group {
 *      char   *gr_name;       // group name
 *      char   *gr_passwd;     // group password
 *      gid_t   gr_gid;        // group ID
 *      char  **gr_mem;        // NULL-terminated array of pointers
 *                               to names of group members
 *  };
 */

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

void format_access_rights(unsigned int st_mode, char *buf);

int
main(int argc, char *argv[])
{
    // check number of args
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // check that stat did not fail
    struct stat sb;
    errno = 0;
    if (lstat(argv[1], &sb) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    struct passwd *pw_ptr;
    errno = 0;
    pw_ptr = getpwuid(sb.st_uid);
    if (pw_ptr == NULL) {
        perror("getpwuid");
	exit(EXIT_FAILURE);
    }

    struct group *grp_ptr;
    errno = 0;
    grp_ptr = getgrgid(sb.st_gid);
    if(grp_ptr == NULL) {
        perror("getgrgid");
	exit(EXIT_FAILURE);
    }

    printf("File : %s\t", argv[1]);
    printf("File type: ");
    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:  printf("block device\n");            break;
        case S_IFCHR:  printf("character device\n");        break;
        case S_IFDIR:  printf("directory\n");               break;
        case S_IFIFO:  printf("FIFO/pipe\n");               break;
        case S_IFLNK:  printf("symlink\n");                 break;
        case S_IFREG:  printf("regular file\n");            break;
        case S_IFSOCK: printf("socket\n");                  break;
        default:       printf("unknown?\n");                break;
    }

    printf("File size : %ld bytes\t", sb.st_size);
    printf("Blocks: %ld (512 bytes)\t", sb.st_blocks);
    printf("I/O block size: %ld bytes\n", sb.st_blksize);

    printf("Peripheral: [%x,%x]h/[%d,%d]d\t", major(sb.st_dev), minor(sb.st_dev), major(sb.st_dev), minor(sb.st_dev));
    printf("I-node: %lu\t", sb.st_ino);
    printf("Link count: %lu\n", sb.st_nlink);

    char access_rights_str[11];
    format_access_rights(sb.st_mode, access_rights_str);
    printf("Access rights: (%04o/%s)\t", sb.st_mode & (~S_IFMT), access_rights_str);

    printf("UID: (%u/%s)\tGID: (%u/%s)\n", sb.st_uid, pw_ptr->pw_name, sb.st_gid, grp_ptr->gr_name);

    if(pw_ptr->pw_gecos != NULL) {
        printf("Additional owner infos: %s\n", pw_ptr->pw_gecos);
    }

    printf("Last status change:       %s", ctime(&sb.st_ctime));
    printf("Last file access:         %s", ctime(&sb.st_atime));
    printf("Last file modification:   %s", ctime(&sb.st_mtime));

    exit(EXIT_SUCCESS);
}


void format_access_rights(unsigned int st_mode, char *buf) {
    snprintf(buf, 11, "%c%c%c%c%c%c%c%c%c%c",
        (st_mode & S_IFMT ) == S_IFDIR ? 'd' : '-', // file is a directory
        (st_mode & S_IRUSR)            ? 'r' : '-', // file is owner readable
        (st_mode & S_IWUSR)            ? 'w' : '-', // file is owner writable
        (st_mode & S_IXUSR)            ? 'x' : '-', // file is owner executable
        (st_mode & S_IRGRP)            ? 'r' : '-', // file is group readable
        (st_mode & S_IWGRP)            ? 'w' : '-', // file is group writable
        (st_mode & S_IXGRP)            ? 'x' : '-', // file is group executable
        (st_mode & S_IROTH)            ? 'r' : '-', // file is readable by others
        (st_mode & S_IWOTH)            ? 'w' : '-', // file is writable by others
        (st_mode & S_IXOTH)            ? 'x' : '-'  // file is executable by others
    );
}
