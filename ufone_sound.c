#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "ufone_sound.h"

#define PCM_NAME "default"



void
ufone_alsa_init (ufone_alsa_info *ainfo)
{
	int err;
	unsigned int rate = 8000;

	/* inicializar o gravador de som */
	if ((err = snd_pcm_open (&ainfo->capture_handle, PCM_NAME,
					SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
			 PCM_NAME,
			 snd_strerror (err));
		exit (1);
	}
	printf ("oi\n");	   
	if ((err = snd_pcm_hw_params_malloc (&ainfo->hw_params)) < 0)
	{
		fprintf (stderr, "cannot allocate hardware parameter \
				structure (%s)\n", snd_strerror (err));
		exit (1);
	}
			 
	if ((err = snd_pcm_hw_params_any (ainfo->capture_handle, ainfo->hw_params)) < 0)
	{
		fprintf (stderr, "cannot initialize hardware parameter \
				structure (%s)\n", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (ainfo->capture_handle, ainfo->hw_params,
					SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (ainfo->capture_handle, ainfo->hw_params,
					SND_PCM_FORMAT_S16_LE)) < 0)
	{
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (ainfo->capture_handle, ainfo->hw_params,
					&rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (ainfo->capture_handle,
					ainfo->hw_params, 1)) < 0)
	{
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params (ainfo->capture_handle, ainfo->hw_params)) < 0)
	{
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (ainfo->hw_params);

	if ((err = snd_pcm_prepare (ainfo->capture_handle)) < 0)
	{
		fprintf (stderr, "cannot prepare audio interface for use \
				(%s)\n", snd_strerror (err));
		exit (1);
	}


	/* inicizalizar o reprodutor de som */
	if ((err = snd_pcm_open (&ainfo->playback_handle, PCM_NAME,
					SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
			 PCM_NAME, snd_strerror (err));
		exit (1);
	}
	   
	if ((err = snd_pcm_hw_params_malloc (&ainfo->hw_params)) < 0)
	{
		fprintf (stderr, "cannot allocate hardware parameter \
				structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
			 
	if ((err = snd_pcm_hw_params_any (ainfo->playback_handle, ainfo->hw_params)) < 0)
	{
		fprintf (stderr, "cannot initialize hardware parameter \
				structure (%s)\n", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (ainfo->playback_handle, ainfo->hw_params, 
					SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (ainfo->playback_handle, ainfo->hw_params,
					SND_PCM_FORMAT_S16_LE)) < 0)
	{
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (ainfo->playback_handle,
					ainfo->hw_params, &rate, 0)) < 0)
	{
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (ainfo->playback_handle,
					ainfo->hw_params, 1)) < 0)
	{
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params (ainfo->playback_handle, ainfo->hw_params)) < 0)
	{
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (ainfo->hw_params);

	if ((err = snd_pcm_prepare (ainfo->playback_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use \
				(%s)\n", snd_strerror (err));
		exit (1);
	}

	return;
}

unsigned int
ufone_alsa_read (short *buf, unsigned int len, ufone_alsa_info *ainfo)
{
	int err;

	if ((err = snd_pcm_readi (ainfo->capture_handle, buf, len)) != len)
	{
		fprintf (stderr, "read from audio interface \
				failed (%s)\n", snd_strerror (err));
		exit (1);
	}
	return (0);
}

unsigned int
ufone_alsa_play (short *buf, unsigned int len, ufone_alsa_info *ainfo)
{
	int err;

	while (((err = snd_pcm_writei(ainfo->playback_handle, buf, len)) !=
				len))
	{
		printf ("erro: %d\n", err);
		if(err == -EPIPE)
		{
			snd_pcm_prepare (ainfo->playback_handle);
		} else
		{
			fprintf (stderr, "write to audio interface \
					failed (%s)\n",
					snd_strerror (err));
			exit (1);
		}
	}
	return (0);
}
