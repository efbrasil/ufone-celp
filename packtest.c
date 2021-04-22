#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "celp.h"
#include "pack.h"

void
fill_window (CELP_WIN *w)
{
	w->lsf_coef_index [0] = 12;
	w->lsf_coef_index [1] = 13;
	w->lsf_coef_index [2] = 2;
	w->lsf_coef_index [3] = 3;
	w->lsf_coef_index [4] = 4;
	w->lsf_coef_index [5] = 5;
	w->lsf_coef_index [6] = 6;
	w->lsf_coef_index [7] = 7;
	w->lsf_coef_index [8] = 0;
	w->lsf_coef_index [9] = 1;

	w->ac_index [0] = 4000;
	w->ac_index [1] = 4001;
	w->ac_index [2] = 4002;
	w->ac_index [3] = 4003;

	w->fc_index [0] = 3990;
	w->fc_index [1] = 3991;
	w->fc_index [2] = 3992;
	w->fc_index [3] = 3993;

	w->ac_gain_quant_ind [0] = 60;
	w->ac_gain_quant_ind [1] = 61;
	w->ac_gain_quant_ind [2] = 62;
	w->ac_gain_quant_ind [3] = 63;

	w->fc_gain_quant_ind [0] = 50;
	w->fc_gain_quant_ind [1] = 51;
	w->fc_gain_quant_ind [2] = 52;
	w->fc_gain_quant_ind [3] = 53;

	return;
}

int
main (void)
{
	unsigned char *pckd;
	CELP_WIN w1;
	CELP_WIN w2;

	memset (&w1, 0x00, sizeof (w1));
	memset (&w2, 0x00, sizeof (w2));

	pckd = (unsigned char *) malloc (CELP_PCKD_SIZE);
	
	fill_window (&w1);
	win2pckd (pckd, &w1);
	pckd2win (&w2, pckd);

	printf ("memcmp: %d\n",memcmp (&w1, &w2, sizeof (CELP_WIN)));

	return (0);
}
