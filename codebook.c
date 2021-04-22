#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "celp.h"
#include "celp_private.h"
#include "error.h"
#include "filter.h"
#include "lpc.h"
#include "quant3.h"

/*
 * Function: celp_init_ac
 *
 * Description:
 * 	Evaluate the real size of the AC, malloc() space for it, and 
 * 	set all the samples to zero.
 *
 * Return value:
 * 	ERROR_OK on sucess, other error code on failure.
 */
unsigned int
celp_init_ac (CELP *c)
{
	int i;
	
	c->ac_number_of_samples = c->ac_length + (c->win_length / 4) - 1;
	c->ac_sample = (double *) calloc (c->ac_number_of_samples,
			sizeof (double));
	c->saved_ac_sample = (double *) calloc (c->ac_number_of_samples,
			sizeof (double));
	c->perc_ac_sample = (double *) calloc (c->ac_number_of_samples,
			sizeof (double));
	c->current_ac_sample = (double *) calloc (c->win_length /4,
			sizeof (double));
	c->current_perc_ac_sample = (double *) calloc (c->win_length /4,
			sizeof (double));
	c->ac_response = (double *) calloc (c->win_length / 4,
			sizeof (double));

	for (i = 0; i < c->ac_number_of_samples; i++)
		c->ac_sample [i] = 0.0;

	return (ERROR_OK);
}

/*
 * Function: celp_apply_sfilter_to_ac
 *
 * Description:
 * 	Filter ALL the AC with the Perceptual Filter.
 *
 * Return value:
 * 	ERROR_OK on sucess, other error code on failure.
 */
unsigned int
celp_apply_sfilter_to_ac (CELP *c)
{
	int i;
	
	/* Saving the filter input */
	for (i = 0; i < c->ac_number_of_samples; i++)
		c->saved_ac_sample [i] = c->ac_sample [i];
	
	filt_sp2 (c->ac_sample, c->perc_ac_sample, c->pfilter_den,
			c->ac_number_of_samples, c->num_coef);

	/* Loading the filter input */
	for (i = 0; i < c->ac_number_of_samples; i++)
		c->ac_sample [i] = c->saved_ac_sample [i];

	return (ERROR_OK);
}

/*
 * Function: celp_ac_update
 *
 * Description:
 * 	Add the current excitation to the end of the AC, shifting all the
 * 	samples to the beggining of the vector, and removing the first (oldest)
 * 	40 samples.
 *
 * Return value:
 * 	ERROR_OK on sucess, other error code on failure.
 */
unsigned int celp_ac_update (CELP *c)
{
	int i;

	for (i = 0; i < c->ac_number_of_samples; i++)
	{
		if (i < (c->ac_number_of_samples - (c->win_length / 4)))
		{
			c->ac_sample [i] =
				c->ac_sample [i + c->win_length / 4];
		} else
		{
			c->ac_sample [i] =
				c->excitation
				[i - (c->ac_number_of_samples
						- (c->win_length / 4))];
		}
	}

	return (ERROR_OK);
}

/*
 * Function: celp_init_fc
 *
 * Description:
 * 	Evaluate the real size of the FC, malloc() space for it, and 
 * 	load the samples from the file codebook4096.dat.
 *
 * Return value:
 * 	ERROR_OK on sucess, other error code on failure.
 */
unsigned int
celp_init_fc (CELP *c)
{
	int ret;

	/*
	c->fc_number_of_samples = 2 * c->fc_length + (c->win_length / 4) - 2;
	*/
	c->fc_number_of_samples = c->fc_length + (c->win_length / 4) - 1;
	c->fc_sample = (double *) calloc (c->fc_number_of_samples,
			sizeof (double));
	c->perc_fc_sample = (double *) calloc (c->fc_number_of_samples,
			sizeof (double));
	c->current_fc_sample = (double *) calloc (c->win_length / 4,
			sizeof (double));
	c->current_perc_fc_sample = (double *) calloc (c->win_length / 4,
			sizeof (double));
	c->fc_response = (double *) calloc (c->win_length / 4,
			sizeof (double));

	ret = load_from_file ("codebook4096.dat", c->fc_sample,
			c->fc_number_of_samples);
	if (ret != ERROR_OK) return (ret);

	return (ERROR_OK);
}

unsigned int
celp_apply_sfilter_to_fc (CELP *c)
{
	int i;
	
	/* Saving the filter input */
	for (i = 0; i < c->ac_number_of_samples; i++)
		c->saved_ac_sample [i] = c->ac_sample [i];

	filt_sp2 (c->fc_sample, c->perc_fc_sample, c->pfilter_den,
			c->fc_number_of_samples, c->num_coef);

	/* Loading the filter input */
	for (i = 0; i < c->ac_number_of_samples; i++)
		c->ac_sample [i] = c->saved_ac_sample [i];

	return (ERROR_OK);
}

unsigned int
celp_set_complete_response (CELP *c)
{
	int i;

	/*
	 * Reading the candidate sequence from the AC which has already gone
	 * into the synthesis filter
	 */
	celp_get_ac_sequence (c->ac_index [c->sub_block], c->ac_sample,
			c->current_ac_sample, c);

	/*
	 * Reading the candidate sequence from the FC which has already gone
	 * into the synthesis filter
	 */
	celp_get_fc_sequence (c->fc_index [c->sub_block], c->fc_sample,
			c->current_fc_sample, c);

	/* Applying the gain */
	for (i = 0; i < c->win_length / 4; i++)
	{
		c->current_ac_sample [i] *= c->ac_gain [c->sub_block];
		c->current_fc_sample [i] *= c->fc_gain [c->sub_block];
		c->excitation [i] = c->current_ac_sample [i]
			+ c->current_fc_sample [i];
	}

	for (i = 0; i < c->num_coef; i++)
		c->sfilter_state [i] = 0.0;

	/* Applying Synthesis Filter */
	filt_sp1 (c->excitation, c->response, c->pfilter_den,
			c->sfilter_state, c->win_length / 4, c->num_coef);

	filt_sp1 (c->zinput, c->zinput_response, c->pfilter_den,
			c->sfilter_state, c->win_length / 4, c->num_coef);

	return (ERROR_OK);
}

unsigned int
celp_get_ac_sequence (int n, double *src, double *dest, CELP *c)
{
	int i;

	for (i = 0; i < c->win_length / 4; i++)
		dest [i] = src [n + i];

	return (ERROR_OK);
}
unsigned int
celp_get_fc_sequence (int n, double *src, double *dest, CELP *c)
{
	int i;

	/*
	for (i = 0; i < c->win_length / 4; i++)
		dest [i] = src [2 * n + i];
	*/
	for (i = 0; i < c->win_length / 4; i++)
		dest [i] = src [n + i];

	return (ERROR_OK);
}

unsigned int
celp_quant_ac_gain (CELP *c)
{
	int index;

	c->ac_gain_quant [c->sub_block] =
		quant_esc (c->ac_gain [c->sub_block], ac_part,
				ac_cb, 64, &index);

	c->ac_gain_quant_ind [c->sub_block] = index;
	c->ac_gain [c->sub_block] = c->ac_gain_quant [c->sub_block];

	return (ERROR_OK);
}
unsigned int
celp_quant_fc_gain (CELP *c)
{
	int index;

	c->fc_gain_quant [c->sub_block] =
		quant_esc (c->fc_gain [c->sub_block], fc_part,
				fc_cb, 64, &index);

	c->fc_gain_quant_ind [c->sub_block] = index;
	c->fc_gain [c->sub_block] = c->fc_gain_quant [c->sub_block];

	return (ERROR_OK);
}

unsigned int
celp_dequant_gains (CELP_WIN *w)
{
	w->ac_gain [0] = ac_cb [w->ac_gain_quant_ind [0]];
	w->ac_gain [1] = ac_cb [w->ac_gain_quant_ind [1]];
	w->ac_gain [2] = ac_cb [w->ac_gain_quant_ind [2]];
	w->ac_gain [3] = ac_cb [w->ac_gain_quant_ind [3]];

	w->fc_gain [0] = fc_cb [w->fc_gain_quant_ind [0]];
	w->fc_gain [1] = fc_cb [w->fc_gain_quant_ind [1]];
	w->fc_gain [2] = fc_cb [w->fc_gain_quant_ind [2]];
	w->fc_gain [3] = fc_cb [w->fc_gain_quant_ind [3]];

	return (ERROR_OK);
}
