#ifndef __FILTER_H__
#define __FILTER_H__

unsigned int hamming (double *, int);
unsigned int apply_window (double *, double *, int);

void cheby1 (double *, int);
void cacm283(double a[], int , double r[]);
void polimulti (double poli1[], double poli2[], double polir[],
		int ordem1, int ordem2);
void filt_pz (double s_input[], double s_output[], double num_filter[],
		double den_filter[], double buffer_e[], double buffer_s[],
		int signal_size, int buffer_size);
void filt_sp1(double s_input[], double s_output[], double den_filter[], 
	      double buffer[], int signal_size, int buffer_size);
void filt_sp2 (double s_input[], double s_output[], double den_filter[], 
		int signal_size, int buffer_size);
/* Need to be fixed, recoded, replaced or moved somewhere else */
double inner_prod (double vector1[], double vector2[], int size);

unsigned int load_from_file(const char file[], double variable[],
		unsigned int num_itens);
double quant_esc (double, double [], double [], int, int *);

#endif
