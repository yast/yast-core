

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 *
 *  Output architecture of an elf file.
 */


#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <endian.h>
#include <byteswap.h>


unsigned char e_ident[EI_NIDENT];
uint16_t e_class;
uint16_t e_data;
uint16_t e_machine;


uint16_t get_uint16 (uint16_t);

uint16_t
get_uint16 (uint16_t i)
{
#if __BYTE_ORDER == __BIG_ENDIAN
    if (e_data == ELFDATA2LSB)
	i = bswap_16 (i);
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
    if (e_data == ELFDATA2MSB)
	i = bswap_16 (i);
#endif

    return i;
}


char* machine_name (void);

char*
machine_name (void)
{
    switch (e_machine)
    {
	case EM_386:	return "i386";
	case EM_IA_64:	return "ia64";
	case EM_X86_64:	return "x86_64";
	case EM_PPC:	return "ppc";
	case EM_PPC64:	return "ppc64";
	case EM_S390:	return "s390";
	case EM_SPARC:	return "sparc";
	case EM_ALPHA:	return "axp";
	case EM_ARM:	return "arm";
    }

    return "unknown";
}


int read_header (char*);

int
read_header (char* filename)
{
    FILE* file = fopen (filename, "rb");
    if (file == NULL)
    {
	fprintf (stderr, "open failed\n");
	return 0;
    }

    if (fread (e_ident, EI_NIDENT, 1, file) != 1)
    {
	fprintf (stderr, "read failed\n");
	fclose (file);
	return 0;
    }

    if (e_ident[EI_MAG0] != ELFMAG0 || e_ident[EI_MAG1] != ELFMAG1 ||
	e_ident[EI_MAG2] != ELFMAG2 || e_ident[EI_MAG3] != ELFMAG3)
    {
	fprintf (stderr, "wrong magic\n");
	fclose (file);
	return 0;
    }

    e_class = e_ident[EI_CLASS];
    if (e_class != ELFCLASS32 && e_class != ELFCLASS64)
    {
	fprintf (stderr, "unknown class\n");
	fclose (file);
	return 0;
    }

    e_data = e_ident[EI_DATA];
    if (e_data != ELFDATA2LSB && e_data != ELFDATA2MSB)
    {
	fprintf (stderr, "unknown data\n");
	fclose (file);
	return 0;
    }

    if (e_class == ELFCLASS32)
    {
	Elf32_Ehdr ehdr32;
	if (fread (&ehdr32.e_type, sizeof (ehdr32) - EI_NIDENT, 1, file) != 1)
	{
	    fprintf (stderr, "read failed\n");
	    fclose (file);
	    return 0;
	}

	e_machine = get_uint16 (ehdr32.e_machine);
    }
    else
    {
	Elf64_Ehdr ehdr64;
	if (fread (&ehdr64.e_type, sizeof (ehdr64) - EI_NIDENT, 1, file) != 1)
	{
	    fprintf (stderr, "read failed\n");
	    fclose (file);
	    return 0;
	}

	e_machine = get_uint16 (ehdr64.e_machine);
    }

    fclose (file);

    return 1;
}


int
main (int argc, char **argv)
{
    if (argc != 2)
    {
	fprintf (stderr, "usage: file\n");
	exit (EXIT_FAILURE);
    }

    if (!read_header (argv[1]))
	exit (EXIT_FAILURE);

    printf ("%s\n", machine_name ());

    exit (EXIT_SUCCESS);
}
