#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "catalog.h"
#include <stream.h>

#define ERRSIZE 1024

typedef struct context {
	int cur;

	int yyval;
	char *yytext;
	int yylen;
	int yysize;
	int debug;
	
	char *sql;
	int sqllen;
	int sqlsize;

	int lineno;
	char *filename;
	char *buf;
	stream *in;
	stream *out;

	char errstr[ERRSIZE];
	struct catalog *cat;

	struct dlist *l;
} context;

#endif /* _CONTEXT_H_ */
