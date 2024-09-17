// Test the QStr functionality
#include "qsql_funs.h"
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
  qstr_trim(str);
  if (qstr_length(str) > 0)
    q_list_push(lst, str);
}

static void process_line(QStr *ln, QStr *current_sql) {
  remove_comments(ln);
  char *p = ln->data; // search index
  while (*p != '\0') {
    // Detect semicolon and add statement
    if (*p == ';') {
      *p = '\0';
      qstr_cat(current_sql, ln->data);
      qstr_trunc(ln, 0);
      add_current_sql(current_sql, sql_list);

      qstr_cat(ln, p + 1);
      p = ln->data;

      // Detect command (starts with back slash)
    } else if (*p == '\\' && isalpha(*(p + 1))) {
      *p = '\0';
      qstr_cat(current_sql, ln->data);
      qstr_trunc(ln, 0);
      add_current_sql(current_sql, sql_list);

      // Command
      char *pi = p + 1;
      char *wrd = read_word(&pi);
      if (wrd != NULL) {
        if (cmd_function != NULL) {
          cmd_function(wrd, &pi);
        } else {
          printf("Undefined function: cmd_function\n");
        }
        qstr_cat(ln, pi);
        p = ln->data;
        free(wrd);
      }

    } else { // Just a normal ch in line
      p++;
    }
  }
  qstr_trimr(ln);
  if (ln->length) {
    qstr_cat(current_sql, ln->data);
    qstr_cat(current_sql, "\n");
  }
}

void read_loop() {
  QStr *str;
  QStr *current_sql = qstr_new(1000, "");
  while ((str = file_gets(stdin))->length > 0) {
    bool a = str->data[str->length - 1] == '\n';
    qstr_trimr(str);
    process_line(str, current_sql);
  }
  add_current_sql(current_sql, sql_list);
}

void format_sql_result(PGresult *res, QSqlOpt *opt, FILE *fd) {
  // Get the number of rows and columns in the query result
  int rows = PQntuples(res);
  int cols = PQnfields(res);

  int *col_lens = malloc(sizeof(int) * cols);
  int *col_types = malloc(sizeof(int) * cols);

  // Determine column lengths
  for (int i = 0; i < cols; i++) {
    col_lens[i] = 0;
    if (opt->header == true && opt->align == true)
      col_lens[i] = strlen(PQfname(res, i));
    col_types[i] = PQftype(res, i);
  }

  if (opt->align == true) {
    int itmp;
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        itmp = PQgetlength(res, i, j);
        if (itmp > col_lens[j])
          col_lens[j] = itmp;
      }
    }
  }

  // create hbar
  QStr *hbar = qstr_new(100, "+");
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < col_lens[i]; j++)
      qstr_cat(hbar, "-");
    qstr_cat(hbar, "-");
  }

  // Print the column names
  if (opt->header == true) {
    if (opt->align)
      fprintf(fd, "%s\n", hbar->data);
    for (int i = 0; i < cols; i++) {
      fprintf(fd, "|%*s", col_lens[i], PQfname(res, i));
    }
    fprintf(fd, "|\n");
  }
  if (opt->align)
    fprintf(fd, "%s\n", hbar->data);

  // Print all the rows and columns
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      // Print the column value
      char *val = PQgetvalue(res, i, j);
      if (col_types[j] == 1114 && strncmp(val, "1970-01-01 00:00:00", 19) == 0)
        val = "";
      fprintf(fd, "|%*s", col_lens[j], val);
    }
    fprintf(fd, "|\n");
  }
  if (opt->align)
    fprintf(fd, "%s\n", hbar->data);
  if (opt->row_count)
    fprintf(fd, "(%d rows)\n", rows);
}
