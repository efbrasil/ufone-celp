#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "celp.h"
#include "celp_private.h"
#include "error.h"
#include "filter.h"
#include "lpc.h"
#include "pack.h"

/*
 * Function: ccoder_init
 * 
 * Description:
 * 	This is the first function that should be called before coding.
 * 	It will malloc() all the necessary space for the celp coding, and
 * 	will return a pointer to a CELP, that will be used with
 * 	all the other functions.
 *
 * 	It is important to call ccoder_destroy after using the celp coder, so
 * 	it can free() the space that was previously malloc()'ed.
 *
 * Return value:
 * 	ccoder_init will return a pointer to a 'CELP' on sucess,
 * 	and will return NULL on failure.
 * 	Someday it will also set ccoder_error with an error number.
 */
CELP
*ccoder_init (int win_length, int num_coef, int fc_length, int ac_length,
		double gamma)
{
	CELP *c;
	unsigned int ret;
	int i;

	/*
	 * win_length must be a multiple of 4, so we can split it to
	 * 4 sub blocks
	 */
	if (((win_length / 4) * 4) != win_length) return (NULL);

	c = (CELP *) calloc (1, sizeof (CELP));
	if (c == NULL) return (NULL);

	/* DEBUG */
	c->debug = 0;
	c->counter = 0;

	c->win_length = win_length;
	c->sub_block_length = c->win_length / 4;
	c->num_coef = num_coef;
	c->gamma = gamma;
	c->first_window = 1;
	c->fc_length = fc_length;
	c->ac_length = ac_length;

	ret = celp_init_fc (c);
	if (ret != ERROR_OK) return (ccoder_destroy (c));

	ret = celp_init_ac (c);
	if (ret != ERROR_OK) return (ccoder_destroy (c));

	ret = ccoder_init_pfilter (c);
	if (ret != ERROR_OK) return (ccoder_destroy (c));

	c->data = (short *) calloc (c->win_length,
			sizeof (short));
	/* XXX BUG */
	c->window = (double *) calloc (c->win_length + 1, sizeof (double));
	c->sample = (double *) calloc (c->win_length, sizeof (double));
	c->lpc_coef = (double *) calloc (c->num_coef, sizeof (double));
	c->lsf_coef = (double *) calloc (c->num_coef, sizeof (double));
	c->dlsf_coef = (double *) calloc (c->num_coef, sizeof (double));
	c->quant_lsf_coef = (double *) calloc (c->num_coef, sizeof (double));
	c->quant_lsf_index = (int *) calloc (c->num_coef, sizeof (int));
	c->interp_lpc_coef = (double *) calloc (c->num_coef, sizeof (double));
	c->interp_lsf_coef = (double *) calloc (c->num_coef, sizeof (double));
	c->old_lsf_coef = (double *) calloc (c->num_coef, sizeof (double));

	c->current_sample = (double *) calloc (c->win_length / 4,
			sizeof (double));
	c->current_target_sample = (double *) calloc (c->win_length / 4,
			sizeof (double));

	c->excitation = (double *) calloc (c->win_length / 4, sizeof (double));
	c->response = (double *) calloc (c->win_length / 4, sizeof (double));
	c->sfilter_state = (double *) calloc (c->num_coef, sizeof (double));
	c->zinput = (double *) calloc (c->win_length, sizeof (double));
	c->zinput_response = (double *) calloc (c->win_length,
			sizeof (double));
	
	for (i = 0; i < c->win_length; i++)
		c->zinput_response [i] = c->zinput [i] = 0.0;

	for (i = 0; i < c->num_coef; i++)
		c->sfilter_state [i] = 0.0;

	/* XXX BUG */
	ret = hamming (c->window, c->win_length + 1);
	if (ret != ERROR_OK) return (ccoder_destroy (c));

	return (c);
}

/*
 * Function: ccoder_destroy
 *
 * Description:
 * 	This function will destroy a ccoder descriptor, free()ing all the
 * 	space that was malloc()'ed in ccoder_init.
 * 	This functions MUST be called to avoid memory leaks.
 * 	It should also free everything that has been malloc()'ed, which it
 * 	currently doesn't do.
 *
 * Return value:
 * 	ccoder_destroy always returns NULL.
 */
CELP
*ccoder_destroy (CELP *c)
{
	free (c->data);
	free (c->sample);
	free (c->window);
	
	free (c->lpc_coef);
	free (c->lsf_coef);
	free (c->quant_lsf_coef);
	free (c->interp_lpc_coef);
	free (c->interp_lsf_coef);
	free (c->old_lsf_coef);

	free (c->ac_sample);
	free (c->ac_response);
	free (c->perc_ac_sample);

	free (c->pfilter_num);
	free (c->pfilter_den);
	free (c->pfilter_states_num);
	free (c->pfilter_states_den);

	free (c);

	return (NULL);
}

/*
 * Function: ccoder_set_samples
 *
 * Description:
 * 	Generate the celp coded version of a window of c->win_length samples.
 *
 * Return value:
 * 	ERROR_OK on sucess, other error code on failure.
 */
unsigned int
ccoder_set_samples (short *sample, CELP_WIN *w, CELP *c)
{
	int i, ret;

	/* Convert from unsigned short to double, and apply the window */
	for (i = 0; i < c->win_length; i++)
	{
		c->data [i] = sample [i];
		c->sample [i] = sample [i] * c->window [i];
	}

	/* Save the old LSF coefficients, for interpolation */
	if (!c->first_window)
		for (i = 0; i < c->num_coef; i++)
			c->old_lsf_coef [i] = c->lsf_coef [i];

	/* Evaluate the LPC coefficients */
	ret = lpc_analysis (c->sample, c->win_length, c->lpc_coef,
			c->num_coef);
	if (ret != ERROR_OK) return (ret);

	/* Convert the LPC coefficients to LSF */
	ret = lpc2lsf (c->lpc_coef, c->lsf_coef, c->num_coef);
	if (ret != ERROR_OK) return (ret);

	/* LSF Quantization */
	ret = celp_quant_lsf (c);
	if (ret != ERROR_OK) return (ret);
	for (i = 0; i < c->num_coef; i++)
	{
		/* Fill the window */
		w->lsf_coef [i] = c->quant_lsf_coef [i];
		w->lsf_coef_index [i] = c->quant_lsf_index [i];

		/* Update the coeffs */
		c->lsf_coef [i] = c->quant_lsf_coef [i];
	}

	/*
	 * The sub blocks will be handled in a different function,
	 * ccoder_sub_block_analysis ()
	 */
	for (c->sub_block = 0; c->sub_block < 4; c->sub_block++)
	{
		ccoder_sub_block_analysis (c);
	}

	/*
	 * Make sure that interpolation will happen next time
	 * ccoder_set_samples() is called
	 */
	c->first_window = 0;

	/*
	 * Fill CELP_WIN *w with the calculated values
	 */
	for (i = 0; i < 4; i++)
	{
		w->ac_index [i] = c->ac_index [i];
		w->ac_gain [i] = c->ac_gain_quant [i];
		w->fc_index [i] = c->fc_index [i];
		w->fc_gain [i] = c->fc_gain_quant [i];
		w->ac_gain_quant_ind [i] = c->ac_gain_quant_ind [i];
		w->fc_gain_quant_ind [i] = c->fc_gain_quant_ind [i];
	}

	/*
	 * Generate the bitstream for this window
	 */
	win2pckd (w->bitstream, w);

	c->counter += 1;

	return (ERROR_OK);
}

/*
 * Function: ccoder_sub_block_analysis
 *
 * Description:
 * 	Generates de AC/FC gains and indexes, for the c->sub_block sub block.
 *
 * 	This function is meant to be called from ccoder_set_samples, and
 * 	shouldn't be called from anywhere else.
 *
 * Return value:
 * 	ERROR_OK on sucess, some other error code on failure.
 */
unsigned int
ccoder_sub_block_analysis (CELP *c)
{
	int i, ret;

	/*
	 * Load the samples of this current sub block from the unsigned
	 * short vector c->data, to the double vector c->current_sample
	 */
	for (i = 0; i < c->win_length / 4; i++)
		c->current_sample [i] =
			c->data [i + (c->win_length / 4 * c->sub_block)];

	/* Generate the interpolated LSF/LPC coefficients for this sub block */
	ret = celp_interp_lsf (c);
	if (ret != ERROR_OK) return (ret);

	/* Evaluate the numerator and denumerator of the Perceptual Filter */
	ret = ccoder_set_pfilter (c);
	if (ret != ERROR_OK) return (ret);

	/* Apply the perceptual filter to the current sub block */
	ret = ccoder_apply_pfilter_to_sub_block (c);
	if (ret != ERROR_OK) return (ret);

	/* Remove the zero input response from the pfiltered signal */
	ret = ccoder_remove_zinput_response (c);
	if (ret != ERROR_OK) return (ret);

	/* Filter the whole AC, using the Perceptual Filter */
	ret = celp_apply_sfilter_to_ac (c);
	if (ret != ERROR_OK) return (ret);

	/* Search the best Index/Gain in the AC */
	ret = ccoder_search_ac (c);
	if (ret != ERROR_OK) return (ret);

	/* AC Gain Quantization */
	ret = celp_quant_ac_gain (c);
	if (ret != ERROR_OK) return (ret);

	/* Remove the response to the AC excitation from the target signal */
	ret = ccoder_ac_update_target_signal (c);
	if (ret != ERROR_OK) return (ret);

	/* Filter the whole FC, using the Perceptual Filter */
	ret = celp_apply_sfilter_to_fc (c);
	if (ret != ERROR_OK) return (ret);

	/* Search the best Index/Gain in the AC */
	ret = ccoder_search_fc (c);
	if (ret != ERROR_OK) return (ret);

	/* FC Gain Quantization */
	ret = celp_quant_fc_gain (c);
	if (ret != ERROR_OK) return (ret);

	ret = celp_set_complete_response (c);
	if (ret != ERROR_OK) return (ret);

	ret = celp_ac_update (c);
	if (ret != ERROR_OK) return (ret);

	return (ERROR_OK);
}

/*
 * Function: ccoder_ac_update_target_signal
 *
 * Description:
 * 	Remove the response to the select AC excitation from the target
 * 	signal.
 *
 * Return value:
 * 	ERROR_OK on sucess, other error code on failure.
 */
unsigned int ccoder_ac_update_target_signal (CELP *c)
{
	int i;

	for (i = 0; i < c->win_length / 4; i++)
		c->current_target_sample [i] -= c->ac_gain [c->sub_block] *
			c->ac_response[i];

	return (ERROR_OK);
}

unsigned int
ccoder_init_pfilter (CELP *c)
{
	int i;
	
	c->pfilter_num = (double *) calloc (c->num_coef + 1, sizeof (double));
	c->pfilter_den = (double *) calloc (c->num_coef + 1, sizeof (double));
	c->pfilter_states_num = (double *) calloc (c->num_coef + 1,
			sizeof (double));
	c->pfilter_states_den = (double *) calloc (c->num_coef + 1,
			sizeof (double));

	for (i = 0; i < c->num_coef; i++)
	{
		c->pfilter_states_num [i] = c->pfilter_states_den [i] =
			0.0;
	}

	return (ERROR_OK);
}

unsigned int
ccoder_set_pfilter (CELP *c)
{
	int i;
	
	c->pfilter_num [0] = 1;
	c->pfilter_den [0] = 1;

	for (i = 1; i <= c->num_coef; i++)
	{
		c->pfilter_num [i] = - c->interp_lpc_coef [i - 1];
		c->pfilter_den [i] = (pow (c->gamma, i)) * c->pfilter_num [i];
	}

	return (ERROR_OK);
}

unsigned int
ccoder_apply_pfilter_to_sub_block (CELP *c)
{
	filt_pz (c->current_sample, c->current_target_sample,
			c->pfilter_num, c->pfilter_den,
			c->pfilter_states_num, c->pfilter_states_den,
			c->win_length / 4, c->num_coef);

	return (ERROR_OK);
}

unsigned int
ccoder_remove_zinput_response (CELP *c)
{
	int i;

	for (i = 0; i < c->win_length / 4; i++)
	{
		c->current_target_sample [i] -= c->zinput_response [i];
	}

	return (ERROR_OK);
}

unsigned int
ccoder_search_ac (CELP *c)
{
	double error = 0.0;
	int NonZeroCorrelationCounter = 0;
	double Correlation_TargetSignal_SequenceAC;
	double AutoCorrelation_SequenceAC;
	double Correlation_TargetSignal_ResponseAC;
	double AutoCorrelation_ResponseAC;
	double Inverse_EMPQ;
	int i;

	/* Searching for the sequence which minimize the EMPQ */
	for (i = 0; i < c->ac_length; i++)
	{
		/* 
		 * Reading the candidate sequence from the AC which has
		 * already gone into the synthesis filter
		 */
		celp_get_ac_sequence (i, c->perc_ac_sample,
				c->current_perc_ac_sample, c);

		/*
		 * Calculating the index which minimizes the EMPQ using the
		 * candidate sequence from the AC
		 */
		Correlation_TargetSignal_SequenceAC = 
			inner_prod (c->current_target_sample,
					c->current_perc_ac_sample,
					c->win_length / 4);

		if (Correlation_TargetSignal_SequenceAC > 0)
		{
			NonZeroCorrelationCounter++;
			AutoCorrelation_SequenceAC =
				inner_prod (c->current_perc_ac_sample,
						c->current_perc_ac_sample, 
						c->win_length / 4);
			Inverse_EMPQ =
				Correlation_TargetSignal_SequenceAC *
				Correlation_TargetSignal_SequenceAC /
				AutoCorrelation_SequenceAC;

			/* 
			 * Comparing the EMPQ to the last and retains the
			 * smaller one, along with the index
			 */
			if (Inverse_EMPQ > error)
			{
				c->ac_index [c->sub_block] = i;
				error = Inverse_EMPQ;
			}
		}
	}

	/* If the search was successful, then... */
	if (NonZeroCorrelationCounter != 0)
	{
		/* Calculating the best excitation from the AC */
		celp_get_ac_sequence (c->ac_index [c->sub_block], c->ac_sample,
				c->current_ac_sample, c);

		/*
		 * Calculating the response to the best excitation from
		 * the AC
		 */
		filt_sp2 (c->current_ac_sample, c->ac_response, c->pfilter_den,
				c->win_length / 4, c->num_coef);

		/* Calculating the gain for the AC */
		Correlation_TargetSignal_ResponseAC =
			inner_prod (c->current_target_sample, c->ac_response,
					c->win_length / 4);

		AutoCorrelation_ResponseAC =
			inner_prod (c->ac_response, c->ac_response,
					c->win_length / 4);

		if (AutoCorrelation_ResponseAC != 0)
		{
			c->ac_gain [c->sub_block] =
				Correlation_TargetSignal_ResponseAC /
				AutoCorrelation_ResponseAC;
		} else
		{
			c->ac_gain [c->sub_block] = 0.0;
		}
	} else
	{
		for (i = 0; i < c->win_length / 4; i++)
		{
			c->current_ac_sample [i] = c->ac_response [i] = 0.0;
		}

		c->ac_index [c->sub_block] = 0;
		c->ac_gain [c->sub_block] = 0.0;
	}

	return (ERROR_OK);
}

unsigned int
ccoder_search_fc (CELP *c)
{
	double error = 0.0;
	int NonZeroCorrelationCounter = 0;
	double Correlation_TargetSignal_SequenceFC;
	double AutoCorrelation_SequenceFC;
	double Correlation_TargetSignal_ResponseFC;
	double AutoCorrelation_ResponseFC;
	double Inverse_EMPQ;
	int i;

	/* Searching for the sequence which minimize the EMPQ */
	for (i = 0; i < c->fc_length; i++)
	{
		celp_get_fc_sequence (i, c->perc_fc_sample,
				c->current_perc_fc_sample, c);

		/*
		 * Calculating the index which minimizes the EMPQ using the
		 * candidate sequence from the FC
		 */
		Correlation_TargetSignal_SequenceFC = 
			inner_prod (c->current_target_sample,
					c->current_perc_fc_sample,
					c->win_length / 4);

		if (Correlation_TargetSignal_SequenceFC > 0)
		{
			NonZeroCorrelationCounter++;
			AutoCorrelation_SequenceFC =
				inner_prod (c->current_perc_fc_sample,
						c->current_perc_fc_sample, 
						c->win_length / 4);

			Inverse_EMPQ =
				Correlation_TargetSignal_SequenceFC *
				Correlation_TargetSignal_SequenceFC /
				AutoCorrelation_SequenceFC;

			/* 
			 * Comparing the EMPQ to the last and retains the
			 * smaller one, along with the index
			 */
			if (Inverse_EMPQ > error)
			{
				c->fc_index [c->sub_block] = i;
				error = Inverse_EMPQ;
			}
		}

		/*
		 * Incrementing "increment_df" 
		 * So that the FC would have a step of 2 units
		 */
	}

	/* If the search was successful, then... */
	if (NonZeroCorrelationCounter != 0)
	{
		/* Reading the best excitation from the FC */
		celp_get_fc_sequence (c->fc_index [c->sub_block],
				c->fc_sample, c->current_fc_sample, c);

		/*
		 * Calculating the response to the best excitation from
		 * the FC
		 */
		filt_sp2 (c->current_fc_sample, c->fc_response, c->pfilter_den,
				c->win_length / 4, c->num_coef);

		/* Calculating the gain for the FC */
		Correlation_TargetSignal_ResponseFC =
			inner_prod (c->current_target_sample, c->fc_response,
					c->win_length / 4);

		AutoCorrelation_ResponseFC =
			inner_prod (c->fc_response, c->fc_response,
					c->win_length / 4);

		/*
		 * We MUST prevent a NaN here
		 * Actually, they shouldn't happen, but something else
		 * must be broken
		 */
		if (AutoCorrelation_ResponseFC != 0)
		{
			c->fc_gain [c->sub_block] =
				Correlation_TargetSignal_ResponseFC
				/ AutoCorrelation_ResponseFC;
		} else
		{
			/* 
			 * FIXME
			 * What should we do here?
			 */
			c->fc_gain [c->sub_block] = 0.0;
		}
	} else
	{
		for (i = 0; i < c->win_length / 4; i++)
		{
			c->current_fc_sample [i] = c->fc_response [i] = 0.0;
		}

		c->fc_index [c->sub_block] = 0;
		c->fc_gain [c->sub_block] = 0.0;
	}

	return (ERROR_OK);
}
