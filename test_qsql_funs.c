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
#include <unistd.h>

QList *sql_list;

int (*cmd_function)(char *, char **);

bool process_exiting = false;

QSqlOpt sql_opt = {true, true, true, true, false, false, true};

int main_output = 0;

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

void test_read_loop() {
  sql_list = q_list_new(11);
  cmd_function = run_command;
  read_loop();
}

void test_convert_sql_val() {
  char dt[100];
  strcpy(dt, "1970-01-01 00:00:00");
  printf("%s(%d) = %s\n", dt, 1000, convert_sql_val(1114, dt, false));
  printf("%s(%d) = %s\n", dt, 1114, convert_sql_val(1114, dt, true));
  strcpy(dt, "2024-03-14 00:00:00");
  printf("%s(%d) = %s\n", dt, 1000, convert_sql_val(1114, dt, false));
  printf("%s(%d) = %s\n", dt, 1114, convert_sql_val(1114, dt, true));
  strcpy(dt, "2024-08-28 09:12:40");
  printf("%s(%d) = %s\n", dt, 1000, convert_sql_val(1114, dt, false));
  printf("%s(%d) = %s\n", dt, 1114, convert_sql_val(1114, dt, true));
}

int main(int argc, char *argv[]) {
  int opt;

  if (argc <= 1) {
    puts("Usage: test_qsql_funs  [opts]\n  where -r : read_loop; -d : date test");
    return 9;
  }

  while ((opt = getopt(argc, argv, "rd")) != -1) {
    switch (opt) {
    case 'r':
      test_read_loop();
      break;
    case 'd':
      test_convert_sql_val();
      break;
    default:
      fprintf(stderr, "Usage: %s [-r] [-d]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  return EXIT_SUCCESS;
}
