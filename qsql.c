#include "qsql_funs.h"
#include <ctype.h>
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// GLOBAL VARIABLES

QList *sql_list;

int (*cmd_function)(char *, char **);
bool process_exiting = false;

PGconn *conn;
QSqlOpt sql_opt = {true, true, true, true, false};

int main_output = 0;

int usage() {
  printf("qsql [opts] [database] < sql\n");
  printf("   options:\n");
  printf("      -A : unaligned\n");
  printf("      -B : no borders\n");
  printf("      -H : no header\n");
  printf("      -e : break on error\n");
  printf("      -d : debug\n");
  return 1;
}

int run_sql(PGconn *conn, const char *query, QSqlOpt *opt) {
  int out = 0;
  // Submit the query and retrieve the result
  PGresult *res = PQexec(conn, query);

  // Check the status of the query result
  ExecStatusType resStatus = PQresultStatus(res);

  // Convert the status to a string and print it
  // fprintf(stdout, "Query Status: %s\n", PQresStatus(resStatus));

  // Check if the query execution was successful
  if (resStatus == PGRES_TUPLES_OK) {
    format_sql_result(res, opt, stdout);
  } else if (resStatus == PGRES_COMMAND_OK) {
    char *cmd_tuples = PQcmdTuples(res);
    if (strlen(cmd_tuples) == 0) {
      fprintf(stdout, "Command successful\n");

    } else {
      fprintf(stdout, "Command successful: Records affected: %s\n",
              PQcmdTuples(res));
    }
  } else {
    fprintf(stdout, "Error while executing the query: %s\n",
            PQerrorMessage(conn));
    out = 3;
  }

  PQclear(res);
  return out;
}

int run_command(char *cmd, char **left) {
  QStr *str;
  char *ch = cmd;
  while (*ch != '\0') {
    *ch = toupper(*ch);
    ch++;
  }
  if (strcmp(cmd, "GO") == 0 || strcmp(cmd, "G") == 0) {
    while ((str = q_list_shift(sql_list)) != NULL) {
      // printf("EXEC %s\n", str->data);
      if (run_sql(conn, str->data, &sql_opt) > 0) {
        main_output = 7;
        if (sql_opt.break_on_error) {
          process_exiting = true;
          break;
        }
      } // run_sql
    } // each sql

  } else if (strcmp(cmd, "PRINT") == 0 || strcmp(cmd, "P") == 0) {
    for (int i = 0; i < q_list_size(sql_list); i++) {
      str = q_list_get(sql_list, i);
      printf("SQL:%s;\n\n", str->data);
    } // each sql
  } else {
    return 1;
  }
  return 0;
}

PGconn *connect_db(const char *conninfo) {
  PGconn *conn = PQconnectdb(conninfo);

  if (PQstatus(conn) != CONNECTION_OK) {
    // If not successful, print the error message and finish the connection
    printf("Error while connecting to the database server: %s\n",
           PQerrorMessage(conn));

    // Finish the connection
    PQfinish(conn);
    return NULL;
  }
  return conn;
}

int main(int argc, char *argv[]) {
  int opt;

  if (argc <= 1) {
    return usage();
  }

  while ((opt = getopt(argc, argv, "AHBe")) != -1) {
    switch (opt) {
    case 'A':
      sql_opt.align = false;
      sql_opt.row_count = false;
      break;
    case 'H':
      sql_opt.header = false;
      break;
    case 'B':
      sql_opt.border = false;
      break;
    case 'e':
      sql_opt.break_on_error = true;
      break;
    case 'd':
      sql_opt.debug = true;
      break;
    default:
      fprintf(stderr, "Usage: %s [-i input_file] [-o output_file] [-v] [-g]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  // conninfo is a string of keywords and values separated by spaces.
  char conninfo[100];
  snprintf(conninfo, 100, "dbname=%s", argv[optind]);
  conn = connect_db(conninfo);
  if (conn == NULL)
    exit(1);

  // printf("Port: %s\n", PQport(conn));
  // printf("Host: %s\n", PQhost(conn));
  // printf("DBName: %s\n", PQdb(conn));
  //
  sql_list = q_list_new(10);
  cmd_function = run_command;
  read_loop();

  PQfinish(conn);

  return main_output;
}
