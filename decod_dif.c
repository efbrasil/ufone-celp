#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "cdecoder.h"

/* These #defines shouldn't be changed */
#define LENGTH 160
#define NUM_COEF 10
#define FC_SIZE 4096
#define AC_SIZE 4096
#define GAMMA 0.8

struct celp_window
{
	int ac_index [4];
	double ac_gain [4];
	int fc_index [4];
	double fc_gain [4];
	double lsf_coef [10];
};

/*
 * Torres,
 * o seu programa tem que fazer o seguinte:
 *
 */

/*
 * Voce nao precisa disso
 */
char *files [] = {"again.dat", "aind.dat", "fgain.dat", "find.dat",
	              "lsf.dat", NULL};

#define NUM_DIFS 2
struct dif
{
	char *name;	/* ex.: "0103" */
	int pad;	/* how many zeros should be removed from the end */
};

struct dif difs [NUM_DIFS] =
{
	{"0103", 10},
	{"0203", 20},
};

int
read_window (struct cdecoder_win *w, FILE **fd)
{
	int ret;

	ret = fread ( &(w->ac_gain), sizeof (w->ac_gain), 1, fd [0]);
	if (ret != 1) return (1);

	ret = fread ( &(w->ac_index), sizeof (w->ac_index), 1, fd [1]);
	if (ret != 1) return (2);
	
	ret = fread ( &(w->fc_gain), sizeof (w->fc_gain), 1, fd [2]);
	if (ret != 1) return (3);
	
	ret = fread ( &(w->fc_index), sizeof (w->fc_index), 1, fd [3]);
	if (ret != 1) return (4);
	
	ret = fread ( &(w->lsf_coef), sizeof (w->lsf_coef), 1, fd [4]);
	if (ret != 1) return (5);

	return (ERROR_OK);
}

int
open_files (FILE **fd)
{
	int j;

	for (j = 0; files [j] != NULL; j++)
	{
		fd [j] = fopen (files [j], "r");
		if (fd [j] == NULL)
		{
			printf ("Erro no arquivo %d\n", j);
		}
	}


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
main (int argc, char **argv)
{
	unsigned short sample [LENGTH];
	FILE *ofd;
	int ret, i, pad;
	struct cdecoder *d;
	struct cdecoder_win w;
	FILE *fd [5];

	if ((argc != 2) && (argc != 3))
	{
		fprintf (stderr, "Usage: %s outfilename [pad]\n", argv [0]);
		exit (ERROR_USAGE);
	}
	
	if (argc == 3)
	{
		pad = atoi (argv [2]);
	} else
	{
		pad = 0;
	}

	ofd = fopen (argv [1], "w");
	if (ofd == NULL)
	{
		fprintf (stderr, "Error opening file.\n");
		exit (ERROR_FOPEN);
	}
	

	d = cdecoder_init (LENGTH, NUM_COEF, FC_SIZE, AC_SIZE, GAMMA);
	if (d == NULL)
	{
		printf ("A cdecoder_init () retornou NULL.\n");
		close_files (fd);
		fclose (ofd);
		exit (ERROR_OK);
	}

	ret = open_files (fd);

	i = 0;

	while ((ret = read_window (&w, fd)) == ERROR_OK)
	{
		if (i != 0)
		{
			/*
			 * Write the previous window
			 */
			ret = fwrite (sample, sizeof (sample [0]), LENGTH, ofd);
			if (ret != LENGTH)
			{
				printf ("Error writing file.\n");
				fclose (ofd);
				close_files (fd);
				cdecoder_destroy (d);

				exit (ERROR_FWRITE);
			}
		}

		ret = cdecoder_set_window (sample, &w, d);
		if (ret != ERROR_OK)
		{
			printf ("A cdecoder_set_samples retornou %d.\n", ret);
			close_files (fd);
			fclose (ofd);
			cdecoder_destroy (d);
			exit (ret);
		}

		i++;
	}

	/*
	 * Write the last window
	 */
	ret = fwrite (sample, sizeof (sample [0]), LENGTH - pad, ofd);
	if (ret != (LENGTH - pad))
	{
		printf ("Error writing file.\n");
		fclose (ofd);
		close_files (fd);
		cdecoder_destroy (d);
		
		exit (ERROR_FWRITE);
	}

	printf ("Eu li %d janelas.\n", i);

	close_files (fd);
	fclose (ofd);
	cdecoder_destroy (d);

	return (ERROR_OK);
}

/*
 * A partir daqui voce precisa
 */
