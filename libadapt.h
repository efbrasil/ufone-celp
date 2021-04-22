#ifndef __LIBADAPT_H__
#define __LIBADAPT_H__

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

struct _BNDR_CTX
{
	unsigned int size;
	double mu;
	gsl_vector *x1;
	gsl_vector *x2;
	gsl_vector *coef;
	gsl_vector *delta;
	double d2;
};

typedef struct _BNDR_CTX BNDR_CTX;

BNDR_CTX *bndr_init (unsigned int, double);
void bndr_free (BNDR_CTX *);
double bndr_iter (double, double, BNDR_CTX *);

struct _LMS_CTX
{
	unsigned int size;
	double mu;
	gsl_matrix *state;
	gsl_matrix *coef;
	gsl_matrix *result;
	gsl_matrix *delta;
};

typedef struct _LMS_CTX LMS_CTX;

LMS_CTX *lms_init (unsigned int, double);
void lms_free (LMS_CTX *);
double lms_iter (double, double, LMS_CTX *);

struct _NLMS_CTX
{
	unsigned int size;
	double alpha;
	double gamma;
	gsl_matrix *state;
	gsl_matrix *coef;
	gsl_matrix *result;
	gsl_matrix *delta;
};

typedef struct _NLMS_CTX NLMS_CTX;

NLMS_CTX *nlms_init (unsigned int, double, double);
void nlms_free (NLMS_CTX *);
double nlms_iter (double, double, NLMS_CTX *);

struct _RLS_CTX
{
	unsigned int size;
	gsl_vector *state;
	gsl_vector *coef;
	gsl_vector *delta;
	gsl_vector *info;
	gsl_vector *ninfo;
	gsl_matrix *icorr;

	gsl_matrix *tmp1, *tmp2;
};

typedef struct _RLS_CTX RLS_CTX;

RLS_CTX *rls_init (unsigned int);
void rls_free (RLS_CTX *);
double rls_iter (double, double, RLS_CTX *);

struct _SDLMS_CTX
{
	unsigned int size;
	double mu;
	gsl_matrix *state;
	gsl_matrix *coef;
	gsl_matrix *result;
	gsl_matrix *delta;
};

typedef struct _SDLMS_CTX SDLMS_CTX;

SDLMS_CTX *sdlms_init (unsigned int, double);
void sdlms_free (SDLMS_CTX *);
double sdlms_iter (double, double, SDLMS_CTX *);

struct _SELMS_CTX
{
	unsigned int size;
	double mu;
	gsl_matrix *state;
	gsl_matrix *coef;
	gsl_matrix *result;
	gsl_matrix *delta;
};

typedef struct _SELMS_CTX SELMS_CTX;

SELMS_CTX *selms_init (unsigned int, double);
void selms_free (SELMS_CTX *);
double selms_iter (double, double, SELMS_CTX *);

struct _SSLMS_CTX
{
	unsigned int size;
	double mu;
	gsl_matrix *state;
	gsl_matrix *coef;
	gsl_matrix *result;
	gsl_matrix *delta;
};

typedef struct _SSLMS_CTX SSLMS_CTX;

SSLMS_CTX *sslms_init (unsigned int, double);
void sslms_free (SSLMS_CTX *);
double sslms_iter (double, double, SSLMS_CTX *);

#endif
