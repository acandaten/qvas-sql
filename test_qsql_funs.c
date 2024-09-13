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

QList *sql_list;
QStr *current_sql;

static int run_command(char *cmd, char **left) {
  QStr *str;
  char *ch = cmd;
  while (*ch != '\0') {
    *ch = toupper(*ch);
    ch++;
  }
  if (strcmp(cmd, "GO") == 0 || strcmp(cmd, "G") == 0) {
    // PRINT
    while ((str = q_list_shift(sql_list)) != NULL) {
      printf("EXEC %s\n", str->data);
      qstr_free(str);
    }

  } else if (strcmp(cmd, "PRINT") == 0 || strcmp(cmd, "P") == 0) {
    for (int i = 0; i < q_list_size(sql_list); i++) {
      str = q_list_get(sql_list, i);
      printf("SQL:%s;\n\n", str->data);
    }
  } else {
    return 1;
  }
  return 0;
}

static void process_line(QStr *ln) {
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
        run_command(wrd, &pi);
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
  while ((str = file_gets(stdin))->length > 0) {
    bool a = str->data[str->length - 1] == '\n';
    qstr_trimr(str);
    process_line(str);
  }
  add_current_sql(current_sql, sql_list);
  // printf("\nCiao for now...\n");
}

int main(int argc, char const *argv[]) {
  sql_list = q_list_new(10);
  current_sql = qstr_new(1000, "");
  read_loop();
  return 0;
}
