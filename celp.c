#include <stdio.h>
#include <stdlib.h>

/* libcelp */
#include "celp.h"
#include "error.h"

/* not libcelp */
#include "utils.h"
#include "wave.h"
#include "const.h"

#define LENGTH 160
#define NUM_COEF 10
#define FC_SIZE 256
#define AC_SIZE 1024
#define GAMMA 0.8

unsigned int
echo_loop (struct celp_options *option)
{
	int ret;
	struct wave_info iwav, owav;
	CELP *c, *d;
	CELP_WIN w;
	short data [LENGTH];

	c = ccoder_init (LENGTH, NUM_COEF, FC_SIZE, AC_SIZE, GAMMA);
	if (c == NULL)
	{
		fprintf (stderr, "A ccoder_init () retornou NULL.\n");
		return (ERROR_MISC);
	}

	d = cdecoder_init (LENGTH, NUM_COEF, FC_SIZE, AC_SIZE, GAMMA);
	if (d == NULL)
	{
		fprintf (stderr, "A cdecoder_init () retornou NULL.\n");
		return (ERROR_MISC);
	}

	owav.freq = 8000;
	owav.bps = 16;
	owav.nchan = 1;
	owav.nsamples = 0;

	if (option->mode == MODE_WAVE)
	{
		/* If the output file is a Wave file, write its header */
		ret = write_wave_header (option->ofd, &owav);
		if (ret != ERROR_OK)
		{
			ccoder_destroy (c);
			cdecoder_destroy (d);
			return (ret);
		}
	}

	if (option->mode == MODE_WAVE)
	{
		/* If the input file is a Wave file, read its header */
		ret = read_wave_header (option->ifd, &iwav);
		if (ret != ERROR_OK) return (ret);
	} else
	{
		iwav.freq = 8000;
		iwav.bps = 16;
		iwav.nchan = 1;
	}

	/* Write the header of the celp file */
	/*
	ret = write_celp_header (option->ofd, &iwav);
	if (ret != ERROR_OK) return (ret);
	*/

	while ((ret = read_wave_window (data, option->ifd, &iwav, LENGTH))
			== ERROR_OK)
	{
		ret = ccoder_set_samples (data, &w, c);
		if (ret != ERROR_OK)
		{
			ccoder_destroy (c);
			cdecoder_destroy (d);
			return (ret);
		}

		/*
		 * w->bitstream has the 22 bytes of this window
		 */
		
		ret = cdecoder_set_bitstream (data, &w, d);
		if (ret != ERROR_OK)
		{
			ccoder_destroy (c);
			cdecoder_destroy (d);
			return (ret);
		}

		if (option->mode == MODE_WAVE)
		{
			ret = write_wave_window (data, option->ofd, &owav,
					LENGTH, 1);
		} else
		{
			ret = write_wave_window (data, option->ofd, &owav,
					LENGTH, 0);
		}

	}

	ccoder_destroy (c);
	cdecoder_destroy (d);
	if (ret != ERROR_EOF) return (ret);

	return (ERROR_OK);
}

unsigned int
code_loop (struct celp_options *option)
{
	int ret;
	struct wave_info iwav;
	CELP *c;
	CELP_WIN w;
	short data [LENGTH];

	c = ccoder_init (LENGTH, NUM_COEF, FC_SIZE, AC_SIZE, GAMMA);
	if (c == NULL)
	{
		fprintf (stderr, "A ccoder_init () retornou NULL.\n");
		return (ERROR_MISC);
	}

	if (option->mode == MODE_WAVE)
	{
		/* If the input file is a Wave file, read its header */
		ret = read_wave_header (option->ifd, &iwav);
		if (ret != ERROR_OK) return (ret);
	} else
	{
		iwav.freq = 8000;
		iwav.bps = 16;
		iwav.nchan = 1;
	}

	/* Write the header of the celp file */
	/*
	ret = write_celp_header (option->ofd, &iwav);
	if (ret != ERROR_OK) return (ret);
	*/

	while ((ret = read_wave_window (data, option->ifd, &iwav, LENGTH))
			== ERROR_OK)
	{
		ret = ccoder_set_samples (data, &w, c);
		if (ret != ERROR_OK)
		{
			ccoder_destroy (c);
			return (ret);
		}


		ret = write_enc_window (&w, option->ofd);
		if (ret != ERROR_OK)
		{
			ccoder_destroy (c);
			return (ret);
		}
	}

	ccoder_destroy (c);
	if (ret != ERROR_EOF) return (ret);

	return (ERROR_OK);
}

unsigned int
decode_loop (struct celp_options *option)
{
	int ret;
	struct wave_info owav;
	CELP *d;
	CELP_WIN w;
	short data [LENGTH];

	d = cdecoder_init (LENGTH, NUM_COEF, FC_SIZE, AC_SIZE, GAMMA);
	if (d == NULL)
	{
		fprintf (stderr, "A cdecoder_init () retornou NULL.\n");
		return (ERROR_MISC);
	}

	owav.freq = 8000;
	owav.bps = 16;
	owav.nchan = 1;
	owav.nsamples = 0;

	if (option->mode == MODE_WAVE)
	{
		/* If the output file is a Wave file, write its header */
		ret = write_wave_header (option->ofd, &owav);
		if (ret != ERROR_OK)
		{
			cdecoder_destroy (d);
			return (ret);
		}
	}

	while ((ret = read_enc_window (&w, option->ifd)) == ERROR_OK)
	{
		/* changed to use the bitstream, instead of the full window */
		ret = cdecoder_set_bitstream (data, &w, d);
		if (ret != ERROR_OK)
		{
			cdecoder_destroy (d);
			return (ret);
		}

		if (option->mode == MODE_WAVE)
		{
			ret = write_wave_window (data, option->ofd, &owav,
					LENGTH, 1);
		} else
		{
			ret = write_wave_window (data, option->ofd, &owav,
					LENGTH, 0);
		}
		if (ret != ERROR_OK)
		{
			ccoder_destroy (d);
			return (ret);
		}
	}

	cdecoder_destroy (d);
	if (ret != ERROR_EOF) return (ret);

	return (ERROR_OK);
}

int
main (int argc, char **argv)
{
	int ret;
	struct celp_options options;

	/* Initialize options struct with default values */
	options.ifilename = options.ofilename = NULL;
	options.mode = MODE_WAVE;
	options.op = OP_ECHO;

	ret = parse_args (&options, argc, argv);
	if (ret != ERROR_OK) return (ret);

	/* Open I/O files */
	ret = open_files (&options);
	if (ret != ERROR_OK) return (ret);

	if (options.op == OP_CODE)
	{
		ret = code_loop (&options);
	} else if (options.op == OP_DECODE)
	{
		ret = decode_loop (&options);
	} else
	{
		ret = echo_loop (&options);
	}


	ret = close_files (&options);

	if (ret != ERROR_OK) return (ret);

	return (ERROR_OK);
}
