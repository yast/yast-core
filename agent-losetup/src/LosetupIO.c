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

#define PROC_DEVICES	"/proc/devices"

/*
 * losetup.c - setup and control loop devices
 */

#include "y2loop.h"
#include "y2lomount.h"
#include "rmd160.h"
#include "LosetupIO.h"



#define SIZE(a)       (sizeof(a)/sizeof(a[0]))
#define HASHLENGTH    20
#define PASSWDBUFFLEN 130 /* getpass returns only max. 128 bytes, see man getpass */
#define LOGFILE       "/var/log/YaST2/y2log"
#define LOGMAXLEN     1024



static void  xstrncpy  (char 		*dest,
		 const char 	*src,
		 size_t 	n);

static void logy2(char *dest);

char logbuffer[LOGMAXLEN];

extern int verbose;
char * clkey;



static void logy2(char *dest)
{
   FILE *logfile = fopen (LOGFILE , "a");
   if (logfile)
   {
      fprintf (logfile, dest );
      fclose(logfile);
   }
}



struct crypt_type_struct
{
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


static void xstrncpy(char *dest, const char *src, size_t n)
{
   strncpy(dest, src, n-1);
   dest[n-1] = 0;
}

static int crypt_type (const char *name)
{
   int i;

   if (name)
   {
      for (i = 0; crypt_type_tbl[i].id != -1; i++)
	 if (!strcasecmp (name, crypt_type_tbl[i].name))
	    return crypt_type_tbl[i].id;
   }
   return -1;
}


int xset_loop (const char *device,
	       const char *file,
	       int   	   offset,
	       const char *encryption,
	       int 	  *loopro,
	       const char *c_passwd)
{
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
	 sprintf( logbuffer, "could not open %s", file);
	 logy2( logbuffer );
	 return 1;
      }
   }
   if ((fd = open (device, mode)) < 0) {
      sprintf( logbuffer, "open: %s", device );
      logy2( logbuffer );
      return 1;
   }
   *loopro = (mode == O_RDONLY);

   memset (&loopinfo, 0, sizeof (loopinfo));
   xstrncpy (loopinfo.lo_name, file, LO_NAME_SIZE);
   if (encryption && (loopinfo.lo_encrypt_type = crypt_type (encryption))
       < 0) {
	 sprintf( logbuffer, "Unsupported encryption type %s",
		  encryption);
	 logy2( logbuffer );
      return 1;
   }
   loopinfo.lo_offset = offset;


#if 0
#ifdef MCL_FUTURE
   /*
    * Oh-oh, sensitive data coming up. Better lock into memory to prevent
    * passwd etc being swapped out and left somewhere on disk.
    */

   if(mlockall(MCL_CURRENT | MCL_FUTURE)) {
      	 sprintf( logbuffer, "could not open %s", file);
	 logy2( "Couldn't lock into memory, exiting");
      return(1);
   }
#endif
#endif


   switch (loopinfo.lo_encrypt_type) {
      case LO_CRYPT_NONE:
	 loopinfo.lo_encrypt_key_size = 0;
	 break;
      case LO_CRYPT_XOR:
	 pass = strdup(c_passwd);
	 xstrncpy (loopinfo.lo_encrypt_key, pass, LO_KEY_SIZE);
	 loopinfo.lo_encrypt_key_size = strlen(loopinfo.lo_encrypt_key);
	 break;
      case LO_CRYPT_DES:
	 logy2( "WARNING: Use of DES is not implemented.");
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
	 pass = strdup(c_passwd);
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
	 sprintf( logbuffer,"Don't know how to get key for encryption system %d",
		  loopinfo.lo_encrypt_type);
	 logy2( logbuffer );
	 return 1;
   }
   if (ioctl (fd, LOOP_SET_FD, ffd) < 0) {
	 logy2("ioctl: LOOP_SET_FD");
      return 1;
   }
   if (ioctl (fd, LOOP_SET_STATUS, &loopinfo) < 0) {
      (void) ioctl (fd, LOOP_CLR_FD, 0);
	 logy2( "ioctl: LOOP_SET_STATUS");
      return 1;
   }
   close (fd);
   close (ffd);

   sprintf( logbuffer, "set_loop(%s,%s,%d): success\n",
			   device, file, offset);
   logy2( logbuffer );

   return 0;
}

