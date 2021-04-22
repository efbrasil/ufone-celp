#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "celp.h"

#define LENGTH 160
#define NUM_COEF 10

int
main (int argc, char **argv)
{
	unsigned short data [LENGTH];
	FILE *ifd, *ofd;
	int j, ret;
	CELP *c;
	CELP_WIN w;

	if (argc != 3)
	{
		fprintf (stderr, "Usage: %s infilename outfilename\n",
				argv [0]);
		exit (ERROR_USAGE);
	}

	ifd = fopen (argv [1], "r");
	if (ifd == NULL)
	{
		fprintf (stderr, "Error opening file.\n");
		exit (ERROR_FOPEN);
	}

	ofd = fopen (argv [2], "w");
	if (ofd == NULL)
	{
		fprintf (stderr, "Error opening file.\n");
		fclose (ifd);
		exit (ERROR_FOPEN);
	}
	

	c = ccoder_init (LENGTH, NUM_COEF, 4096, 4096, 0.8);
	if (c == NULL)
	{
		printf ("A ccoder_init () retornou NULL.\n");
		fclose (ifd);
		fclose (ofd);
		exit (ERROR_OK);
	}

	for (j = 0; ; j++)
	{
		ret = fread (data, sizeof (data [0]), LENGTH, ifd);
		if (ret != LENGTH)
		{
			fclose (ifd);
			fclose (ofd);
			ccoder_destroy (c);
			if ((ret == 0) && (feof (ifd)))
			{
				printf ("End Of File\n");
				exit (ERROR_OK);
			}
			
			printf ("Error reading data from file (ret = %d)\n",
					ret);
			exit (ERROR_FREAD);
		}

		ret = ccoder_set_samples (data, &w, c);
		if (ret != ERROR_OK)
		{
			printf ("A ccoder_set_samples retornou %d.\n", ret);
			fclose (ifd);
			fclose (ofd);
			ccoder_destroy (c);
			exit (ret);
		}

		ret = fwrite (&w, sizeof (w), 1, ofd);
		if (ret != 1)
		{
			printf ("Error writing to outfile.\n");
			fclose (ifd);
			fclose (ofd);
			ccoder_destroy (c);
			exit (ERROR_FWRITE);
		}
	}

	ccoder_destroy (c);
	fclose (ifd);
	fclose (ofd);
	return (ERROR_OK);
}
