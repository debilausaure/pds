typedef struct ustar_header_s
{                    /* Byte offset    Field type                 */
  char name[100];            /*   0    NUL-terminated if NUL fits */
  char mode[8];              /* 100                               */
  char uid[8];               /* 108                               */
  char gid[8];               /* 116                               */
  char size[12];             /* 124                               */
  char mtime[12];            /* 136                               */
  char chksum[8];            /* 148                               */
  char typeflag;             /* 156    see below                  */
  char linkname[100];        /* 157    NUL-terminated if NUL fits */
  char magic[6];             /* 257    must be TMAGIC (NUL term.) */
  char version[2];           /* 263    must be TVERSION           */
  char uname[32];            /* 265    NUL-terminated             */
  char gname[32];            /* 297    NUL-terminated             */
  char devmajor[8];          /* 329                               */
  char devminor[8];          /* 337                               */
  char prefix[155];          /* 345    NUL-terminated if NUL fits */
                             /* 500                               */
/* If the first character of prefix is '\0', the file name is name;
   otherwise, it is prefix/name.  Files whose pathnames don't fit in
   that length can not be stored in a tar archive.  */
} ustar_header_t;

typedef struct sane_ustar_header_s {
	char complete_name[257];
	mode_t mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	unsigned mtime;
	char chksum[8];
	char typeflag;
	char linkname[101];
	char uname[32];
	char gname[32];
	unsigned devmajor;
	unsigned devminor;
} sane_ustar_header_t;

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

/* Mode bitmasks */

#define TSUID	04000
#define TSGID	02000
#define TUREAD	00400
#define TUWRITE	00200
#define TUEXEC	00100
#define TGREAD	00040
#define TGWRITE	00020
#define TGEXEC	00010
#define TOREAD	00004
#define TOWRITE	00002
#define TOEXEC	00001

/* The values for typeflag:
   Values 'A'-'Z' are reserved for custom implementations.
   All other values are reserved for future POSIX.1 revisions.  */

#define REGTYPE		'0'	/* Regular file (preferred code).  */
#define AREGTYPE	'\0'	/* Regular file (alternate code).  */
#define LNKTYPE		'1'	/* Hard link.  */
#define SYMTYPE		'2'	/* Symbolic link (hard if not supported).  */
#define CHRTYPE		'3'	/* Character special.  */
#define BLKTYPE		'4'	/* Block special.  */
#define DIRTYPE		'5'	/* Directory.  */
#define FIFOTYPE	'6'	/* Named pipe.  */
#define CONTTYPE	'7'	/* Contiguous file */
 /* (regular file if not supported).  */

