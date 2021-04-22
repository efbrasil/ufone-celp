#ifndef _WAVE_H_
#define _WAVE_H_

#include <stdio.h>
#include "celp.h"
#include "const.h"

struct wave_info
{
	unsigned int freq;              /* sampling rate      */
	unsigned short nchan;           /* number of channels */
	unsigned short bps;             /* bits per sample    */
	unsigned int nsamples;          /* number of samples  */
};

unsigned int read_wave_header (FILE *, struct wave_info *);
unsigned int write_wave_header (FILE *, struct wave_info *);
unsigned int read_wave_window (short *, FILE *, struct wave_info *,
		unsigned int);
unsigned int write_wave_window (short *, FILE *, struct wave_info *,
		unsigned int, int);
unsigned int write_enc_window (CELP_WIN *, FILE *);
unsigned int read_enc_window (CELP_WIN *, FILE *);

#endif
