#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "celp.h"
#include "celp_private.h"
#include "quant2.h"
#include "error.h"
#include "filter.h"

unsigned int
lpc_analysis (double *sample, int length, double *coef, int num)
{
	int i, j;
	double aux, inverse_EMPQ, *corr, *ao, *k;
	float win_binomial[11] = {1.0, 0.999375390381012, 0.997503900885916,
		                  0.994392535006671, 0.990052916711755,
				  0.984501218113371, 0.977758059085197,
				  0.969848379702430, 0.960801286608190,
				  0.950649874630354, 0.939431025178194};

	if (((corr = (double *) calloc (num + 1, sizeof (double))) == NULL) ||
	    ((k = (double *) calloc (num, sizeof (double))) == NULL)        ||
	    ((ao = (double *) calloc (num, sizeof (double))) == NULL))
	{
		return (ERROR_MALLOC);
	}

	/* Evaluate the auto-correlation of this window */
	for (j = 0; j <= num; j++)
	{
		corr [j] = 0.0;
		for (i = j; i < length + 1; i++)
			corr [j] += (double) (sample [i] * sample [i - j]);

		corr[j] *= win_binomial[j];
	}

	/*
	 * Calculating the LPCs using the recursive algorithm
	 * of Levinson-Durbin
	 */
	ao [0] = k [0] = (corr [1] / corr [0]);
	inverse_EMPQ = corr [0] * (1 - k [0] * k [0]);

	for (i = 1; i < num; i++)
	{
		aux = 0.0;
		for (j = 0; j <= i - 1; j++)
			aux += ao[j] * corr[i - j];
		
		k [i] = (corr [i + 1] - aux) / inverse_EMPQ;
		coef [i] = k [i];
		
		for (j = 0; j <= i - 1; j++)
			coef [j] = ao [j] - k [i] * ao [i - j - 1];
		
		inverse_EMPQ *= (1 - k [i] * k [i]);
		for (j = 0; j <= i; j++)
			ao [j] = coef [j];
	}

	free (corr);
	free (k);
	free (ao);

	return (ERROR_OK);
}

unsigned int
lpc2lsf (double *lpc, double *lsf, int num)
{
	double g1 [100], g2 [100];
	double g1r [100], g2r [100];
	int even;
	int g1_order, g2_order;
	int orderd2;
	int i, j;
	
	/*int swap;*/
	double factor;

	/* Compute the lengths of the x polynomials. */

	even = (num & 1) == 0;  /* True if order is even. */
	if (even) 
		g1_order = g2_order = num / 2;
	else
	{
		fprintf (stderr, "Odd order not implemented yet\n");
		exit (1);
		g1_order = (num + 1) / 2;
		g2_order = g1_order - 1;
	}
	
	/* Compute the first half of K & R F1 & F2 polynomials. */
	
	/* Compute half of the symmetric and antisymmetric polynomials. */
	/* Remove the roots at +1 and -1. */
	
	orderd2 = (num + 1) / 2;
	g1 [orderd2] = 1;
	for (i = 1; i <= orderd2; i++)
		g1 [g1_order - i] = -(lpc [num-i] + lpc [i-1]);
	
	g2 [orderd2] = 1;
	for(i = 1; i <= orderd2; i++)
		g2 [orderd2 - i] = lpc [num - i] - lpc [i - 1];
	
	if (even)
	{
		for(i = 1; i <= orderd2; i++)
			g1 [orderd2 - i] -= g1 [orderd2 - i + 1];
		
		for(i = 1; i <= orderd2; i++)
			g2 [orderd2 - i] += g2 [orderd2 - i + 1];
	} else
		for (i = 2; i <= orderd2; i++) 
			g2 [orderd2 - i] += g2 [orderd2 - i + 2];   /* Right? */

	/* Convert into polynomials in cos(alpha) */
	cheby1 (g1, g1_order);
	cheby1 (g2, g2_order);
	factor = 0.5;

	/* Find the roots of the 2 even polynomials.*/
	cacm283 (g1, g1_order, g1r);
	cacm283 (g2, g2_order, g2r);
	
	/* Convert back to angular frequencies in the range 0 to pi. */
	for(i = 0, j = 0; ; )
	{
		lsf [j++] = acos (factor * g1r [i]);
		if (j >= num) break;
		
		lsf [j++] = acos (factor * g2r [i]);
		if (j >= num) break;
		i++;
	}

	return (ERROR_OK);
}

unsigned int
lsf2lpc (double *lsf, double *lpc, int num) 
{
	double pa1[3], pa2[3], pa3[3], pa4[3], pa5[3], pa6[5], pa7[5], pa8[9];
	double qa1[3], qa2[3], qa3[3], qa4[3], qa5[3], qa6[5], qa7[5], qa8[9];
	double p[11], q[11];
	int i;


	/* Calculating the Q(z) and P(z) polynomials */
	pa1[0] = pa1[2] = pa2[0] = pa2[2] = pa3[0] = pa3[2] = pa4[0] =
		pa4[2] = pa5[0] = pa5[2] = 1.0;

	qa1[0] = qa1[2] = qa2[0] = qa2[2] = qa3[0] = qa3[2] = qa4[0] =
		qa4[2] = qa5[0] = qa5[2] = 1.0;

	pa1[1] = -2 * cos (lsf [0]);
	pa2[1] = -2 * cos (lsf [2]);
	pa3[1] = -2 * cos (lsf [4]);
	pa4[1] = -2 * cos (lsf [6]);
	pa5[1] = -2 * cos (lsf [8]);
	qa1[1] = -2 * cos (lsf [1]);
	qa2[1] = -2 * cos (lsf [3]);
	qa3[1] = -2 * cos (lsf [5]);
	qa4[1] = -2 * cos (lsf [7]);
	qa5[1] = -2 * cos (lsf [9]);

	polimulti (pa1, pa2, pa6, 2, 2);
	polimulti (pa3, pa4, pa7, 2, 2);
	polimulti (pa6, pa7, pa8, 4, 4);
	polimulti (pa8, pa5, p, 8, 2);
	polimulti (qa1, qa2, qa6, 2, 2);
	polimulti (qa3, qa4, qa7, 2, 2);
	polimulti (qa6, qa7, qa8, 4, 4);
	polimulti (qa8, qa5, q, 8, 2);
	
	/* Calculating the LPCs */
	for (i = 0; i < num; i++)
		lpc [i] = -(p [i + 1] + p [i] + q [i + 1] - q [i]) / 2;

	return (ERROR_OK);
}

unsigned int
celp_interp_lsf (CELP *c)
{
	int i;
	double q[4];
	
	q[0] = 0.25;
	q[1] = 0.5;
	q[2] = 0.75;
	q[3] = 1.0;

	if (c->first_window)
	{
		/* No interpolation for the first window */
		for (i = 0; i < c->num_coef; i++)
		{
			c->interp_lsf_coef [i] = c->lsf_coef [i];
		}
	} else
	{
		for (i = 0; i < c->num_coef; i++)
		{
			c->interp_lsf_coef [i] = 
				(1 - q [c->sub_block]) * c->old_lsf_coef [i]
				+ q [c->sub_block]  * c->lsf_coef [i];
		}
	}

	/* Evaluate interp_lpc_coef from interp_lsf_coef */
	return (lsf2lpc (c->interp_lsf_coef, c->interp_lpc_coef, c->num_coef));
}

unsigned int
celp_quant_lsf (CELP *c)
{
	int index;
	
	c->dlsf_coef [0] = quant_esc (c->lsf_coef [0], part0, cb0, 16, &index);
	c->quant_lsf_coef [0] = c->dlsf_coef [0];
	c->quant_lsf_index [0] = index;


	c->dlsf_coef [1] = quant_esc (c->lsf_coef [1] - c->quant_lsf_coef [0],
			part1, cb1, 16, &index);
	c->quant_lsf_coef [1] = c->quant_lsf_coef [0] + c->dlsf_coef [1];
	c->quant_lsf_index [1] = index;


	c->dlsf_coef [2] = quant_esc (c->lsf_coef [2] - c->quant_lsf_coef [1],
			part2, cb2, 8, &index);
	c->quant_lsf_coef [2] = c->quant_lsf_coef [1] + c->dlsf_coef [2];
	c->quant_lsf_index [2] = index;

	
	c->dlsf_coef [3] = quant_esc (c->lsf_coef [3] - c->quant_lsf_coef [2],
			part3, cb3, 8, &index);
	c->quant_lsf_coef [3] = c->quant_lsf_coef [2] + c->dlsf_coef [3];
	c->quant_lsf_index [3] = index;

	
	c->dlsf_coef [4] = quant_esc (c->lsf_coef [4] - c->quant_lsf_coef [3],
			part4, cb4, 8, &index);
	c->quant_lsf_coef [4] = c->quant_lsf_coef [3] + c->dlsf_coef [4];
	c->quant_lsf_index [4] = index;

	
	c->dlsf_coef [5] = quant_esc (c->lsf_coef [5] - c->quant_lsf_coef [4],
			part5, cb5, 8, &index);
	c->quant_lsf_coef [5] = c->quant_lsf_coef [4] + c->dlsf_coef [5];
	c->quant_lsf_index [5] = index;

	
	c->dlsf_coef [6] = quant_esc (c->lsf_coef [6] - c->quant_lsf_coef [5],
			part6, cb6, 8, &index);
	c->quant_lsf_coef [6] = c->quant_lsf_coef [5] + c->dlsf_coef [6];
	c->quant_lsf_index [6] = index;

	
	c->dlsf_coef [7] = quant_esc (c->lsf_coef [7] - c->quant_lsf_coef [6],
			part7, cb7, 8, &index);
	c->quant_lsf_coef [7] = c->quant_lsf_coef [6] + c->dlsf_coef [7];
	c->quant_lsf_index [7] = index;

	
	c->dlsf_coef [8] = quant_esc (c->lsf_coef [8] - c->quant_lsf_coef [7],
			part8, cb8, 8, &index);
	c->quant_lsf_coef [8] = c->quant_lsf_coef [7] + c->dlsf_coef [8];
	c->quant_lsf_index [8] = index;

	
	c->dlsf_coef [9] = quant_esc (c->lsf_coef [9] - c->quant_lsf_coef [8],
			part9, cb9, 8, &index);
	c->quant_lsf_coef [9] = c->quant_lsf_coef [8] + c->dlsf_coef [9];
	c->quant_lsf_index [9] = index;

	
	return (ERROR_OK);
}

unsigned int
celp_dequant_lsf (CELP_WIN *w)
{
	w->lsf_coef [0] = cb0 [w->lsf_coef_index [0]];
	w->lsf_coef [1] = cb1 [w->lsf_coef_index [1]] + w->lsf_coef [0];
	w->lsf_coef [2] = cb2 [w->lsf_coef_index [2]] + w->lsf_coef [1];
	w->lsf_coef [3] = cb3 [w->lsf_coef_index [3]] + w->lsf_coef [2];
	w->lsf_coef [4] = cb4 [w->lsf_coef_index [4]] + w->lsf_coef [3];
	w->lsf_coef [5] = cb5 [w->lsf_coef_index [5]] + w->lsf_coef [4];
	w->lsf_coef [6] = cb6 [w->lsf_coef_index [6]] + w->lsf_coef [5];
	w->lsf_coef [7] = cb7 [w->lsf_coef_index [7]] + w->lsf_coef [6];
	w->lsf_coef [8] = cb8 [w->lsf_coef_index [8]] + w->lsf_coef [7];
	w->lsf_coef [9] = cb9 [w->lsf_coef_index [9]] + w->lsf_coef [8];

	return (ERROR_OK);
}
