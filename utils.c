#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "const.h"
#include "utils.h"
#include "error.h"

unsigned int
parse_args (struct celp_options *option, int argc, char **argv)
{
	int c;
	int user_mode = 0;

	while ((c = getopt (argc, argv, "cdehi:o:rvw")) != -1)
	{
		switch (c)
		{
			case 'c':
				option->op = OP_CODE;
				break;

			case 'd':
				option->op = OP_DECODE;
				break;

			case 'e':
				option->op = OP_ECHO;
				break;

			case 'h':
				show_usage (argv);
				exit (ERROR_OK);

			case 'i':
				option->ifilename = optarg;
				if (user_mode == 0)
					option->mode = MODE_WAVE;
				break;

			case 'o':
				option->ofilename = optarg;
				if (user_mode == 0)
					option->mode = MODE_WAVE;
				break;

			case 'r':
				option->mode = MODE_RAW;
				user_mode = 1;
				break;

			case 'v':
				option->verbose += 1;
				break;

			case 'w':
				option->mode = MODE_WAVE;
				user_mode = 1;
				break;

			default:
				show_usage (argv);
				exit (ERROR_UNKNOWN_ARGUMENT);
		}
	}

	return (ERROR_OK);
}

void
show_usage (char **argv)
{
	fprintf (stderr, "Usage:\t%s [options]\n", argv[0]);
	fprintf (stderr, "Available options:\n"
			" -c\t\tCode (Default)\n"
			" -d\t\tDecode\n"
			" -h\t\tDisplay this help\n"
			" -i filename\tSet input filename (default: stdin)\n"
			" -o filename\tSet output filename (default: stdout)\n"
			" -r\t\tUse RAW file mode"
			" (Default, if infile/outfile is not set)\n"
			" -v\t\tBe verbose\n"
			" -w\t\tUse WAVE file mode"
			" (Default, if infile/outfile is not set)\n");

	return;
}

void
show_banner (void)
{
	fprintf (stderr, "CELP coder v2.0\n");
	return;
}

/*
 * Function: open_files
 * 
 * Description:
 * 	This function will open the I/O files, and will handle the cases
 * 	where this files are stdin and/or stdout
 */
unsigned int
open_files (struct celp_options *option)
{
	if (option->ifilename == NULL)
	{
		option->ifd = stdin;
	} else
	{
		option->ifd = fopen (option->ifilename, "r");
		if (!option->ifd)
		{
			return (ERROR_FOPEN);
		}
	}

	if (option->ofilename == NULL)
	{
		option->ofd = stdout;
	} else
	{
		option->ofd = fopen (option->ofilename, "w");
		if (!option->ofd)
		{
			return (ERROR_FOPEN);
		}
	}

	return (ERROR_OK);
}

unsigned int
close_files (struct celp_options *option)
{
	fclose (option->ofd);
	fclose (option->ifd);

	return (ERROR_OK);
}
