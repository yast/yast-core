

/*
 * Author: Arvin Schnell <arvin@suse.de>
 */


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <xcrypt.h>

#include "y2log.h"
#include "y2crypt.h"


static int
read_loop (int fd, char* buffer, int count)
{
    int offset, block;

    offset = 0;
    while (count > 0)
    {
	block = read (fd, &buffer[offset], count);

	if (block < 0)
	{
	    if (errno == EINTR)
		continue;
	    return block;
	}

	if (!block)
	    return offset;

	offset += block;
	count -= block;
    }

    return offset;
}


static char*
make_crypt_salt (const char* crypt_prefix, int crypt_rounds)
{
#define CRYPT_GENSALT_OUTPUT_SIZE (7 + 22 + 1)

#ifndef RANDOM_DEVICE
#define RANDOM_DEVICE "/dev/urandom"
#endif

    int fd = open (RANDOM_DEVICE, O_RDONLY);
    if (fd < 0)
    {
	y2error ("Can't open %s for reading: %s\n", RANDOM_DEVICE,
		 strerror (errno));
	return 0;
    }

    char entropy[16];
    if (read_loop (fd, entropy, sizeof(entropy)) != sizeof(entropy))
    {
	close (fd);
	y2error ("Unable to obtain entropy from %s\n", RANDOM_DEVICE);
	return 0;
    }

    close (fd);

    char output[CRYPT_GENSALT_OUTPUT_SIZE];
    char* retval = crypt_gensalt_rn (crypt_prefix, crypt_rounds, entropy,
				     sizeof (entropy), output, sizeof (output));

    memset (entropy, 0, sizeof (entropy));

    if (!retval)
    {
	y2error ("Unable to generate a salt, check your crypt settings.\n");
	return 0;
    }

    return strdup (retval);
}


bool
crypt_pass (string unencrypted, crypt_t use_crypt, string* encrypted)
{
    struct crypt_data output;
    memset (&output, 0, sizeof (output));

    char* salt;
    char* newencrypted = 0;

    switch (use_crypt)
    {
	case CRYPT:
 	    salt = make_crypt_salt ("", 0);
	    if (!salt)
	    {
		y2error ("Cannot create salt for standard crypt");
		return false;
	    }
	    newencrypted = crypt_rn (unencrypted.c_str (), salt, &output,
				     sizeof (output));
	    free (salt);
	    break;

	case MD5:
	    salt = make_crypt_salt ("$1$", 0);
	    if (!salt)
	    {
		y2error ("Cannot create salt for MD5 crypt");
		return false;
	    }
	    newencrypted = crypt_rn (unencrypted.c_str (), salt, &output,
				     sizeof (output));
	    free (salt);
	    break;

	case BIGCRYPT:
	    salt = make_crypt_salt ("", 0);
	    if (!salt)
	    {
		y2error ("Cannot create salt for bigcrypt");
		return false;
	    }
	    newencrypted = bigcrypt (unencrypted.c_str (), salt);
	    free (salt);
	    break;

	case BLOWFISH:
	    salt = make_crypt_salt ("$2y$", 0);
	    if (!salt)
	    {
		y2error ("Cannot create salt for blowfish crypt");
		return false;
	    }
	    newencrypted = crypt_rn (unencrypted.c_str (), salt, &output,
				     sizeof (output));
	    free (salt);
	    break;

	default:
	    y2error ("Don't know crypt type %d", use_crypt);
	    return false;
    }

    if (!newencrypted
    /* catch retval magic by ow-crypt/libxcrypt */
    || !strcmp(newencrypted, "*0") || !strcmp(newencrypted, "*1"))
    {
	y2error ("crypt_r () returns 0 pointer");
	return false;
    }

    *encrypted = string (newencrypted);
    return true;
}
