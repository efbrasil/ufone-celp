#ifndef _UTILS_H_
#define _UTILS_H_

struct celp_options
{
	int verbose;		/* Verbosity level                    */
	int op;			/* Operation: 0 = code    1 = descode */
	int mode;		/* File mode: 0 = Wave    2 = Raw     */
	char *ifilename;
	char *ofilename;
	FILE *ifd;
	FILE *ofd;
};

unsigned int parse_args (struct celp_options *, int, char **);
void show_banner (void);
void show_usage (char **argv);
unsigned int open_files (struct celp_options *);
unsigned int close_files (struct celp_options *);

#endif
