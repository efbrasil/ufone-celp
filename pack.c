#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pack.h"
#include "celp.h"

void
dopack (unsigned char **bitstream, int index, int bitno, int *pos)
{
	
	int posLeft;

	/* Clear the bits before starting in a new byte */
	if ((*pos) == 0)
		**bitstream = 0;

	while (bitno > 0)
	{
		/* Jump to the next byte if end of this byte is reached */
		if (*pos == 8)
		{
			*pos = 0;
			(*bitstream)++;
			**bitstream=0;
		}
		
		posLeft = 8 - (*pos);
		
		/* Insert index into the bitstream */
		if (bitno <= posLeft)
		{
			**bitstream |= (unsigned char)(index<<(posLeft-bitno));
			*pos += bitno;
			bitno = 0;
		} else
		{
			**bitstream |= (unsigned char)(index>>(bitno-posLeft));
			*pos=8;
			index-=((index>>(bitno-posLeft))<<(bitno-posLeft));
			bitno -= posLeft;
		}
	}
}

void
unpack (unsigned char **bitstream, int *index, int bitno, int *pos) 
{
	int BitsLeft;
	
	*index=0;
	while (bitno>0)
	{
		/* 
		 * move forward in bitstream when the end of the 
		 * byte is reached
		 */ 
		if (*pos==8)
		{ 
			*pos=0; 
			(*bitstream)++; 
		} 
		
		BitsLeft=8-(*pos); 
		
		/* Extract bits to index */ 
		if (BitsLeft>=bitno)
		{ 
			*index+=((((**bitstream)<<(*pos)) & 0xFF)>>(8-bitno));
			*pos+=bitno;
			bitno=0;
		} else
		{ 
			if ((8-bitno)>0)
			{ 
				*index+=((((**bitstream)<<(*pos)) & 0xFF)>> 
						(8-bitno)); 
				*pos=8; 
			} else
			{ *index+=(((int)(((**bitstream)<<(*pos)) & 0xFF))<< 
					(bitno-8)); 
			*pos=8; 
			} 
			bitno-=BitsLeft; 
		} 
	} 
}

void
pckd2win (CELP_WIN *w, unsigned char *pckd)
{
	int pos = 0;
	int mask;

	/* coeficientes LSF */
	unpack (&pckd, &w->lsf_coef_index [0], 4, &pos);
	unpack (&pckd, &w->lsf_coef_index [1], 4, &pos);
	unpack (&pckd, &w->lsf_coef_index [2], 3, &pos);
	unpack (&pckd, &w->lsf_coef_index [3], 3, &pos);
	unpack (&pckd, &w->lsf_coef_index [4], 3, &pos);
	unpack (&pckd, &w->lsf_coef_index [5], 3, &pos);
	unpack (&pckd, &w->lsf_coef_index [6], 3, &pos);
	unpack (&pckd, &w->lsf_coef_index [7], 3, &pos);
	unpack (&pckd, &w->lsf_coef_index [8], 3, &pos);
	unpack (&pckd, &w->lsf_coef_index [9], 3, &pos);

	/* indices AC */
	unpack (&pckd, &w->ac_index [0], 12, &pos);
	unpack (&pckd, &w->ac_index [1], 12, &pos);
	unpack (&pckd, &w->ac_index [2], 12, &pos);
	unpack (&pckd, &w->ac_index [3], 12, &pos);

	/* indices FC */
	unpack (&pckd, &w->fc_index [0], 12, &pos);
	unpack (&pckd, &w->fc_index [1], 12, &pos);
	unpack (&pckd, &w->fc_index [2], 12, &pos);
	unpack (&pckd, &w->fc_index [3], 12, &pos);

	/* ganhos AC */
	unpack (&pckd, &w->ac_gain_quant_ind [0], 6, &pos);
	unpack (&pckd, &w->ac_gain_quant_ind [1], 6, &pos);
	unpack (&pckd, &w->ac_gain_quant_ind [2], 6, &pos);
	unpack (&pckd, &w->ac_gain_quant_ind [3], 6, &pos);

	/* ganhos FC */
	unpack (&pckd, &w->fc_gain_quant_ind [0], 6, &pos);
	unpack (&pckd, &w->fc_gain_quant_ind [1], 6, &pos);
	unpack (&pckd, &w->fc_gain_quant_ind [2], 6, &pos);
	unpack (&pckd, &w->fc_gain_quant_ind [3], 6, &pos);

	/* set the unused bits of the FC index to zero */
	mask = (1 << FC_INDEX_SIZE) - 1;
	w->fc_index [0] &= mask;
	w->fc_index [1] &= mask;
	w->fc_index [2] &= mask;
	w->fc_index [3] &= mask;

	/* set the unused bits of the AC index to zero */
	mask = (1 << AC_INDEX_SIZE) - 1;
	w->ac_index [0] &= mask;
	w->ac_index [1] &= mask;
	w->ac_index [2] &= mask;
	w->ac_index [3] &= mask;

	return;
}

void
win2pckd (unsigned char *pckd, CELP_WIN *w)
{
	int pos = 0;

	/* coeficientes LSF */
	dopack (&pckd, w->lsf_coef_index [0], 4, &pos);		/*   0 */
	dopack (&pckd, w->lsf_coef_index [1], 4, &pos);		/*   4 */
	dopack (&pckd, w->lsf_coef_index [2], 3, &pos);		/*   8 */
	dopack (&pckd, w->lsf_coef_index [3], 3, &pos);		/*  11 */ 
	dopack (&pckd, w->lsf_coef_index [4], 3, &pos);		/*  14 */
	dopack (&pckd, w->lsf_coef_index [5], 3, &pos);		/*  17 */
	dopack (&pckd, w->lsf_coef_index [6], 3, &pos);		/*  20 */
	dopack (&pckd, w->lsf_coef_index [7], 3, &pos);		/*  23 */
	dopack (&pckd, w->lsf_coef_index [8], 3, &pos);		/*  26 */
	dopack (&pckd, w->lsf_coef_index [9], 3, &pos);		/*  29 */

	/* indices AC */
	dopack (&pckd, w->ac_index [0], 12, &pos);		/*  32 */
	dopack (&pckd, w->ac_index [1], 12, &pos);		/*  44 */
	dopack (&pckd, w->ac_index [2], 12, &pos);		/*  56 */
	dopack (&pckd, w->ac_index [3], 12, &pos);		/*  68 */

	/* indices FC */
	dopack (&pckd, w->fc_index [0], 12, &pos);		/*  80 */
	dopack (&pckd, w->fc_index [1], 12, &pos);		/*  92 */
	dopack (&pckd, w->fc_index [2], 12, &pos);		/* 104 */
	dopack (&pckd, w->fc_index [3], 12, &pos);		/* 116 */

	/* ganhos AC */
	dopack (&pckd, w->ac_gain_quant_ind [0], 6, &pos);	/* 128 */
	dopack (&pckd, w->ac_gain_quant_ind [1], 6, &pos);	/* 134 */
	dopack (&pckd, w->ac_gain_quant_ind [2], 6, &pos);	/* 140 */
	dopack (&pckd, w->ac_gain_quant_ind [3], 6, &pos);	/* 146 */

	/* ganhos FC */
	dopack (&pckd, w->fc_gain_quant_ind [0], 6, &pos);	/* 152 */
	dopack (&pckd, w->fc_gain_quant_ind [1], 6, &pos);	/* 158 */
	dopack (&pckd, w->fc_gain_quant_ind [2], 6, &pos);	/* 164 */
	dopack (&pckd, w->fc_gain_quant_ind [3], 6, &pos);	/* 170 */

	return;
}
