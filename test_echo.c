#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "celp.h"

#define LENGTH 160
#define NUM_COEF 10

int
main (int argc, char **argv)
{
	short data [LENGTH], sample [LENGTH];
	FILE *ifd, *ofd;
	int i, j, ret;
	CELP *c, *d;
	CELP_WIN w, wdec;

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
		ret = fread (data, sizeof (data [0]), LENGTH, ifd);
		if (ret != LENGTH)
		{
			if ((ret == 0) && (feof (ifd)))
			{
				printf ("End Of File\n");
				fclose (ifd);
				fclose (ofd);
				ccoder_destroy (c);
				cdecoder_destroy (d);

				exit (ERROR_OK);
			}
			
			fclose (ifd);
			fclose (ofd);
			ccoder_destroy (c);
			cdecoder_destroy (d);
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

		for (i = 0; i < 4; i++)
		{
			wdec.ac_index [i] = w.ac_index [i];
			wdec.ac_gain [i] = w.ac_gain [i];
			wdec.fc_index [i] = w.fc_index [i];
			wdec.fc_gain [i] = w.fc_gain [i];
		}

		for (i = 0; i < NUM_COEF; i++)
			wdec.lsf_coef [i] = w.lsf_coef [i];

		ret = cdecoder_set_window (sample, &wdec, d);
		if (ret != ERROR_OK)
		{
			printf ("A cdecoder_set_samples retornou %d.\n", ret);
			fclose (ifd);
			fclose (ofd);
			ccoder_destroy (c);
			exit (ret);
		}

		ret = fwrite (sample, sizeof (sample [0]), LENGTH, ofd);
		if (ret != LENGTH)
		{
			printf ("Error writing to outfile.\n");
			fclose (ifd);
			fclose (ofd);
			ccoder_destroy (c);
			exit (ERROR_FWRITE);
		}
	}

	ccoder_destroy (c);
	cdecoder_destroy (d);
	fclose (ifd);
	fclose (ofd);
	return (ERROR_OK);
}
