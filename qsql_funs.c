// Test the QStr functionality
#include "qsql_funs.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <vepQStr.h>
#define __USE_XOPEN
#include <time.h>

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

// Reads an alpha word (no numbers or speecial chars) if at start , else null.
// If the word is returned, it is malloc copy and needs to be freed at some
// stage.
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
  if (process_exiting)
    return;
  QStr *str = qstr_dup(sql);
  qstr_trunc(sql, 0);
  qstr_trim(str);
  if (qstr_length(str) > 0)
    q_list_push(lst, str);
}

static bool isCmdChar(char ch) { return (ch == 'p' || ch == 'P' || ch == 'g' || ch == 'G'); }

// Parse and add line to current. Return non-zero if error and wish to stop
// processing
static int process_line(QStr *ln, QStr *current_sql) {
  int error = 0;
  remove_comments(ln);
  char *p = ln->data; // search index
  while (*p != '\0' && !process_exiting) {
    // Detect semicolon and add statement
    if (*p == ';') {
      *p = '\0';
      qstr_cat(current_sql, ln->data);
      qstr_trunc(ln, 0);
      add_current_sql(current_sql, sql_list);

      qstr_cat(ln, p + 1);
      p = ln->data;

      // Detect command (starts with back slash)
      // } else if (*p == '\\' && isalpha(*(p + 1))) {
    } else if (*p == '\\' && isCmdChar(*(p + 1)) == true) {
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
          process_exiting = true;
          error = 5;
        }
        qstr_cat(ln, pi);
        p = ln->data;
        free(wrd);
      }

    } else { // Just a normal ch in line
      p++;
    }
  } // while not at end

  qstr_trimr(ln);
  if (ln->length) {
    qstr_cat(current_sql, ln->data);
    qstr_cat(current_sql, "\n");
  }
  return error;
}

void read_loop() {
  QStr *str;
  QStr *current_sql = qstr_new(1000, "");
  while ((str = file_gets(stdin))->length > 0 && !process_exiting) {
    bool a = str->data[str->length - 1] == '\n';
    qstr_trimr(str);
    process_line(str, current_sql);
  }
  add_current_sql(current_sql, sql_list);
}

static bool isEpochTime(struct tm *tm) {
  if (tm->tm_hour == 0 && tm->tm_min == 0 && tm->tm_sec == 0) {
    return true;
  }
  return false;
}

static bool isEpoch(struct tm *tm) {
  if (tm->tm_hour == 0 && tm->tm_min == 0 && tm->tm_sec == 0) {
    if (tm->tm_year == 70 && tm->tm_mon == 0 && tm->tm_mday == 1)
      return true;
  }
  return false;
}

static bool parseISOTimestamp(const char *str, struct tm *stm) {
  char *ret;
  if (str[4] == '-' && str[7] == '-' && str[13] == ':') {
    ret = strptime(str, "%Y-%m-%d %H:%M:%S", stm);
    if (ret == NULL)
      return false;
    return true;
  }
  return false;
}

char *convert_sql_val(int coltype, char *val, bool ingres_date_flag) {
  struct tm timeval;
  static char out[30];
  if (coltype != 1114) {
    return val;
  }
  if (ingres_date_flag == false) {
    return val;
  }
  if (parseISOTimestamp(val, &timeval) == false)
    return val;
  if (isEpoch(&timeval)) {
    strcpy(out, "");
  } else if (isEpochTime(&timeval)) {
    strftime(out, sizeof(out), "%d-%b-%Y", &timeval);
  } else {
    strftime(out, sizeof(out), "%d-%b-%Y %H:%M:%S", &timeval);
  }
  return out;
}

void format_sql_result(PGresult *res, QSqlOpt *opt, FILE *fd) {
  // Get the number of rows and columns in the query result
  QStr *hbar = qstr_new(100, "");
  int rows = PQntuples(res);
  int cols = PQnfields(res);
  char end_line[10] = "|\n";

  snprintf(end_line, 3, "%s\n", opt->delimit_char);
  if (opt->border == false)
    strcpy(end_line, "\n");

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
        if (col_types[j] == 1114) {
          itmp = strlen(convert_sql_val(1114, PQgetvalue(res, i, j), opt->ingres_date));
        }
        if (itmp > col_lens[j])
          col_lens[j] = itmp;
      }
    }

    // create hbar
    qstr_cat(hbar, "+");
    for (int i = 0; i < cols; i++) {
      for (int j = 0; j < col_lens[i]; j++)
        qstr_cat(hbar, "-");
      qstr_cat(hbar, "-");
    }
    if (opt->border == false)
      qstr_trunc(hbar, hbar->length - 2);
    qstr_cat(hbar, "\n");

    // negate VARCHAR lengths for right-padd
    for (int j = 0; j < cols; j++) {
      if (col_types[j] == 18 || col_types[j] == 25 || col_types[j] == 1043) {
        col_lens[j] = -1 * col_lens[j];
      }
    }
  }

  // adjust col_len based on type

  // Print the column names
  if (opt->header == true) {
    fprintf(fd, "%s", hbar->data);

    for (int i = 0; i < cols; i++) {
      if (i > 0 || opt->border == true)
        fprintf(fd, "%s%*s", opt->delimit_char, col_lens[i], PQfname(res, i));
      else
        fprintf(fd, "%*s", col_lens[i], PQfname(res, i));
    }
    fputs(end_line, fd);
  }
  fprintf(fd, "%s", hbar->data);

  // Print all the rows and columns
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      // Print the column value
      char *val = convert_sql_val(col_types[j], PQgetvalue(res, i, j), opt->ingres_date);
      // if (col_types[j] == 1114 && strncmp(val, "1970-01-01 00:00:00", 19) == 0)
      //   val = "";
      if (j > 0 || opt->border == true)
        fprintf(fd, "%s%*s", opt->delimit_char, col_lens[j], val);
      else
        fprintf(fd, "%*s", col_lens[j], val);
    }
    fputs(end_line, fd);
  }
  if (opt->align)
    fprintf(fd, "%s", hbar->data);
  if (opt->row_count)
    fprintf(fd, "(%d rows)\n", rows);

  free(col_lens);
  free(col_types);
  qstr_free(hbar);
}
