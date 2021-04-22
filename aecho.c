#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

int      
main (int argc, char *argv[])
{
	int err;
	short buf [160];
	snd_pcm_t *capture_handle;
	snd_pcm_t *playback_handle;
	snd_pcm_hw_params_t *hw_params;

	unsigned int rate = 44100;

	/* inicializar o gravador de som */
	if ((err = snd_pcm_open (&capture_handle, argv[1],
					SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
			 argv[1],
			 snd_strerror (err));
		exit (1);
	}
	printf ("oi\n");	   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
	{
		fprintf (stderr, "cannot allocate hardware parameter \
				structure (%s)\n", snd_strerror (err));
		exit (1);
	}
			 
	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0)
	{
		fprintf (stderr, "cannot initialize hardware parameter \
				structure (%s)\n", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params,
					SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params,
					SND_PCM_FORMAT_S16_LE)) < 0)
	{
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params,
					&rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (capture_handle,
					hw_params, 1)) < 0)
	{
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0)
	{
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (capture_handle)) < 0)
	{
		fprintf (stderr, "cannot prepare audio interface for use \
				(%s)\n", snd_strerror (err));
		exit (1);
	}


	/* inicizalizar o reprodutor de som */
	if ((err = snd_pcm_open (&playback_handle, argv[1],
					SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
			 argv[1], snd_strerror (err));
		exit (1);
	}
	   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
	{
		fprintf (stderr, "cannot allocate hardware parameter \
				structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
			 
	if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0)
	{
		fprintf (stderr, "cannot initialize hardware parameter \
				structure (%s)\n", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, 
					SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params,
					SND_PCM_FORMAT_S16_LE)) < 0)
	{
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (playback_handle,
					hw_params, &rate, 0)) < 0)
	{
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (playback_handle,
					hw_params, 1)) < 0)
	{
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0)
	{
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use \
				(%s)\n", snd_strerror (err));
		exit (1);
	}

	for (; ;)
	{
		if ((err = snd_pcm_readi (capture_handle, buf, 160)) != 160)
		{
			fprintf (stderr, "read from audio interface \
					failed (%s)\n", snd_strerror (err));
			exit (1);
		}

		snd_pcm_wait (playback_handle, 0);

		while (((err = snd_pcm_writei(playback_handle, buf, 160)) !=
					160))
		{
			printf ("erro: %d\n", err);
			if(err == -EPIPE)
			{
				snd_pcm_prepare (playback_handle);
			} else
			{
				fprintf (stderr, "write to audio interface \
						failed (%s)\n",
						snd_strerror (err));
				exit (1);
			}
		}
	}


	snd_pcm_close (capture_handle);
	snd_pcm_close (playback_handle);
	exit (0);
}
