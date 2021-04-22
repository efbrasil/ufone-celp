#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "error.h"

unsigned int
hamming (double *window, int length)
{
	int i;
	double pi;

	pi = 4.0 * atan (1.0);
                                                                                
	for (i = 0; i < length; i++)
		window [i] = 0.54 - 0.46 * cos ((2.0 * pi * i) / (length - 1));

	return (ERROR_OK);
}

unsigned int
apply_window (double *sample, double *window, int length)
{
	int i;

	for (i = 0; i < length; i++)
		sample [i] *= window [i];

	return (ERROR_OK);
}

void cheby1 (double *g, int ord) 
{
	int i, j;

	for (i = 2; i<= ord; i++)
	{
		for (j = ord; j > i; j--)
			g [j - 2] -= g [j];
		g [j - 2] -= 2.0 * g [j];
	}
}

void cacm283 (double a [], int ord, double r [])
{
	int i, k;
	double val, p, delta, error;
	double rooti;
	int swap;

	for(i = 0; i < ord; i++)
		r [i] = 2.0 * (i + 0.5) / ord - 1.0;
  
	for(error = 1 ; error > 1.e-12; )
	{
		error = 0;
		for(i = 0; i < ord; i++)
		{
			/* Update each point. */
			rooti = r [i];
			val = a [ord];
			p = a [ord];
			for (k = ord - 1; k >= 0; k--)
			{
				val = val * rooti + a [k];
				if (k != i)
					p *= rooti - r [k];
			}
			delta = val / p;
			r [i] -= delta;
			error += delta * delta;
		}
	}

	/* Do a simple bubble sort to get them in the right order. */
	do
	{
		double tmplsp;
		swap = 0;
		for (i = 0; i < ord - 1; i++)
		{
			if (r [i] < r [i + 1])
			{
				tmplsp = r [i];
				r [i] = r [i + 1];
				r [i + 1] = tmplsp;
				swap++;
			}
		}
	} while (swap > 0);
}

void
polimulti (double poli1[], double poli2[], double polir[], int ordem1,
	   int ordem2) 
{
	int i, j;
	
	/* Multiplying the polynomials */
	for (j = 0; j <= (ordem1 + ordem2); j++)
	{
		polir [j] = 0;
		for (i = 0; i <= ordem1; i++)
		{
			if ((j - i) >= 0 && (j - i) <= ordem2)
				polir[j] += poli1[i] * poli2[j - i];
		}
	}
}

void
filt_pz (double s_input[], double s_output[], double num_filter[],
	 double den_filter[], double buffer_e[], double buffer_s[],
	 int signal_size, int buffer_size)
{
	int i, j;
	double acum_e, acum_s;

	/* Begin the filtering */
	for (j = 0; j < signal_size; j++)
	{
		/*
		 * Multiply the samples contained in the buffers by the 
		 * coefficients
		 */
		acum_e = acum_s = 0.0;
		for (i = 0; i < buffer_size; i++)
		{
			acum_e += buffer_e [i] * num_filter [i + 1];
			acum_s += buffer_s [i] * den_filter [i + 1];
		}

		/* Calculating output */
		s_output [j] = num_filter [0] * s_input [j] + acum_e - acum_s;
		s_output [j] /= den_filter [0];

		/* Updating buffers */
		for (i = buffer_size - 1; i > 0; i--)
		{
			buffer_e [i] = buffer_e [i - 1];
			buffer_s [i] = buffer_s [i - 1];
		}
		buffer_e [0] = s_input [j];
		buffer_s [0] = s_output [j];
	}
}

void filt_sp1(double s_input[], double s_output[], double den_filter[], 
	      double buffer[], int signal_size, int buffer_size)
{
	int i, j;
	double acum;
	
	/* Begin the filtering */
	for (j = 0; j < signal_size; j++)
	{
		/*
		 * Multiply the samples contained in the buffers by
		 * the coefficients
		 */
		acum = 0.0;
		for (i = 0; i < buffer_size; i++)
			acum += buffer[i] * den_filter[i + 1];
		
		/* Calculating output */
		s_output [j] = s_input [j] - acum;
		s_output [j] /= den_filter [0];
		
		/* Updating buffers */
		for (i = buffer_size - 1; i > 0; i--)
			buffer [i] = buffer [i-1];
		
		buffer [0] = s_output [j];
	}
}

unsigned int
filt_sp2(double s_input[], double s_output[], double den_filter[],
		int signal_size, int buffer_size)
{
	int i, j;
	double acum, *buffer;


	/* Allocating memory */
	if ((buffer = (double * ) calloc (buffer_size,
					sizeof (double))) == NULL)
	{
		return (ERROR_MALLOC);
	}
	
	/* Begin the filtering */
	for (j = 0; j < signal_size; j++)
	{
		/* 
		 * Multiply the samples contained in the buffers by the
		 * coefficients
		 */
		acum = 0.0;
		
		for (i = 0; i < buffer_size; i++)
			acum += buffer [i] * den_filter [i + 1];
		
		/* Calculating output */
		s_output [j] = s_input [j] - acum;
		s_output [j] /= den_filter [0];
		
		/* Updating buffer */
		for (i = buffer_size - 1; i > 0; i--)
			buffer [i] = buffer [i - 1];
		
		buffer [0] = s_output [j];
	}

	free (buffer);

	return (ERROR_OK);
}

double
inner_prod (double vector1[], double vector2[], int size)
{
	int j;
	double prod;

	/* Calculating the inner product */
	prod = 0.0;
	for (j = 0; j < size; j++)
		prod += vector1 [j] * vector2 [j];
	
	return (prod);
}

unsigned int
load_from_file(const char file[], double variable[], unsigned int num_itens)
{
	FILE *fp;

	/* Opens binary file */
	if ((fp = fopen (file,"rb")) == NULL)
	{
		return (ERROR_FOPEN);
	}
	
	/* Reads data from file */
	if (fread (variable, sizeof (double), num_itens, fp) != num_itens)
	{
		fclose (fp);
		return (ERROR_FREAD);
	}

	/* Closes data file */
	fclose (fp);

	return (ERROR_OK);
}

double quant_esc(double x, double particao[], double codebook[],
		int tam_codebook, int *index)
{
	int i, j;

	/* Initializing the codebook index */
	i = 0;

	/* Calculating the quantized value */
	for (j = 0; j < tam_codebook - 1; j++)
	{
		if (x > particao [j])
			i = j + 1;
	}

	*index = i;

	return (codebook [i]);
}
