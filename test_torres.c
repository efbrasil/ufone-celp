#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "celp.h"

#define LENGTH 160
#define NUM_COEF 10
#define FC_SIZE 4096
#define AC_SIZE 4096
#define GAMMA 0.8

char *files [] = {"again.dat", "aind.dat", "fgain.dat", "find.dat",
	"lsf.dat", "lsf_index.dat", "again_ind.dat", "fgain_ind.dat", NULL};

int
open_files (FILE **fd)
{
	int j;

	for (j = 0; files [j] != NULL; j++)
		fd [j] = fopen (files [j], "w");

	return (0);
}

int
close_files (FILE **fd)
{
	int j;

	for (j = 0; files [j] != NULL; j++)
		fclose (fd [j]);

	return (0);
}

int
write_window (CELP_WIN *w, FILE **fd)
{
	int ret;

	ret = fwrite ( &(w->ac_gain), sizeof (w->ac_gain), 1, fd [0]);
	if (ret != 1) return (1);

	ret = fwrite ( &(w->ac_index), sizeof (w->ac_index), 1, fd [1]);
	if (ret != 1) return (2);
	
	ret = fwrite ( &(w->fc_gain), sizeof (w->fc_gain), 1, fd [2]);
	if (ret != 1) return (3);
	
	ret = fwrite ( &(w->fc_index), sizeof (w->fc_index), 1, fd [3]);
	if (ret != 1) return (4);
	
	ret = fwrite ( &(w->lsf_coef), sizeof (w->lsf_coef), 1, fd [4]);
	if (ret != 1) return (5);

	ret = fwrite ( &(w->lsf_coef_index), sizeof (w->lsf_coef_index),
			1, fd [5]);
	if (ret != 1) return (6);

	ret = fwrite ( &(w->ac_gain_quant_ind), sizeof (w->ac_gain_quant_ind),
			1, fd [6]);
	if (ret != 1) return (7);

	ret = fwrite ( &(w->fc_gain_quant_ind), sizeof (w->fc_gain_quant_ind),
			1, fd [7]);
	if (ret != 1) return (8);

	return (0);
}

int
main (int argc, char **argv)
{
	short data [LENGTH];
	FILE *ifd;
	FILE *fd [8];
	int j, ret;
	CELP *c;
	CELP_WIN w;

	if (argc != 2)
	{
		fprintf (stderr, "Usage: %s infilename\n", argv [0]);
		exit (ERROR_USAGE);
	}

	ifd = fopen (argv [1], "r");
	if (ifd == NULL)
	{
		fprintf (stderr, "Error opening file.\n");
		exit (ERROR_FOPEN);
	}

	ret = open_files (fd);

	c = ccoder_init (LENGTH, NUM_COEF, FC_SIZE, AC_SIZE, GAMMA);
	if (c == NULL)
	{
		printf ("A ccoder_init () retornou NULL.\n");
		fclose (ifd);
		close_files (fd);
		exit (ERROR_OK);
	}

	for (j = 0; ; j++)
	{
		ret = fread (data, sizeof (data [0]), LENGTH, ifd);
		if (ret != LENGTH)
		{

			if ((ret == 0) && (feof (ifd)))
			{
				fclose (ifd);
				close_files (fd);
				ccoder_destroy (c);
				printf ("End of File\n");
				exit (ERROR_OK);
			}
			
			fclose (ifd);
			close_files (fd);
			ccoder_destroy (c);
			printf ("Error reading data from file (ret = %d)\n",
					ret);
			exit (ERROR_FREAD);
		}
		printf ("Janela %d\n", j);

		ret = ccoder_set_samples (data, &w, c);
		if (ret != ERROR_OK)
		{
			printf ("A ccoder_set_samples retornou %d.\n", ret);
			fclose (ifd);
			close_files (fd);
			ccoder_destroy (c);
			exit (ret);
		}

		ret = write_window (&w, fd);
	}

	ccoder_destroy (c);
	fclose (ifd);
	close_files (fd);

	return (ERROR_OK);
}
