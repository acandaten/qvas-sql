#include "vepQStr.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int usage() {
  printf("qsql [database] < sql");
  return 1;
}

int main(int argc, char *argv[]) {

  if (argc <= 1) {
    return usage();
  }
  // Connect to the database
  // conninfo is a string of keywords and values separated by spaces.
  char conninfo[100];
  snprintf(conninfo, 100, "dbname=%s", argv[1]);

  // Create a connection
  PGconn *conn = PQconnectdb(conninfo);

  // Check if the connection is successful
  if (PQstatus(conn) != CONNECTION_OK) {
    // If not successful, print the error message and finish the connection
    printf("Error while connecting to the database server: %s\n",
           PQerrorMessage(conn));

    // Finish the connection
    PQfinish(conn);

    // Exit the program
    exit(1);
  }

  // printf("Port: %s\n", PQport(conn));
  // printf("Host: %s\n", PQhost(conn));
  // printf("DBName: %s\n", PQdb(conn));

  // Execute a query
  char *query = "SELECT n_user, t_login_name, d_last_change, n_lock FROM "
                "vc_user WHERE n_user < 10 ORDER BY n_user";

  // Submit the query and retrieve the result
  PGresult *res = PQexec(conn, query);

  // Check the status of the query result
  ExecStatusType resStatus = PQresultStatus(res);

  // Convert the status to a string and print it
  printf("Query Status: %s\n", PQresStatus(resStatus));

  // Check if the query execution was successful
  if (resStatus != PGRES_TUPLES_OK) {
    // If not successful, print the error message and finish the connection
    printf("Error while executing the query: %s\n", PQerrorMessage(conn));

    // Clear the result
    PQclear(res);

    // Finish the connection
    PQfinish(conn);

    // Exit the program
    exit(1);
  }

  // Get the number of rows and columns in the query result
  int rows = PQntuples(res);
  int cols = PQnfields(res);
  //  printf("Number of columns: %d\n", cols);

  // // Set up PQprintOpt
  // PQprintOpt opt = {0};
  // opt.header = 1;            // Print column headers
  // opt.align = 1;             // Align fields
  // opt.standard = 0;          // Use standard output format
  // opt.html3 = 0;             // Don't use HTML format
  // opt.expanded = 0;          // Don't use expanded format
  // opt.pager = 0;             // Don't use pager
  // opt.fieldSep = "|";        // Field separator
  // opt.tableOpt = "border=2"; // No table options
  // opt.caption = NULL;        // No caption
  // opt.fieldName = NULL;      // Use default field names
  // Print the result
  // PQprint(stdout, res, &opt);

  int *col_lens = malloc(sizeof(int) * cols);
  int *col_types = malloc(sizeof(int) * cols);

  // Determine column lengths
  for (int i = 0; i < cols; i++) {
    col_lens[i] = strlen(PQfname(res, i));
    col_types[i] = PQftype(res, i);
  }

  int itmp;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      itmp = PQgetlength(res, i, j);
      if (itmp > col_lens[j])
        col_lens[j] = itmp;
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
  printf("%s\n", hbar->data);
  for (int i = 0; i < cols; i++) {
    printf("|%*s", col_lens[i], PQfname(res, i));
  }
  printf("|\n");
  printf("%s\n", hbar->data);

  // Print all the rows and columns
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      // Print the column value
      char *val = PQgetvalue(res, i, j);
      if (col_types[j] == 1114 && strncmp(val, "1970-01-01 00:00:00", 19) == 0)
        val = "";
      printf("|%*s", col_lens[j], val);
    }
    printf("|\n");
  }
  printf("%s\n", hbar->data);
  printf("(%d rows)\n", rows);

  // Clear the result
  PQclear(res);

  // Finish the connection
  PQfinish(conn);

  return 0;
}
