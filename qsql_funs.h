
#ifndef QSTR_FUNS_ /* Include guard */
#define QSTR_FUNS_

#include "vepQList.h"
#include "vepQStr.h"
#include <libpq-fe.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  bool header; /* print output field headings and row count */
  bool align;  /* fill align the fields */
  bool border; /* old brain dead format */
  bool row_count;
  bool break_on_error;
  bool debug;
} QSqlOpt;

extern QList *sql_list;
extern int (*cmd_function)(char *, char **);
extern bool process_exiting;

QStr *file_gets(FILE *file);

// Remove comments from QStr
void remove_comments(QStr *ln);

// Reads a word from a pointer (return needs to be freed when finished)
char *read_word(char **ptr);

void add_current_sql(QStr *sql, QList *lst);

// Format SQL Results
void format_sql_result(PGresult *res, QSqlOpt *opt, FILE *fd);

// read loop.
void read_loop();

#endif // QSTR_FUNS_
