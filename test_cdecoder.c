#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "celp.h"

#define LENGTH 160
#define NUM_COEF 10

int
main (int argc, char **argv)
{
	unsigned short sample [LENGTH];
	/* unsigned short data [LENGTH]; */
	FILE *ifd, *ofd;
	int j, ret;
	CELP *d;
	CELP_WIN w;
	if (argc != 3)
	{
		fprintf (stderr, "Usage: %s infilename outfilename\n", argv [0]);
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
	

	d = cdecoder_init (LENGTH, NUM_COEF, 4096, 4096, 0.8);
	if (d == NULL)
	{
		printf ("A cdecoder_init () retornou NULL.\n");
		fclose (ifd);
		fclose (ofd);
		exit (ERROR_OK);
	}

	for (j = 0; ; j++)
	{
		ret = fread (&w, sizeof (w), 1, ifd);
		if (ret != 1)
		{
			fclose (ofd);
			cdecoder_destroy (d);

			if ((ret == 0) && (feof (ifd)))
			{
				printf ("End Of File\n");
				fclose (ifd);
				exit (ERROR_OK);
			}
			
			fclose (ofd);
			printf ("Error reading data from file (ret = %d)\n",
					ret);
			exit (ERROR_FREAD);
		}

		ret = cdecoder_set_window (sample, &w, d);
		if (ret != ERROR_OK)
		{
			printf ("A cdecoder_set_samples retornou %d.\n", ret);
			fclose (ifd);
			fclose (ofd);
			cdecoder_destroy (d);
			exit (ret);
		}

		ret = fwrite (sample, sizeof (sample [0]), LENGTH, ofd);
		if (ret != LENGTH)
		{
			printf ("Error writing to outfile. (ret = %d)\n", ret);
			fclose (ifd);
			fclose (ofd);
			cdecoder_destroy (d);
			exit (ERROR_FWRITE);
		}
	}

	cdecoder_destroy (d);
	fclose (ifd);
	fclose (ofd);
	return (ERROR_OK);
}
