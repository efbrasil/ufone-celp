#ifndef __UFONE_FIFO_H__
#define __UFONE_FIFO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef struct _WIN_LIST
{
	short *data;
	struct _WIN_LIST *next;
	struct _WIN_LIST *back;
} WIN_LIST;

typedef struct _UFONE_LIST
{
	WIN_LIST *fifo_root;
	WIN_LIST *fifo_last;
	unsigned int fifo_size;
	unsigned int win_size;
	pthread_mutex_t *fifo_mut;
	pthread_cond_t *fifo_not_empty;
} UFONE_LIST;

void print_data (short *data, unsigned int win_size, char *txt, int n);
WIN_LIST *fifo_get (UFONE_LIST *list);
void fifo_add (UFONE_LIST *list, short *data);
void fifo_print (UFONE_LIST *list);
void fifo_print_back (UFONE_LIST *list);
UFONE_LIST *fifo_init (int win_size);
void ufone_fifo_add (UFONE_LIST *list, short *data);
void ufone_fifo_get (UFONE_LIST *list, short *data);

#endif
