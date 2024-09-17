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

int (*cmd_function)(char *, char **);

int run_command(char *cmd, char **left) {
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

int main(int argc, char const *argv[]) {
  sql_list = q_list_new(11);
  cmd_function = run_command;
  read_loop();
  return 0;
}
