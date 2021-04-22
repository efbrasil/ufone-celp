#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef struct _WIN_LIST
{
	unsigned short *data;
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

#define WIN_LEN 10

void
print_data (unsigned short *data, unsigned int win_size, char *txt, int n)
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

	if (!aux)
	{
		printf ("(GET ) null\n");
	} else
	{
		print_data (aux->data, list->win_size, "GET ", list->fifo_size);
	}

	return (aux);
}

void
fifo_add (UFONE_LIST *list, unsigned short *data)
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

	print_data (aux->data, list->win_size, "ADD ", list->fifo_size);
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
ufone_fifo_add (UFONE_LIST *list, unsigned short *data)
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
ufone_fifo_get (UFONE_LIST *list, unsigned short *data)
{
	WIN_LIST *aux;
	int i;

	pthread_mutex_lock (list->fifo_mut);

	while (list->fifo_root == NULL)
	{
		printf ("(CONS) Fila vazia\n");
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

void *
thread_consumer (void *l)
{
	UFONE_LIST *list = (UFONE_LIST *) l;
	int sleep_sec;
	unsigned short data [WIN_LEN];


        srand (time (0) + 10);

	while (1)
	{
		sleep_sec = (int) ((10.0 * rand()) / (RAND_MAX + 1.0));
		/*
		printf ("(CONS) sleep = %d\n", sleep_sec);
		sleep (sleep_sec);
		*/

		ufone_fifo_get (list, &data[0]);
	}

	return (NULL);
}

void *
thread_producer (void *l)
{
	UFONE_LIST *list = (UFONE_LIST *) l;
	int sleep_sec, j;
	unsigned short data [WIN_LEN];


        srand (time (0));

	while (1)
	{
		sleep_sec = (int) ((10.0 * rand()) / (RAND_MAX + 1.0));
		/*
		printf ("(PROD) sleep = %d\n", sleep_sec);
		sleep (sleep_sec);
		*/

		for (j = 0; j < WIN_LEN; j++)
			data [j] = sleep_sec;

		ufone_fifo_add (list, &data[0]);
	}

	return (NULL);
}

void
teste_threads (void)
{
	pthread_t pro, con;
	UFONE_LIST *list;

	list = fifo_init (WIN_LEN);

	pthread_create (&pro, NULL, thread_producer, list);
	pthread_create (&con, NULL, thread_consumer, list);

	pthread_join (pro, NULL);
	pthread_join (con, NULL);

	return;
}

void
teste_simples (void)
{
	UFONE_LIST *list;
	WIN_LIST *aux;
	unsigned short i, j;
	unsigned short data [WIN_LEN];

	list = fifo_init (WIN_LEN);

	aux = fifo_get (list);
	fifo_print (list);
	fifo_print_back (list);
		
	for (i = 0; i < 5; i++)
	{
		for (j = 0; j < WIN_LEN; j++)
			data [j] = i;

		fifo_add (list, &data[0]);
	}

	fifo_print (list);
	fifo_print_back (list);

	for (i = 0; i < 8; i++)
	{
		aux = fifo_get (list);
		fifo_print (list);
		fifo_print_back (list);
	}
}

int main (void)
{
	teste_threads();

	return (0);
}
