#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "const.h"
#include "wave.h"
#include "error.h"

/*
 * Function: read_wave_header
 *
 * Description:
 * 	Check if the file is a valid wave, and if it is in the
 * 	8 kHz / mono / 16 bits/sample format
 *
 * Arguments
 * - filename: a string with the name of the file that should be checked
 *
 * Return: ERROR_OK if the file is valid, or one of the following:
 * - ERROR_CORRUPT_FILE: the file is not a valid wave file
 * - ERROR_WRONG_FREQ:   wrong frequency
 * - ERROR_WRONG_CHAN:   wrong number of channels (we only handle mono)
 * - ERROR_WRONG_BPS:    wrong bits/sample (we only handle 16 bits/sample)
 */
unsigned int
read_wave_header (FILE *fd, struct wave_info *wav)
{
	unsigned char buf [44];
	unsigned short int Bps;		/* bytes per sample */

	if (fread (buf, 1, 44, fd) != 44)
	{
		return (ERROR_CORRUPT_FILE);
	}

	/* Let's first check the fixed stuff */
	if (memcmp ((char *) buf +  0, (char *) "RIFF",             4) || \
	    memcmp ((char *) buf +  8, (char *) "WAVE",             4) || \
	    memcmp ((char *) buf + 12, (char *) "fmt ",             4) || \
	    memcmp ((char *) buf + 16, (char *) "\x10\x00\x00\x00", 4) || \
	    memcmp ((char *) buf + 20, (char *) "\x01\x00",         2) || \
	    memcmp ((char *) buf + 36, (char *) "data",             4))
	{
		return (ERROR_CORRUPT_FILE);
	}

	/* Now let's check our stuff */
	wav->freq  = *((unsigned int *) (buf + 24));
	wav->nchan = (unsigned short int) *(buf + 22);
	wav->bps   = (unsigned short int) *(buf + 34);

	/* Should we use #define'd values here? */
	if (wav->freq  != 8000) return (ERROR_WRONG_FREQ);
	if (wav->nchan != 1)    return (ERROR_WRONG_NCHAN);
	if (wav->bps   != 16)   return (ERROR_WRONG_BPS);

	/* Remember, bps = bits/sample and Bps = bytes/sample */
	Bps = wav->bps / 8;

	/* Finally, let's get the number of samples */
	wav->nsamples = ((*((unsigned int *) (buf + 40))) / Bps) / wav->nchan;
	
	return (ERROR_OK);
}

unsigned int
write_wave_header (FILE *fd, struct wave_info *wav)
{
	unsigned char buf [44];
	unsigned int Bpsec;			/* bytes per second */
	unsigned short int balign;		/* block alignment  */
	unsigned int file_size, data_size;

	if (fseek (fd, 0, SEEK_SET) != 0)
	{
		return (ERROR_FSEEK);
	}

	Bpsec = wav->nchan * wav->freq * wav->bps / 8;
	balign = wav->nchan * wav->bps / 8;
	data_size = wav->nsamples * wav->bps / 8;
	file_size = 44 - 8 + data_size;

	memcpy ((char *) buf +  0, "RIFF",			4);
	memcpy ((char *) buf +  4, (char *) &(file_size),	4);
	memcpy ((char *) buf +  8, "WAVE",			4);
	memcpy ((char *) buf + 12, "fmt ",			4);
	memcpy ((char *) buf + 16, "\x10\x00\x00\x00",		4);
	memcpy ((char *) buf + 20, "\x01\x00",			2);
	memcpy ((char *) buf + 22, (char *) &(wav->nchan),	2);
	memcpy ((char *) buf + 24, (char *) &(wav->freq),	4);
	memcpy ((char *) buf + 28, (char *) &(Bpsec),		4);
	memcpy ((char *) buf + 32, (char *) &(balign),		2);
	memcpy ((char *) buf + 34, (char *) &(wav->bps),	2);
	memcpy ((char *) buf + 36, "data",			4);
	memcpy ((char *) buf + 40, (char *) &(data_size),	4);

	if (fwrite (buf, 44, 1, fd) != 1)
	{
		return (ERROR_FWRITE);
	}

	if (fseek (fd, 0, SEEK_END) != 0)
	{
		return (ERROR_FSEEK);
	}

	return (ERROR_OK);
}

/*
 * Function: read_wave_window
 * Description:
 * 	read one window with 'win_size' samples from the file 'fd'.
 * 	The information about the file is in 'wav'.
 *
 * Return: the number of samples read
 *         0 = EOF
 *         -1 = error (probably not enough bytes to complete a sample)
 */
unsigned int
read_wave_window (short *buf, FILE *fd, struct wave_info *wav,
		unsigned int win_size)
{
	unsigned int i, ret;
	ret = fread (buf, wav->bps / 8, win_size, fd);

	if (ret == win_size)
	{
		return (ERROR_OK);
	} else if ((ret != 0) && (feof (fd)))
	{
		fprintf (stderr, "Incomplete window read.\n");
		fprintf (stderr, "Filling with %d zeros.\n",
				win_size - ret);
		/*
		 * If there's not enough data, we fill the rest of the window
		 * with zeros.
		 */
		for (i = 0; i < (win_size - ret); i++)
			buf [ret + i] = 0;

		return (ERROR_OK);
	} else if ((ret == 0) && (feof (fd)))
	{
		return (ERROR_EOF);
	} else
	{
		return (ERROR_FREAD);
	}
}

unsigned int
write_wave_window (short *buf, FILE *fd, struct wave_info *wav,
		unsigned int win_size, int header)
{
	unsigned int ret;

	wav->nsamples += win_size;

	if (header)
	{
		ret = write_wave_header (fd, wav);
		if (ret != ERROR_OK) return (ret);
	}
	
	ret = fwrite (buf, wav->bps / 8, win_size, fd);
	if (ret == win_size)
	{
		return (ERROR_OK);
	} else
	{
		return (ERROR_FWRITE);
	}
}

unsigned int
write_enc_window (CELP_WIN *w, FILE *fd)
{
	int ret;
	ret = fwrite (w->bitstream, 22, 1, fd);

	if (ret == 1)
	{
		return (ERROR_OK);
	} else
	{
		return (ERROR_FWRITE);
	}
}

unsigned int
read_enc_window (CELP_WIN *w, FILE *fd)
{
	int ret;
	ret = fread (w->bitstream, 22, 1, fd);

	if (ret == 1)
	{
		return (ERROR_OK);
	} else
	{
		return (ERROR_FREAD);
	}
}
