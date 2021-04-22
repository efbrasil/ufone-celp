#ifndef __PACK_H__
#define __PACK_H__

#include "celp.h"

void dopack (unsigned char **, int, int, int *);
void unpack (unsigned char **, int *, int, int *); 
void pckd2win (CELP_WIN *, unsigned char *);
void win2pckd (unsigned char *, CELP_WIN *);

#define CELP_PCKD_SIZE 22

#define FC_INDEX_SIZE 8
#define AC_INDEX_SIZE 10

#endif
