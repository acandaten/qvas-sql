// Test the QStr functionality
#include "qsql_funs.h"
#include "vepQList.h"
#include "vepQStr.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

QStr *file_gets(FILE *file) {
  char inp[100];
  QStr *s = qstr_new(100, "");

  while (fgets(inp, 100, file) != NULL) {
    qstr_cat(s, inp);
    if ((s->data)[s->length - 1] == '\n') {
      break;
    }
  }
  return s;
}

void remove_comments(QStr *ln) {
  if (ln->data[0] == '#')
    qstr_trunc(ln, 0);
  char *p = ln->data; // search index
  while (*p != '\0') {
    if (*p == '-' && *(p + 1) == '-') {
      *p = '\0';
      ln->length = (p - ln->data);
      break;
    }
    p++;
  }
}

char *read_word(char **ptr) {
  if (ptr == NULL || *ptr == NULL) {
    return NULL;
  }

  // If we've reached the end of the string, return NULL
  if (**ptr == '\0' || !isalpha((unsigned char)**ptr)) {
    return NULL;
  }

  // Find the end of the word
  const char *start = *ptr;
  while (**ptr != '\0' && isalpha((unsigned char)**ptr)) {
    (*ptr)++;
  }

  // Calculate the length of the word
  size_t len = *ptr - start;

  // Allocate memory for the word (plus null terminator)
  char *word = malloc(len + 1);
  if (word == NULL) {
    return NULL; // Memory allocation failed
  }

  // Copy the word and null-terminate it
  strncpy(word, start, len);
  word[len] = '\0';

  return word;
}

void add_current_sql(QStr *sql, QList *lst) {
  QStr *str = qstr_dup(sql);
  qstr_trunc(sql, 0);
  char *p = str->data;
  while (*p != '\0' && isspace(*p))
    p++;
  qstr_trunc(str, 0);
  qstr_cat(str, p);
  qstr_trimr(str);
  if (qstr_length(str) > 0)
    q_list_push(lst, str);
}
