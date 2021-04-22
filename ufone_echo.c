#include <stdio.h>
#include <stdlib.h>

#include "ufone_sound.h"

#define LEN 160

int
main (int argc, char **argv)
{
	ufone_alsa_info ainfo;
	short buf [LEN];

	ufone_alsa_init (&ainfo);
	
	for (;;)
	{
		ufone_alsa_read (buf, LEN, &ainfo); 
		ufone_alsa_play (buf, LEN, &ainfo);
	}

	return (0);
}
