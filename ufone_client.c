#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>

/* libcelp */
#include "celp.h"
#include "error.h"

/* not libcelp */
#include "utils.h"
#include "wave.h"
#include "const.h"
#include "pack.h"

#define LENGTH 160
#define NUM_COEF 10
#define FC_SIZE 256
#define AC_SIZE 1024
#define GAMMA 0.8

#define TCP_PORT 20041
#define LOCAL_UDP_PORT 20041
#define REMOTE_UDP_PORT 20042
#define MAXBUFLEN 200
#define IN_FILENAME "gabriel.wav"
#define REMOTE_HOST "192.168.123.2"

char *login, *passwd, *tel;

int tcp_sock_fd;
pthread_mutex_t sig_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sig_cond = PTHREAD_COND_INITIALIZER;

pthread_t thr_udp_send, thr_udp_recv;
int udp_sock_send, udp_sock_recv;

CELP *c, *d;

int bytes_in = 0;
int bytes_out = 0;
time_t time_init;

/* /dev/dsp stuff */
int audio;
#define MODE AFMT_S16_LE

void
ufone_client_signal (int signum)
{
	pthread_mutex_lock (&sig_mut);
	pthread_cond_signal (&sig_cond);
	pthread_mutex_unlock (&sig_mut);
}

unsigned int
ufone_client_udp_init (void)
{
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	struct hostent *he;

	/* inicializa socket que escuta */
	if ((udp_sock_recv = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		return (ERROR_SOCKET);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons (LOCAL_UDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', 8);
	
	if (bind (udp_sock_recv, (struct sockaddr *)&my_addr,
				sizeof (struct sockaddr)) == -1)
	{
		return (ERROR_BIND);
	}

	/* inicializa socket que envia */
	if ((udp_sock_send = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		return (ERROR_SOCKET);
	}

	if ((he = gethostbyname (REMOTE_HOST)) == NULL)
	{
		return (ERROR_RESOLV);
	}

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons (REMOTE_UDP_PORT);
	their_addr.sin_addr = *((struct in_addr *) he->h_addr);
	memset (&(their_addr.sin_zero), '\0', 8);

	if (connect (udp_sock_send, (struct sockaddr *) &their_addr,
				sizeof (struct sockaddr)) == -1)
	{
		return (ERROR_CONNECT);
	}

	printf ("UDP inicializado.\n");
	return (ERROR_OK);
}

void *
ufone_client_udp_send (void *dummy)
{
	short data [LENGTH];
	CELP_WIN w;
	int ret;
	unsigned char *pckd;

	pckd = (unsigned char *) malloc (CELP_PCKD_SIZE);

	while ((ret = read (audio, data, LENGTH * 2)) != -1)
	{
		ret = ccoder_set_samples (data, &w, c);
		if (ret != ERROR_OK)
		{
			ccoder_destroy (c);
			return (NULL);
		}

		win2pckd (pckd, &w);

		if (send (udp_sock_send, pckd, CELP_PCKD_SIZE, 0) == -1)
			return (NULL);

		bytes_out += CELP_PCKD_SIZE;
	}

	return (NULL);
}

void *
ufone_client_udp_recv (void *dummy)
{
	int addr_len, numbytes, ret;
	struct sockaddr_in their_addr;
	unsigned char buf [MAXBUFLEN];
	short data [LENGTH];
	CELP_WIN w;


	addr_len = sizeof (struct sockaddr);

	while ((numbytes = recvfrom (udp_sock_recv,buf, MAXBUFLEN-1, 0,
				(struct sockaddr *) &their_addr,
				(socklen_t *) &addr_len)) > 0)
	{
		bytes_in += numbytes;
		
		pckd2win (&w, buf);

		celp_dequant_lsf (&w);
		celp_dequant_gains (&w);

		ret = cdecoder_set_window (data, &w, d);
		if (ret != ERROR_OK)
		{
			cdecoder_destroy (d);
			return (NULL);
		}

		if (write (audio, data, LENGTH * 2) == -1)
			return (NULL);

	}
	
	close(udp_sock_recv);

	return (NULL);
}

unsigned int
ufone_client_sound_init (void)
{
	int format;

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

	if ((audio = open ("/dev/dsp", O_RDWR, 0)) == -1)
		return (ERROR_OPEN);

	if (ioctl (audio, SNDCTL_DSP_SETDUPLEX, 0) == -1)
		return (ERROR_IOCTL);

	format = MODE;

	if (ioctl (audio, SNDCTL_DSP_SETFMT, &format) == -1)
		return (ERROR_IOCTL);
		
	if (format != MODE)
		return (ERROR_FMT);

	format = 0;
	
	if (ioctl (audio, SNDCTL_DSP_STEREO, &format) == -1)
		return (ERROR_IOCTL);

	if (format != 0)
		return (ERROR_CHANNELS);

	format = 8000;

	if (ioctl (audio, SNDCTL_DSP_SPEED, &format) == -1)
		return (ERROR_IOCTL);

	return (ERROR_OK);
}

unsigned int
ufone_client_tcp_init (void)
{
	struct sockaddr_in their_addr;
	struct hostent *he;
	char buf [MAXBUFLEN];
	int numbytes;
	time_t delta;
	double bytes;


	if ((he = gethostbyname (REMOTE_HOST)) == NULL)
	{
		return (ERROR_RESOLV);
	}
	
	tcp_sock_fd = socket (AF_INET, SOCK_STREAM, 0);
	if (tcp_sock_fd == -1)
	{
		return (ERROR_SOCKET);
	}
	
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons (TCP_PORT);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);
	
	fprintf (stderr, "Inicializando conexao\n");

	if (connect (tcp_sock_fd, (struct sockaddr *) &their_addr,
				sizeof (struct sockaddr)) == -1)
	{
		return (ERROR_CONNECT);
	}

	fprintf (stderr, "Conexao estabelecida\n");

	/* abre os sockets udp */
	ufone_client_udp_init ();
	ufone_client_sound_init ();

	snprintf (buf, MAXBUFLEN - 1, "%s|%s|%s", login, passwd, tel);

	fprintf (stderr, "Enviando login\n");

	if (send (tcp_sock_fd, buf, strlen (buf), 0) == -1)
		return (ERROR_SEND);

	numbytes = recv (tcp_sock_fd, buf, MAXBUFLEN - 1, 0);
	if (numbytes == -1)
		return (ERROR_RECV);

	buf [numbytes] = '\0';
	if (strcmp ("+OK\n", buf))
	{
		return (ERROR_PASSWD);
	}

	fprintf (stderr, "Login aceito\n");

	/*
	 * cria uma thread para enviar os dados e uma pra receber
	 */
	if (pthread_create (&thr_udp_send, NULL, ufone_client_udp_send, NULL))
	{
		return (ERROR_PTHREAD_CREATE);
	}
	if (pthread_create (&thr_udp_recv, NULL, ufone_client_udp_recv, NULL))
	{
		return (ERROR_PTHREAD_CREATE);
	}

	pthread_mutex_lock (&sig_mut);
	pthread_cond_wait (&sig_cond, &sig_mut);
	pthread_mutex_unlock (&sig_mut);

	delta = time (NULL) - time_init;
	printf ("tempo: %d\n", (int) delta);
	fprintf (stderr, "Saindo\n");

	bytes = bytes_in;
	bytes = bytes / delta;
	printf ("Entrada: %f\n", bytes);
	bytes = bytes_out;
	bytes = bytes / delta;
	printf ("Saida: %f\n", bytes);

	close (tcp_sock_fd);

	return (ERROR_OK);
}

int
main (int argc, char *argv[])
{
	int ret;

	signal (SIGINT, ufone_client_signal);

	if (argc != 4)
	{
		fprintf (stderr, "Uso: %s login senha telefone\n", argv [0]);
		exit (0);
	}

	login = argv [1];
	passwd = argv [2];
	tel = argv [3];

	time_init = time (NULL);

	ret = ufone_client_tcp_init ();
	if (ret != ERROR_OK)
	{
		fprintf(stderr,"A ufone_client_tcp_init () retornou %d\n",ret);
		exit (ret);
	}
	close (audio);

	return (ERROR_OK);
}
