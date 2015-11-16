
#include "column_list.h"
#include "dataset.h"
#include "map.h"
#include "row.h"
#include "table.h"
#include <stdio.h>   /* C standard IO functions. */
#include <stdlib.h>  /* Prototypes of common lib functions.  exit(..) Also, EXIT_FAILURE. */
#include <string.h>

void insert_statement(Table *table, DataSet *data_set) {
    lseek(table->fd, 0, SEEK_END);

    int row_size = table_get_row_size(table);
    char *row = (char*)alloca(row_size * sizeof(char));
    for (int r = 0; r < data_set->num_rows; r++) {
        memset(row, 0, row_size);
        for (int c = 0; c < column_list_size(data_set->columns); c++) {
            int i = column_list_find(table->columns, column_list_get(data_set->columns, c)->name);
            if (i < 0) {
                printf("Unable to find the column '%s' in the table '%s'\n",
                       column_list_get(data_set->columns, c)->name, table->name);
                return;
            }
            if (column_list_get(data_set->columns, c)->type == C_INT) {
                int *intVal = row_get_int(row, column_list_get(table->columns, i));
                *intVal = *row_get_int(data_set->rows[r],
                                            column_list_get(data_set->columns, c));
            } else if (column_list_get(data_set->columns, c)->type == C_CHAR) {
                char *charVal = row_get_char(row, column_list_get(table->columns, i));
                strlcpy(charVal, row_get_char(data_set->rows[r],
                                                   column_list_get(data_set->columns, c)),
                        column_list_get(data_set->columns, c)->size);
            }
        }
        row_write(table->fd, row, row_size);
    }
}

void describe_statement(Table *table) {
    printf("   Table:'%s'\n", table->name);
    printf("-----------------------------------------------------------------\n");
    printf("name                                     type   size\n");
    printf("-----------------------------------------------------------------\n");
    for (int i = 0; i < column_list_size(table->columns); i++) {
        printf("%-40s %d      %d\n",
               column_list_get(table->columns, i)->name,
               column_list_get(table->columns, i)->type,
               column_list_get(table->columns, i)->size);
    }
    printf("\n");
}

void print_help() {
    printf("db\n");
    printf("\n");
    printf("describe <table-name>\n");
    printf("    - describe the structure of the given table\n");
    printf("select <table-name>\n");
    printf("    - select all the rows of the given table\n");
    printf("insert <table-name> <value>+\n");
    printf("    - insert a new row into the given table\n");
    printf("      One value must be supplied for each column in the table\n");
    printf("      in the order the columns appear in the table.\n");
    printf("\n");
}

Map *open_all_tables() {
    Map *open_tables = map_new();

    // The 'columns' table is statically defined because it contains the
    // definitions of the other tables.
    Table *columns = table_new("columns");
    table_read_definition(columns, columns);
    table_open(columns);
    map_put(open_tables, "columns", columns);

    // The 'tables' table is statically defined because it contains the list of
    // the other tables.
    Table *tables = table_new("tables");
    table_read_definition(tables, columns);
    table_open(tables);
    map_put(open_tables, "tables", tables);

    // Now query the 'tables' table. Using that, it will lead to a list of
    // tables. Using the list of tables, the 'columns' table will be queried to
    // get the necessary table metadata to define the table structure.
    DataSet *data_set = select_statement(tables);

    int table_name_col_num = column_list_find(tables->columns, "table_name");

    for (int i = 0; i < data_set->num_rows; i++) {
        char *row = *(data_set->rows + i);
        Column *column = column_list_get(data_set->columns, table_name_col_num);
        char *table_name = row_get_char(row, column);
        if (strcmp("tables", table_name) != 0
                && strcmp("columns", table_name) != 0) {
            Table *table = table_new(table_name);
            table_read_definition(table, columns);
            table_open(table);
            map_put(open_tables, table_name, table);
        }
    }

    dataset_free(data_set);

    return open_tables;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_help();
        exit(EXIT_SUCCESS);
    }
    Map *open_tables = open_all_tables();

    if (strcmp(argv[1], "describe") == 0) {
        Table *table = map_get(open_tables, argv[2]);
        if (table == NULL) {
            printf("There is no table '%s'.", argv[2]);
        } else {
            describe_statement(table);
        }
    } else if (strcmp(argv[1], "select") == 0) {
        Table *table = map_get(open_tables, argv[2]);
        if (table == NULL) {
            printf("There is no table '%s'.", argv[2]);
        } else {
            DataSet *data_set = select_statement(table);
            dataset_print(data_set);
            dataset_free(data_set);
        }
    } else if (strcmp(argv[1], "insert") == 0) {
        Table *table = map_get(open_tables, argv[2]);
        if (table == NULL) {
            printf("There is no table '%s'.", argv[2]);
        } else if (column_list_size(table->columns) != (argc - 3)) {
            printf("The table has %d columns but %d data items were passed in.\n",
                   column_list_size(table->columns),
                   argc - 3);
        } else {
            DataSet *data_set = dataset_new();
            for (int i = 0; i < column_list_size(table->columns); i++) {
                column_list_add(data_set->columns, column_list_get(table->columns, i));
            }

            int row_size = table_get_row_size(table);
            char *row = (char*)alloca(row_size * sizeof(char));
            memset(row, 0, row_size);
            for (int c = 0; c < column_list_size(data_set->columns); c++) {
                if (column_list_get(data_set->columns, c)->type == C_INT) {
                    int *intVal = row_get_int(row, column_list_get(table->columns, c));
                    *intVal = atoi(argv[c+3]);
                } else if (column_list_get(data_set->columns, c)->type == C_CHAR) {
                    char *charVal = row_get_char(row, column_list_get(table->columns, c));
                    strlcpy(charVal, argv[c+3], column_list_get(data_set->columns, c)->size);
                }
            }
            dataset_add_row(data_set, row);

            insert_statement(table, data_set);
            dataset_free(data_set);
        }
    }

    // Close all the tables.
    char **keys = map_keys(open_tables);
    for (int i = 0; i < map_size(open_tables); i++) {
        table_free(map_get(open_tables, keys[i]));
    }
    map_free(open_tables);
}

