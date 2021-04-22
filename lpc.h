#ifndef __LPC_H__
#define __LPC_H__

#include "celp.h"
#include "celp_private.h"

unsigned int lpc_analysis (double *, int, double *, int);
unsigned int lpc2lsf (double *, double *, int);
unsigned int lsf2lpc (double *, double *, int);
unsigned int celp_interp_lsf (CELP *);
unsigned int celp_quant_lsf (CELP *);

#endif
