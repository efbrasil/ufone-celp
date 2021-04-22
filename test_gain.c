#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "celp.h"

#define LENGTH 160
#define FC_SIZE 4096
#define AC_SIZE 4096
#define GAMMA 0.8
#define NUM_COEF 10

int
main (int argc, char **argv)
{
	short data [LENGTH];
	FILE *ifd;
	int i, ret, j;
	CELP *c = NULL;
	CELP_WIN w;

	if (argc < 2)
	{
		fprintf (stderr, "Usage: %s file1 file2 ...\n",
				argv [0]);
		exit (ERROR_USAGE);
	}

	for (i = 1; i < argc; i++)
	{
		fprintf (stderr, "Processando arquivo %s.\n", argv [i]);

		ifd = fopen (argv [i], "r");
		if (ifd == NULL)
		{
			fprintf (stderr, "Error opening file.\n");
			exit (ERROR_FOPEN);
		}

		c = ccoder_init (LENGTH, NUM_COEF, FC_SIZE, AC_SIZE, GAMMA);
		if (c == NULL)
		{
			fprintf (stderr, "A ccoder_init () retornou NULL.\n");
			fclose (ifd);
			exit (ERROR_OK);
		}

		while ((ret = fread (data, sizeof (data [0]), LENGTH, ifd))
					== LENGTH)
		{
			ret = ccoder_set_samples (data, &w, c);
			if (ret != ERROR_OK)
			{
				fprintf (stderr, "A ccoder_set_samples"
						" retornou %d.\n", ret);
				fclose (ifd);
				ccoder_destroy (c);
				exit (ret);
			}

			for (j = 0; j < 4; j++)
			{
				printf ("%.20f\t %.20f\n", w.ac_gain [j],
						w.fc_gain [j]);
			}
		}

		if (ret != LENGTH)
		{
			if ((ret == 0) && (feof (ifd)))
			{
				fclose (ifd);
				ccoder_destroy (c);
			} else
			{
				fclose (ifd);
				ccoder_destroy (c);
				fprintf (stderr, "Error reading data from file"
						" (ret = %d)\n", ret);
				exit (ERROR_FREAD);
			}
		}
	}
	
	return (ERROR_OK);
}
