#ifndef __CELP_PRIVATE_H__
#define __CELP_PRIVATE_H__

/*
 * Private functions
 * You shouldn't use any of the following functions from outside
 */
/* Coder private functions */
unsigned int ccoder_search_ac (CELP *);
unsigned int ccoder_search_fc (CELP *);
unsigned int ccoder_ac_update_target_signal (CELP *);
unsigned int ccoder_init_pfilter (CELP *);
unsigned int ccoder_set_pfilter (CELP *);
unsigned int ccoder_apply_pfilter_to_sub_block (CELP *);
unsigned int ccoder_sub_block_analysis (CELP *);
unsigned int ccoder_interp_lsf (CELP *);
unsigned int ccoder_remove_zinput_response (CELP *);

/* Decoder private functions */
unsigned int cdecoder_ac_update (CELP *);
unsigned int cdecoder_get_sub_block (CELP *);
unsigned int cdecoder_interp_lsf (CELP *);

/* Common private functions */
unsigned int celp_init_ac (CELP *);
unsigned int celp_init_fc (CELP *);
unsigned int celp_quant_ac_gain (CELP *);
unsigned int celp_quant_fc_gain (CELP *);
unsigned int celp_get_ac_sequence (int, double *, double *, CELP *);
unsigned int celp_get_fc_sequence (int, double *, double *, CELP *);
unsigned int celp_apply_sfilter_to_ac (CELP *);
unsigned int celp_apply_sfilter_to_fc (CELP *);
unsigned int celp_set_complete_response (CELP *);
unsigned int celp_ac_update (CELP *);

#endif
