#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "ccoder.h"

#define LENGTH 160	/* Size of your window. Must be a multiple of four. */
#define NUM_COEF 10	/* Number of LP coefficients. Use 10, please */
#define FC_SIZE 4096	/* Size of fixed codebook */
#define AC_SIZE 4096	/* Size of adaptive codebook */
#define GAMMA 0.8	/* Gamma, for the perceptual filter */
				   
void
main (int argc, char **argv)
{
	struct ccoder *c;
	struct ccoder_win w;

	unsigned short data [LENGTH];

	while (1)
	{
		c = ccoder_init (LENGTH, NUM_COEF, FC_SIZE, AC_SIZE, GAMMA);
		if (ret != ERROR_OK)
			return (ret);

		/*
		 * Acquire LENGTH samples to the unsigned short vector
		 * 'data'
		 */
		acquire_samples (data, LENGTH);

		ret = ccoder_set_samples (data, &w, c);
		if (ret != ERROR_OK)
			return (ret);

		/*
		 * After calling ccoder_set_samples the parameters for
		 * this windows are int the structure 'w'
		 */
		do_someting (&w);
	}

	ccoder_destroy (c);
	return (ERROR_OK);
}
