#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "ufone_fifo.h"

#define WIN_LEN 160

void
print_data (short *data, unsigned int win_size, char *txt, int n)
{
	int i;

	printf ("(%s) ", txt);

	for (i = 0; i < win_size; i++)
		printf ("%d ", data[i]);

	printf ("(%d) ", n);

	printf ("\n");

	return;
}


/*
 * A funcao fifo_get le o primeiro elemento da fila, e o remove.
 */
WIN_LIST *
fifo_get (UFONE_LIST *list)
{
	WIN_LIST *aux;
	WIN_LIST **root;
	WIN_LIST **last;

	root = &(list->fifo_root);
	last = &(list->fifo_last);

	/* Salva o valor que estava no inicio da fila */
	aux = *root;

	/* So faz alguma coisa se a fila nao estiver vazia */
	if (*root != NULL)
	{
		if ((*root)->next == NULL)
		{
			/* Caso o primeiro elemento tambem seja o ultimo,
			 * a funcao atualiza o ponteiro para o final
			 * da fila */
			*last = NULL;
		} else
		{
			/* Caso contrario, ela atualiza o segundo valor,
			 * para que ele saiba que agora e o primeiro */
			(*root)->next->back = NULL;
		}

		/* O ponteiro para o inicio da fila agora aponta para
		 * o segundo item */
		(*root) = (*root)->next;

		/* Decrementa o tamanho da fila */
		list->fifo_size--;
	}

#if 0
	if (!aux)
	{
		printf ("(GET ) null\n");
	} else
	{
		print_data (aux->data, list->win_size, "GET ", list->fifo_size);
	}
#endif

	return (aux);
}

void
fifo_add (UFONE_LIST *list, short *data)
{
	WIN_LIST *aux, **root, **last;
	int i;

	root = &(list->fifo_root);
	last = &(list->fifo_last);

	/* Aloca o espaco necessario para o novo item */
	aux = malloc (sizeof (WIN_LIST));
	aux->next = NULL;
	aux->back = *last;

	aux->data = calloc (list->win_size, sizeof (*data));
	for (i = 0; i < list->win_size; i++)
		aux->data [i] = data [i];

	if (*last == NULL)
	{
		/* Caso a fila esteja vazia, esse novo item tambem
		 * sera o primeiro */
		*root = aux;
	} else
	{
		/* Caso contrario, o item que era o ultimo agora aponta para
		 * esse */
		(*last)->next = aux;
	}

	/* O novo item sempre sera o ultimo da fila */
	*last = aux;

	/* Incrementa o tamanho da fila */
	list->fifo_size++;

#if 0
	print_data (aux->data, list->win_size, "ADD ", list->fifo_size);
#endif
}

/*
 * Imprime todos os itens da lista
 */
void
fifo_print (UFONE_LIST *list)
{
	WIN_LIST *aux;
	WIN_LIST **root;
	int i;

	root = &(list->fifo_root);

	aux = *root;

	printf ("(PRNT) Tamanho da fila: %d\n", list->fifo_size);

	while (aux)
	{
		printf ("(PRNT) ");
		for (i = 0; i < list->win_size; i++)
			printf ("%d ", aux->data[i]);
		printf ("\n");
		aux = aux->next;
	}
}

/*
 * Imprime todos os itens da lista, do fim para o inicio
 */
void
fifo_print_back (UFONE_LIST *list)
{
	WIN_LIST *aux;
	WIN_LIST **last;
	int i;

	last = &(list->fifo_last);

	aux = *last;

	printf ("(BACK) Tamanho da fila: %d\n", list->fifo_size);

	while (aux)
	{
		printf ("(BACK) ");
		for (i = 0; i < list->win_size; i++)
			printf ("%d ", aux->data[i]);
		printf ("\n");
		aux = aux->back;
	}
}

UFONE_LIST *
fifo_init (int win_size)
{
	UFONE_LIST *list;

	list = malloc (sizeof (UFONE_LIST));
	list->fifo_root = NULL;
	list->fifo_last = NULL;
	list->fifo_size = 0;
	list->win_size = win_size;

	list->fifo_mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init (list->fifo_mut, NULL);

	list->fifo_not_empty =
		(pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (list->fifo_not_empty, NULL);

	return (list);
}


/*
 * Essa funcao e um wrapper para a fifo_add, adicionando as protecoes
 * de exclusao mutua
 */
void
ufone_fifo_add (UFONE_LIST *list, short *data)
{
	pthread_mutex_lock (list->fifo_mut);
	fifo_add (list, data);
	pthread_mutex_unlock (list->fifo_mut);
	pthread_cond_signal (list->fifo_not_empty);
}

/*
 * Essa funcao e um wrapper para a fifo_get, adicionando as protecoes
 * de exclusao mutua
 */
void
ufone_fifo_get (UFONE_LIST *list, short *data)
{
	WIN_LIST *aux;
	int i;

	pthread_mutex_lock (list->fifo_mut);

	while (list->fifo_root == NULL)
	{
#if 0
		printf ("(CONS) Fila vazia\n");
#endif
		pthread_cond_wait (list->fifo_not_empty, list->fifo_mut);
	}

	aux = fifo_get (list);
	pthread_mutex_unlock (list->fifo_mut);

	if (aux == NULL)
	{
		for (i = 0; i < list->win_size; i++)
			data [i] = 0;
	} else
	{
		for (i = 0; i < list->win_size; i++)
			data [i] = aux->data [i];

		free (aux->data);
		free (aux);
	}


	return;
}
