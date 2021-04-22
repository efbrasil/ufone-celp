#ifndef __CELP_H__
#define __CELP_H__

struct _CELP
{
	int win_length;			/* Window length, in samples 	*/
	int num_coef;			/* Number of LPC coefficients 	*/

	/* Voice samples */
	double *sample, *current_sample, *current_target_sample;
	double *window;
	short *data;
	int sub_block, sub_block_length;
	unsigned short first_window;

	/* Fixed Codebook info */
	int fc_length;
	int fc_number_of_samples;
	int fc_index [4];
	double fc_gain [4];
	double fc_gain_quant [4];
	int fc_gain_quant_ind [4];
	double *fc_sample;
	double *perc_fc_sample;
	double *fc_response;
	double *current_fc_sample;
	double *current_perc_fc_sample;

	/* Adaptive Codebook info */
	int ac_length;
	int ac_number_of_samples;
	int ac_index [4];
	double ac_gain [4];
	double ac_gain_quant [4];
	int ac_gain_quant_ind [4];
	double *ac_sample, *saved_ac_sample;
	double *perc_ac_sample;
	double *ac_response;
	double *current_ac_sample;
	double *current_perc_ac_sample;

	/* General Codebook info */
	double *excitation;
	double *response;

	/* LP Coefficients */
	double *lpc_coef;
	double *lsf_coef;
	double *dlsf_coef;
	double *quant_lsf_coef;
	int *quant_lsf_index;
	double *old_lsf_coef;
	double *interp_lpc_coef;
	double *interp_lsf_coef;

	/* Synth Filter */
	double *sfilter_state;
	double *sfilter_den;

	/* Perc Filter */
	double gamma;
	double *pfilter_num, *pfilter_den;
	double *pfilter_states_num, *pfilter_states_den;

	/* Zero Input Response */
	double *zinput;
	double *zinput_response;

	/* DEBUG */
	int debug;
	unsigned int counter;
	FILE *logfd;
};

struct _CELP_WIN
{
	int ac_index [4];
	double ac_gain [4];
	int fc_index [4];
	double fc_gain [4];
	double lsf_coef [10];
	int    ac_gain_quant_ind [4];
	int    fc_gain_quant_ind [4];
	int    lsf_coef_index [10];
	unsigned char bitstream [22];
};

typedef struct _CELP CELP;
typedef struct _CELP_WIN CELP_WIN;

/*
 * Public interface
 * These are the only functions and variables that should be used
 */
extern unsigned int ccoder_errno;
extern unsigned int cdecoder_errno;

/* Coder public functions */
CELP *ccoder_init (int, int, int, int, double);
CELP *ccoder_destroy (CELP *);
unsigned int ccoder_set_samples (short *, CELP_WIN *,CELP *);

/* Decoder public functions */
CELP *cdecoder_init (int, int, int, int, double);
CELP *cdecoder_destroy (CELP *);
unsigned int cdecoder_set_bitstream (short *, CELP_WIN *, CELP *);
unsigned int cdecoder_set_window (short *, CELP_WIN *,
		CELP *);

/* temp */
unsigned int celp_dequant_gains (CELP_WIN *);
unsigned int celp_dequant_lsf (CELP_WIN *);


#endif
