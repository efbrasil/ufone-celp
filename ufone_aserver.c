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

#include "ufone_sound.h"
#include "ufone_fifo.h"

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
#define LOCAL_UDP_PORT 20042
#define REMOTE_UDP_PORT 20041
#define MAXBUFLEN 200
#define BACKLOG 1
#define OUT_FILENAME "out.wav"

ufone_alsa_info ainfo;

int tcp_sock_fd, tcp_sock_new, udp_sock_recv, udp_sock_send;
char login [MAXBUFLEN], passwd [MAXBUFLEN], tel [MAXBUFLEN];
pthread_t thr_udp_send, thr_udp_recv; 

CELP *c, *d;

char *remote_host;

FILE *fd_adapt_x;
FILE *fd_adapt_d;

unsigned int
ufone_server_check_login (char *login, char *passwd)
{
	/* hehe */
	return (ERROR_OK);
}

unsigned int
ufone_server_dial (void)
{
	return (ERROR_OK);
}

void *
ufone_server_udp_send (void *list)
{
	short data [LENGTH];
	CELP_WIN w;
	int ret;
	unsigned char *pckd;
	int i;

	pckd = (unsigned char *) malloc (CELP_PCKD_SIZE);

	while ((ret = ufone_alsa_read (data, LENGTH, &ainfo)) == 0)
	{
		/* fwrite (data, sizeof (short), LENGTH, fd_adapt_d); */
		/* ufone_fifo_get (list, &adapt_d[0]); */

		for (i = 0; i < LENGTH; i++)
			data [i] = 1.2 * data [i];

		ret = ccoder_set_samples (data, &w, c);
		if (ret != ERROR_OK)
		{
			ccoder_destroy (c);
			return (NULL);
		}

		win2pckd (pckd, &w);

		if (send (udp_sock_send, pckd, CELP_PCKD_SIZE, 0) == -1)
			return (NULL);
	}

	return (NULL);
}

void *
ufone_server_udp_recv (void *list)
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
		pckd2win (&w, buf);

		celp_dequant_lsf (&w);
		celp_dequant_gains (&w);

		ret = cdecoder_set_window (data, &w, d);
		if (ret != ERROR_OK)
		{
			cdecoder_destroy (d);
			return (NULL);
		}

		/* fwrite (data, sizeof (short), LENGTH, fd_adapt_x); */
		/* ufone_fifo_add (list, &data[0]); */

		if (ufone_alsa_play (data, LENGTH, &ainfo) != 0)
			return (NULL);

	}
	
	close(udp_sock_recv);

	return (NULL);
}

unsigned int
ufone_server_udp_init (void)
{
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	struct hostent *he;

	/* socket que escuta */
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

	/* socket que envia */
	if ((udp_sock_send = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		return (ERROR_SOCKET);
	}

	if ((he = gethostbyname (remote_host)) == NULL)
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

	return (ERROR_OK);
}

unsigned int
ufone_server_sound_init (void)
{
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

	ufone_alsa_init (&ainfo);

	fd_adapt_x = fopen ("/tmp/adapt_x.raw", "wb");
	fd_adapt_d = fopen ("/tmp/adapt_d.raw", "wb");

	return (ERROR_OK);
}

unsigned int
ufone_server_tcp_init (void)
{
	int ret;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	socklen_t sin_size;
	char buf [MAXBUFLEN];
	int numbytes, i;
	UFONE_LIST *list;
	
	tcp_sock_fd = socket (AF_INET, SOCK_STREAM, 0);
	if (tcp_sock_fd == -1)
	{
		return (ERROR_SOCKET);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(TCP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', 8);
	
	ret = bind (tcp_sock_fd, (struct sockaddr *) &my_addr,
			sizeof(struct sockaddr));
	if (ret == -1)
	{
		return (ERROR_BIND);
	}
	
	listen (tcp_sock_fd, BACKLOG);
	
	sin_size = sizeof(struct sockaddr_in);

	fprintf (stderr, "Servidor inicializado. Aguardando conexao\n");

	tcp_sock_new = accept (tcp_sock_fd, (struct sockaddr *) &their_addr,
			&sin_size);
	if (tcp_sock_new == -1)
	{
		return (ERROR_ACCEPT);
	}

	remote_host = inet_ntoa (their_addr.sin_addr);

	close (tcp_sock_fd);

	fprintf (stderr, "Conexao estabelecida\nEndereco do cliente: %s\n",
			remote_host);

	numbytes = recv (tcp_sock_new, buf, MAXBUFLEN - 1, 0);
	if (numbytes == -1)
		return (ERROR_RECV);

	buf [numbytes] = '\0';
	for (i = 0; i < numbytes; i++)
		if ((buf [i] == '\r') || (buf [i] == '\n'))
			buf [i] = '\0';

	strcpy (login, strtok (buf, "|"));
	strcpy (passwd, strtok (NULL, "|"));
	strcpy (tel, strtok (NULL, "|"));
	printf ("login: %s\n", login);
	printf ("password: %s\n", passwd);
	printf ("tel: %s\n", tel);

	if (ufone_server_check_login (login, passwd) != ERROR_OK)
	{
		fprintf (stderr, "login/password error\n");
		numbytes = send (tcp_sock_new, "-PASSWD\n", 9, 0);
		if (numbytes == -1)
			return (ERROR_SEND);
		return (ERROR_PASSWD);
	}

	fprintf (stderr, "Discando\n");
	ufone_server_dial ();
	fprintf (stderr, "Abrindo audio\n");

	ret = ufone_server_udp_init ();
	if (ret != ERROR_OK) return (ret);

	ret = ufone_server_sound_init ();
	if (ret != ERROR_OK) return (ret);

	numbytes = send (tcp_sock_new, "+OK\n", 4, 0);
	if (numbytes == -1)
		return (ERROR_SEND);

	/* list = fifo_init (LENGTH); */

	/*
	 * agora temos que ficar monitorando o tcp, e em paralelo, executando
	 * a ufone_server_udp_recv
	 */
	if (pthread_create (&thr_udp_send, NULL, ufone_server_udp_send, list))
	{
		return (ERROR_PTHREAD_CREATE);
	}
	if (pthread_create (&thr_udp_recv, NULL, ufone_server_udp_recv, list))
	{
		return (ERROR_PTHREAD_CREATE);
	}

	while ((numbytes = recv (tcp_sock_new, buf, MAXBUFLEN - 1, 0)) > 0)
	{
		buf [numbytes] = '\0';
		printf ("tcp [%03d]: %s\n", numbytes, buf);
	}


	fprintf (stderr, "Conexao finalizada.\n");

	return (ERROR_OK);
}

int
main (void)
{
	int ret;

	fprintf (stderr, "Inicializando servidor\n");

	ret = ufone_server_tcp_init ();
	if (ret != ERROR_OK)
	{
		fprintf(stderr,"A ufone_server_tcp_init () retornou %d\n",ret);
		exit (ret);
	}

	close (tcp_sock_new);
	close (tcp_sock_fd);

	fclose (fd_adapt_x);
	fclose (fd_adapt_d);

	system ("mv /tmp/adapt_d.raw data/adapt_d.raw");
	system ("mv /tmp/adapt_x.raw data/adapt_x.raw");

	return (ERROR_OK);
}
