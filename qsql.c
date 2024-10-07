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
QSqlOpt sql_opt = {true, true, true, true, false, false, true, .ingres_date = true, .delimit_char = "|"};

int main_output = 0;

// LOCAL FUNCTIONS

int usage() {
  printf("qsql [opts] [database] < sql\n");
  printf("   options:\n");
  printf("      -A : unaligned\n");
  printf("      -B : no borders\n");
  printf("      -H : no header\n");
  printf("      -D : no Ingres date\n");
  printf("      -e : break on error\n");
  printf("      -t : transaction (autocommit off)\n");
  printf("      -d : debug\n");
  return 1;
}

char *handle_copy_command(char *query) {
  char *file = NULL;
  regmatch_t *matches = NULL;

  if ((matches = check_regex(query, "^copy .+ (from|to) +'([^']+)'", 3)) != NULL) {

    // ensure that there is room to change query.
    int filename_len = matches[2].rm_eo - matches[2].rm_so;
    if (filename_len < 4) {
      free(matches);
      return NULL;
    }

    file = malloc(filename_len + 3);
    snprintf(file, 255, "%.*s", filename_len, query + matches[2].rm_so);
    query[matches[2].rm_so - 1] = '\0';

    if (query[matches[1].rm_so] == 't' || query[matches[1].rm_so] == 'T') {
      strcat(query, "stdout");

    } else { // from
      strcat(query, "stdin");
    }
    strcat(query, query + matches[2].rm_eo + 1);

    // printf("q: %s\n", query + matches[1].rm_so);
    // printf("file: %s\n", file);

    free(matches);
  }

  return file;
}

int run_sql(PGconn *conn, char *query, QSqlOpt *opt) {
  int out = 0;
  regmatch_t *match = NULL;
  char *filename;
  char *line;
  int bytes_read;

  if ((filename = handle_copy_command(query)) != NULL) {
    // printf("filename: %s\n", filename);
  }

  // printf("query: %s\n", query);

  // Submit the query and retrieve the result
  PGTransactionStatusType status = PQtransactionStatus(conn);
  if (status == PQTRANS_IDLE && opt->autocommit == false) {
    PQexec(conn, "START TRANSACTION");
    status = PQtransactionStatus(conn);
  }
  // printf("Before Transaction Status: %d : %s\n", status, query);

  if ((match = check_regex(query, "^commit", 1)) != NULL) {
    free(match);
    if (status == PQTRANS_IDLE || status == PQTRANS_INERROR) { // No Transaction
      query = NULL;
    }
  }

  if (query == NULL)
    return 0;

  PGresult *res = PQexec(conn, query);
  // status = PQtransactionStatus(conn);
  // printf("After Transaction Status: %d\n", status);

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
      // fprintf(stdout, "run sql : %s\n", query);
      fprintf(stdout, "Command successful.\n");

    } else {
      fprintf(stdout, "Command successful: Records affected: %s\n", PQcmdTuples(res));
    }

  } else if (resStatus == PGRES_COPY_IN) {
    char buffer[10000];
    FILE *fd = stdin;
    fd = fopen(filename, "r");

    if (fd == NULL) {
      fprintf(stderr, "Could not open file: %s\n", filename);
      exit(1);
    }
    // Read and send data
    while (fgets(buffer, sizeof(buffer), fd) != NULL) {
      if (PQputCopyData(conn, buffer, strlen(buffer)) != 1) {
        fprintf(stderr, "PQputCopyData failed: %s", PQerrorMessage(conn));
        fclose(fd);
        PQfinish(conn);
        exit(1);
      }
    }

    // Close the fd
    fclose(fd);

    // Signal the end of the COPY operation
    if (PQputCopyEnd(conn, NULL) != 1) {
      fprintf(stderr, "PQputCopyEnd failed: %s", PQerrorMessage(conn));
      PQfinish(conn);
      exit(1);
    }

    // Get the result
    res = PQgetResult(conn);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      fprintf(stderr, "COPY command failed: %s", PQerrorMessage(conn));
      PQclear(res);
      PQfinish(conn);
      exit(1);
    }
    printf("Copy from '%s' successful.\n", filename);

    // fprintf(stdout, "Error while executing the query : PGRES_COPY_IN \n");

  } else if (resStatus == PGRES_COPY_OUT) {
    FILE *fd = stdout;

    if (filename != NULL) {
      fd = fopen(filename, "a+");
    }

    while ((bytes_read = PQgetCopyData(conn, &line, 0)) > 0) {
      // printf("Read %d bytes: %s", bytes_read, line);
      fputs(line, fd);
      PQfreemem(line);
    }

    if (bytes_read == -2) {
      fprintf(stderr, "Error while COPY: %s", PQerrorMessage(conn));
      PQfinish(conn);
      exit(1);
    }
    if (filename != NULL) {
      fclose(fd);
      printf("Copy to '%s' successful.\n", filename);
    }

  } else {
    fprintf(stdout, "Error while executing the query: %s\n", PQerrorMessage(conn));
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
    printf("Error while connecting to the database server: %s\n", PQerrorMessage(conn));

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

  while ((opt = getopt(argc, argv, "AHBDetv:")) != -1) {
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
    case 'D':
      sql_opt.ingres_date = false;
      break;
    case 'e':
      sql_opt.break_on_error = true;
      break;
    case 't':
      sql_opt.autocommit = false;
      break;
    case 'd':
      sql_opt.debug = true;
      break;
    case 'v':
      if (strlen(optarg) > 0) {
        if (strcmp(optarg, "tab") == 0) {
          sprintf(sql_opt.delimit_char, "\t");

        } else {
          sql_opt.delimit_char[0] = optarg[0];
          sql_opt.delimit_char[1] = '\0';
        }
      }
      break;
    default:
      fprintf(stderr, "Usage: %s [-i input_file] [-o output_file] [-v] [-g]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  // conninfo is a string of keywords and values separated by spaces.
  char conninfo[100];
  snprintf(conninfo, 100, "dbname=%s", argv[optind]);
  conn = connect_db(conninfo);
  if (conn == NULL)
    exit(1);

  // DONE IN run_sql
  // if (sql_opt.autocommit == false) {
  //   run_sql(conn, "START TRANSACTION", &sql_opt);
  // }

  // printf("Port: %s\n", PQport(conn));
  // printf("Host: %s\n", PQhost(conn));
  // printf("DBName: %s\n", PQdb(conn));
  //
  sql_list = q_list_new(10);
  cmd_function = run_command;
  read_loop();

  PGTransactionStatusType status = PQtransactionStatus(conn);

  if (status == PQTRANS_INERROR || status == PQTRANS_INTRANS) {
    if (main_output != 0) {
      fprintf(stdout, "\nROLLBACK\n");
      run_sql(conn, "ROLLBACK", &sql_opt);

    } else {
      run_sql(conn, "COMMIT", &sql_opt);
    }
  }

  PQfinish(conn);

  return main_output;
}
