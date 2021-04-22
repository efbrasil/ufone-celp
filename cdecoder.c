#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "celp.h"
#include "celp_private.h"
#include "error.h"
#include "filter.h"
#include "lpc.h"
#include "pack.h"

CELP
*cdecoder_init (int win_length, int num_coef, int fc_length, int ac_length,
		double gamma)
{
	CELP *d;
	unsigned int ret;
	int i;

	/*
	 * win_length must be a multiple of 4, so we can split it to
	 * 4 sub blocks
	 */
	if (((win_length / 4) * 4) != win_length) return (NULL);

	d = (CELP *) calloc (1, sizeof (CELP));
	if (d == NULL) return (NULL);

	d->win_length = win_length;
	d->sub_block_length = d->win_length / 4;
	d->num_coef = num_coef;
	d->first_window = 1;
	d->fc_length = fc_length;
	d->ac_length = ac_length;

	ret = celp_init_fc (d);
	if (ret != ERROR_OK) return (cdecoder_destroy (d));

	ret = celp_init_ac (d);
	if (ret != ERROR_OK) return (cdecoder_destroy (d));

	d->data = (short *) calloc (d->win_length,
			sizeof (short));
	d->sample = (double *) calloc (d->win_length, sizeof (double));
	d->lpc_coef = (double *) calloc (d->num_coef, sizeof (double));
	d->lsf_coef = (double *) calloc (d->num_coef, sizeof (double));
	d->interp_lpc_coef = (double *) calloc (d->num_coef, sizeof (double));
	d->interp_lsf_coef = (double *) calloc (d->num_coef, sizeof (double));
	d->old_lsf_coef = (double *) calloc (d->num_coef, sizeof (double));

	d->excitation = (double *) calloc (d->win_length / 4, sizeof (double));
	d->response = (double *) calloc (d->win_length / 4, sizeof (double));
	d->sfilter_den = (double *) calloc (d->num_coef + 1, sizeof (double));
	d->sfilter_state = (double *) calloc (d->num_coef, sizeof (double));
	for (i = 0; i < d->num_coef; i++)
		d->sfilter_state [i] = 0.0;

	/* DEBUG */
	d->debug = 0;
	d->counter = 0;

	return (d);
}

CELP *
cdecoder_destroy (CELP *d)
{
	/*
	 * Must free() all the calloc'd stuff
	 */
	free (d);
	return (NULL);
}

unsigned int
cdecoder_set_bitstream (short *sample, CELP_WIN *w, CELP *d)
{
	/*
	 * Recover the window from the bitstream
	 */
	pckd2win (w, w->bitstream);
	celp_dequant_lsf (w);
	celp_dequant_gains (w);

	return (cdecoder_set_window (sample, w, d));
}

unsigned int
cdecoder_set_window (short *sample, CELP_WIN *w,
		CELP *d)
{
	int i;

	
	/* Save the old LSF coefficients, for interpolation */
	if (!d->first_window)
		for (i = 0; i < d->num_coef; i++)
			d->old_lsf_coef [i] = d->lsf_coef [i];

	/* Load the information from the window */
	for (i = 0; i < 4; i++)
	{
		d->ac_index [i] = w->ac_index [i];
		d->ac_gain [i] = w->ac_gain [i];
		d->fc_index [i] = w->fc_index [i];
		d->fc_gain [i] = w->fc_gain [i];
	}

	for (i = 0; i < d->num_coef; i++)
		d->lsf_coef [i] = w->lsf_coef [i];

	/*
	 * The sub blocks will be handled in a different function,
	 * cdecoder_get_sub_block ()
	 */
	for (d->sub_block = 0; d->sub_block < 4; d->sub_block++)
	{
		cdecoder_get_sub_block (d);
		for (i = 0; i < d->win_length / 4; i++)
			sample [i + (d->win_length / 4 * d->sub_block)] =
				(int) d->response [i];
	}

	/*
	 * Make sure that interpolation will happen next time
	 * cdecoder_set_window() is called
	 */
	d->first_window = 0;

	d->counter += 1;

	return (ERROR_OK);
}

unsigned int
cdecoder_get_sub_block (CELP *d)
{
	int i, ret;

	ret = celp_interp_lsf (d);
	if (ret != ERROR_OK) return (ret);

	/*
	 * Reading the candidate sequence from the AC which has already gone
	 * into the synthesis filter
	 */
	celp_get_ac_sequence (d->ac_index [d->sub_block], d->ac_sample,
			d->current_ac_sample, d);

	/*
	 * Reading the candidate sequence from the FC which has already gone
	 * into the synthesis filter
	 */
	celp_get_fc_sequence (d->fc_index [d->sub_block], d->fc_sample,
			d->current_fc_sample, d);

	for (i = 0; i < d->win_length / 4; i++)
	{
		d->excitation [i] =
			(d->ac_gain [d->sub_block] * d->current_ac_sample [i]) +
			(d->fc_gain [d->sub_block] * d->current_fc_sample [i]);
	}

	/* Calculating the denominator of the Synthesis Filter */
	d->sfilter_den [0] = 1.0;

	for (i = 1; i <= d->num_coef; i++)
		d->sfilter_den [i] = - d->interp_lpc_coef [i - 1];

	/*
	 * Applying the decoded excitation into the Synthesis
	 * to calculate the decoded voice sub block
	 */
	filt_sp1 (d->excitation, d->response, d->sfilter_den, d->sfilter_state,
			d->win_length / 4, d->num_coef);

	ret = celp_ac_update (d);
	if (ret != ERROR_OK) return (ret);


	return (ERROR_OK);
}
