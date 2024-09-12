// Test the QStr functionality
#include "vepQList.h"
#include "vepQStr.h"
#include <stdbool.h>
#include <stdio.h>
#include <strings.h>

QList *sql_list;
QStr *current_sql;

static QStr *file_gets(FILE *file) {
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

static void remove_comments(QStr *ln) {
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

static void add_current_sql() {
  printf("-----------------\n%s\n--------------------\n", current_sql->data);
  qstr_trunc(current_sql, 0);
}

static void process_line(QStr *ln) {
  remove_comments(ln);
  char *p = ln->data; // search index
  while (*p != '\0') {
    if (*p == ';') {
      *p = '\0';
      qstr_cat(current_sql, ln->data);
      qstr_trunc(ln, 0);
      qstr_cat(ln, p + 1);
      p = ln->data;
      add_current_sql();

    } else
      p++;
  }
  // printf("D1> %s\n", ln->data);
  qstr_trimr(ln);
  if (ln->length) {
    qstr_cat(current_sql, ln->data);
    qstr_cat(current_sql, "\n");
  }

  // printf("\n-----------------\n%s\n--------------------\n", ln->data);
}

void read_loop() {
  QStr *str;
  while ((str = file_gets(stdin))->length > 0) {
    bool a = str->data[str->length - 1] == '\n';
    qstr_trimr(str);
    process_line(str);
  }
  add_current_sql();
  printf("\nCiao for now...\n");
}

int main(int argc, char const *argv[]) {
  sql_list = q_list_new(10);
  current_sql = qstr_new(1000, "");
  read_loop();
  return 0;
}
