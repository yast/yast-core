/* Taken from Ted's losetup.c - Mitch <m.dsouza@mrc-apu.cam.ac.uk> */
/* Added vfs mount options - aeb - 960223 */
/* Removed lomount - aeb - 960224 */

/* 1999-02-22 Arkadiusz Mi¶kiewicz <misiek@pld.ORG.PL>
 * - added Native Language Support
 * Sun Mar 21 1999 - Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 * - fixed strerr(errno) in gettext calls
 */

#define PROC_DEVICES	"/proc/devices"

/*
 * losetup.c - setup and control loop devices
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "y2loop.h"
#include "y2lomount.h"
#include "rmd160.h"
//#include "xstrncpy.h"
//#include "nls.h"

extern int verbose;
extern char *xstrdup (const char *s);	/* not: #include "sundries.h" */
extern void error (const char *fmt, ...);	/* idem */
void xstrncpy(char *dest, const char *src, size_t n);

void
xstrncpy(char *dest, const char *src, size_t n) {
	strncpy(dest, src, n-1);
	dest[n-1] = 0;
}

struct crypt_type_struct {
	int id;
	char *name;
	int keylength;
} crypt_type_tbl[] = {
	{ LO_CRYPT_NONE, "no",0 },
	{ LO_CRYPT_NONE, "none",0 },
	{ LO_CRYPT_XOR, "xor",0 },
	{ LO_CRYPT_DES, "DES",8 },
	{ LO_CRYPT_FISH2, "twofish",20 },
	{ LO_CRYPT_BLOW, "blowfish",20 },
	{ LO_CRYPT_CAST128, "cast128", 16},
	{ LO_CRYPT_SERPENT, "serpent", 16},
	{ LO_CRYPT_MARS, "mars",16 },
	{ LO_CRYPT_RC6, "rc6",16 },
	{ LO_CRYPT_DES_EDE3, "DES_EDE3",24},
	{ LO_CRYPT_DFC, "dfc",16 },
	{ LO_CRYPT_IDEA, "idea",16},
	{ -1, NULL,0   }
};

char * clkey;

static int
crypt_type (const char *name) {
	int i;

	if (name) {
		for (i = 0; crypt_type_tbl[i].id != -1; i++)
			if (!strcasecmp (name, crypt_type_tbl[i].name))
				return crypt_type_tbl[i].id;
	}
	return -1;
}


#define SIZE(a) (sizeof(a)/sizeof(a[0]))
#define HASHLENGTH 20
#define PASSWDBUFFLEN 130 /* getpass returns only max. 128 bytes, see man getpass */

int
xset_loop (const char *device, const char *file, int offset,
	   const char *encryption, int *loopro, char *c_passwd);

int
xset_loop (const char *device, const char *file, int offset,
	  const char *encryption, int *loopro, char *c_passwd) {

	struct loop_info loopinfo;
	int fd, ffd, mode, i;
	int keylength;
	char *pass;
	char keybits[2*HASHLENGTH];
	char passwdbuff[PASSWDBUFFLEN];

	mode = (*loopro ? O_RDONLY : O_RDWR);
	if ((ffd = open (file, mode)) < 0) {
		if (!*loopro && errno == EROFS)
			ffd = open (file, mode = O_RDONLY);
		if (ffd < 0) {
			perror (file);
			return 1;
		}
	}
	if ((fd = open (device, mode)) < 0) {
		perror (device);
		return 1;
	}
	*loopro = (mode == O_RDONLY);

	memset (&loopinfo, 0, sizeof (loopinfo));
	xstrncpy (loopinfo.lo_name, file, LO_NAME_SIZE);
	if (encryption && (loopinfo.lo_encrypt_type = crypt_type (encryption))
	    < 0) {
		fprintf (stderr, "Unsupported encryption type %s\n",
			 encryption);
		return 1;
	}
	loopinfo.lo_offset = offset;

#ifdef MCL_FUTURE
	/*
	 * Oh-oh, sensitive data coming up. Better lock into memory to prevent
	 * passwd etc being swapped out and left somewhere on disk.
	 */

	if(mlockall(MCL_CURRENT | MCL_FUTURE)) {
		perror("memlock");
		fprintf(stderr, "Couldn't lock into memory, exiting.\n");
		exit(1);
	}
#endif

	switch (loopinfo.lo_encrypt_type) {
	case LO_CRYPT_NONE:
		loopinfo.lo_encrypt_key_size = 0;
		break;
	case LO_CRYPT_XOR:
	        pass = c_passwd;
		xstrncpy (loopinfo.lo_encrypt_key, pass, LO_KEY_SIZE);
		loopinfo.lo_encrypt_key_size = strlen(loopinfo.lo_encrypt_key);
		break;
	case LO_CRYPT_DES:
	        printf("WARNING: Use of DES is not implemented.\n");
		exit(1);
		break;
	case LO_CRYPT_FISH2:
	case LO_CRYPT_BLOW:
	case LO_CRYPT_IDEA:
	case LO_CRYPT_CAST128:
	case LO_CRYPT_SERPENT:
	case LO_CRYPT_MARS:
	case LO_CRYPT_RC6:
	case LO_CRYPT_DES_EDE3:
	case LO_CRYPT_DFC:
		pass = c_passwd;
		for (i=0; i<PASSWDBUFFLEN; i++) passwdbuff[i] = '\0';
		strncpy(passwdbuff+1,pass,PASSWDBUFFLEN-1);
		// for (i=0; i<PASSWDBUFFLEN; i++)
		//   printf( "\n-char %02d: %d %c", i, (int)passwdbuff[i], passwdbuff[i]);
		passwdbuff[0] = 'A';
		rmd160_hash_buffer(keybits,pass,strlen(pass));
		rmd160_hash_buffer(keybits+HASHLENGTH,passwdbuff,strlen(pass)+1);
		memcpy((char*)loopinfo.lo_encrypt_key,keybits,2*HASHLENGTH);
		keylength=0;
		for(i=0; crypt_type_tbl[i].id != -1; i++){
		         if(loopinfo.lo_encrypt_type == crypt_type_tbl[i].id){
			         keylength = crypt_type_tbl[i].keylength;
				 break;
			 }
		}
		loopinfo.lo_encrypt_key_size=keylength;
		
		break;
	default:
		fprintf (stderr,
			 "Don't know how to get key for encryption system %d\n",
			 loopinfo.lo_encrypt_type);
		return 1;
	}
	if (ioctl (fd, LOOP_SET_FD, ffd) < 0) {
		perror ("ioctl: LOOP_SET_FD");
		return 1;
	}
	if (ioctl (fd, LOOP_SET_STATUS, &loopinfo) < 0) {
		(void) ioctl (fd, LOOP_CLR_FD, 0);
		perror ("ioctl: LOOP_SET_STATUS");
		return 1;
	}
	close (fd);
	close (ffd);
	if (verbose > 1)
		printf("set_loop(%s,%s,%d): success\n",
		       device, file, offset);
	return 0;
}


#include <getopt.h>
#include <stdarg.h>

int verbose = 0;
static char *progname;


char *
xstrdup (const char *s) {
	char *t;

	if (s == NULL)
		return NULL;

	t = strdup (s);

	if (t == NULL) {
		fprintf(stderr, "not enough memory");
		exit(1);
	}

	return t;
}

void
error (const char *fmt, ...) {
	va_list args;

	va_start (args, fmt);
	vfprintf (stderr, fmt, args);
	va_end (args);
	fprintf (stderr, "\n");
}


static void
usage(void) {
	struct crypt_type_struct *c;
	fprintf(stderr, "usage:\n\
  %s [ -e encryption ] [ -o offset ] [ -p passwd ] loop_device file # setup\n",
		progname);
	fprintf(stderr, "    where encryption is one of:\n");
	c = &crypt_type_tbl[0];
	while(c->name) {
		fprintf(stderr, "       %s\n", c->name);
		c++;
	}
	exit(1);
}

int
main(int argc, char **argv) {
	char *offset, *encryption, *c_passwd;
	int delete,off,c;
	int res = 0;
	int ro = 0;


	delete = off = 0;
	offset = encryption = clkey = c_passwd = NULL;
	progname = argv[0];
	while ((c = getopt(argc,argv,"de:p:vk:")) != EOF)
        {
		switch (c) {
		case 'd':
			delete = 1;
			break;
		case 'e':
			encryption = optarg;
			break;
		case 'p':
			c_passwd = optarg;
			break;
		case 'o':
			offset = optarg;
			break;
		case 'k':
			clkey = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
		}
	}
	if (argc == 1) usage();
	if ((delete && (argc != optind+1 || encryption || offset)) ||
	    (!delete && (argc < optind+1 || argc > optind+2)))
		usage();
	if (argc == optind+1)
        {
            usage();
	}
        else
        {
		if (offset && sscanf(offset,"%d",&off) != 1)
			usage();
		res = xset_loop(argv[optind],argv[optind+1],off,encryption,&ro, c_passwd);
	}
	return res;
}

