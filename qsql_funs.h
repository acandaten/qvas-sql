
#ifndef QSTR_FUNS_ /* Include guard */
#define QSTR_FUNS_

#include "vepQList.h"
#include "vepQStr.h"
#include <stdio.h>

QStr *file_gets(FILE *file);

// Remove comments from QStr
void remove_comments(QStr *ln);

// Reads a word from a pointer (return needs to be freed when finished)
char *read_word(char **ptr);

void add_current_sql(QStr *sql, QList *lst);

#endif // QSTR_FUNS_
