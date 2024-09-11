#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

int main(int argc, char *argv[]) {

    // Connect to the database
    // conninfo is a string of keywords and values separated by spaces.
    char *conninfo = "dbname=laveddb1";

    // Create a connection
    PGconn *conn = PQconnectdb(conninfo);

    // Check if the connection is successful
    if (PQstatus(conn) != CONNECTION_OK) {
        // If not successful, print the error message and finish the connection
        printf("Error while connecting to the database server: %s\n", PQerrorMessage(conn));

        // Finish the connection
        PQfinish(conn);

        // Exit the program
        exit(1);
    }

    // We have successfully established a connection to the database server
    printf("Port: %s\n", PQport(conn));
    printf("Host: %s\n", PQhost(conn));
    printf("DBName: %s\n", PQdb(conn));

    // Execute a query
    char *query = "SELECT n_user, t_login_name, d_last_change, n_lock FROM vc_user WHERE n_user < 10 ORDER BY n_user";

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

    // We have successfully executed the query
    printf("Query Executed Successfully\n");

    // Get the number of rows and columns in the query result
    int rows = PQntuples(res);
    int cols = PQnfields(res);
    printf("Number of rows: %d\n", rows);
    printf("Number of columns: %d\n", cols);

      // Set up PQprintOpt
    PQprintOpt opt = {0};
    opt.header    = 1;      // Print column headers
    opt.align     = 1;      // Align fields
    opt.standard  = 0;      // Use standard output format
    opt.html3     = 0;      // Don't use HTML format
    opt.expanded  = 0;      // Don't use expanded format
    opt.pager     = 0;      // Don't use pager
    opt.fieldSep  = "|";    // Field separator
    opt.tableOpt  = NULL;   // No table options
    opt.caption   = NULL;   // No caption
    opt.fieldName = NULL;   // Use default field names

    // Print the result
    PQprint(stdout, res, &opt);

    // // Print the column names
    // for (int i = 0; i < cols; i++) {
    //     printf("%s\t", PQfname(res, i));
    // }
    // printf("\n");
    //
    // // Print all the rows and columns
    // for (int i = 0; i < rows; i++) {
    //     for (int j = 0; j < cols; j++) {
    //         // Print the column value
    //         printf("%s\t", PQgetvalue(res, i, j));
    //     }
    //     printf("\n");
    // }

    // Clear the result
    PQclear(res);

    // Finish the connection
    PQfinish(conn);

    return 0;
}
