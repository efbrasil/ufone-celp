#ifndef __UFONE_SOUND_H__
#define __UFONE_SOUND_H__

#include <alsa/asoundlib.h>

struct _ufone_alsa_info
{
	snd_pcm_t *capture_handle;
	snd_pcm_t *playback_handle;
	snd_pcm_hw_params_t *hw_params;
};
typedef struct _ufone_alsa_info ufone_alsa_info;

void ufone_alsa_init (ufone_alsa_info *);
unsigned int ufone_alsa_read (short *, unsigned int, ufone_alsa_info *);
unsigned int ufone_alsa_play (short *, unsigned int, ufone_alsa_info *);
#endif
