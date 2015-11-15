
#include <stdio.h>   /* C standard IO functions. */
#include <stdlib.h>  /* Prototypes of common lib functions.
                        exit(..)
                        Also, EXIT_FAILURE. */
#include <string.h>
#include "map.h"
#include "table.h"
#include "dataset.h"
#include "row.h"

DataSet *select_statement(Table *table) {
    DataSet *data_set = dataset_new();
    int row_size = table_get_row_size(table);

    lseek(table->fd, 0, SEEK_SET);

    for (int i = 0; i < table->num_columns; i++) {
        dataset_add_column(data_set, table->columns + i);
    }

    char *row = (char*)calloc(row_size, sizeof(char));
    while (read_row(table->fd, row, row_size) > 0) {
        dataset_add_row(data_set, row);
        row = (char*)calloc(row_size, sizeof(char));
    }
    free(row);
    return data_set;
}

int find_column(Table *table, const char *column_name) {
    for (int i = 0; i < table->num_columns; i++) {
        if (strcmp(table->columns[i].name, column_name) == 0) {
            return i;
        }
    }
    return -1;
}

void insert_statement(Table *table, DataSet *data_set) {
    lseek(table->fd, 0, SEEK_END);

    int row_size = table_get_row_size(table);
    char *row = (char*)alloca(row_size * sizeof(char));
    for (int r = 0; r < data_set->num_rows; r++) {
        memset(row, 0, row_size);
        for (int c = 0; c < data_set->num_columns; c++) {
            int i = find_column(table, data_set->columns[c]->name);
            if (i < 0) {
                printf("Unable to find the column '%s' in the table '%s'\n",
                       data_set->columns[c]->name, table->name);
                return;
            }
            if (data_set->columns[c]->type == C_INT) {
                int *intVal = get_row_int_data(row, table->columns + i);
                *intVal = *get_row_int_data(data_set->rows[r], data_set->columns[c]);
            } else if (data_set->columns[c]->type == C_CHAR) {
                char *charVal = get_row_char_data(row, table->columns + i);
                strlcpy(charVal, get_row_char_data(data_set->rows[r], data_set->columns[c]),
                        data_set->columns[c]->size);
            }
        }
        write_row(table->fd, row, row_size);
    }
}

void describe_statement(Table *table) {
    printf("   Table:'%s'\n", table->name);
    printf("-----------------------------------------------------------------\n");
    printf("name                                     type   size\n");
    printf("-----------------------------------------------------------------\n");
    for (int i = 0; i < table->num_columns; i++) {
        printf("%-40s %d      %d\n",
               table->columns[i].name, table->columns[i].type, table->columns[i].size);
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

    Table *columns = table_new("columns");
    table_read_definition(columns, columns);
    table_open(columns);
    map_put(open_tables, "columns", columns);

    Table *tables = table_new("tables");
    table_read_definition(tables, columns);
    table_open(tables);
    map_put(open_tables, "tables", tables);

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
        } else if (table->num_columns != (argc - 3)) {
            printf("The table has %d columns but %d data items were passed in.\n",
                   table->num_columns,
                   argc - 3);
        } else {
            DataSet *data_set = dataset_new();
            for (int i = 0; i < table->num_columns; i++) {
                dataset_add_column(data_set, table->columns + i);
            }

            int row_size = table_get_row_size(table);
            char *row = (char*)alloca(row_size * sizeof(char));
            memset(row, 0, row_size);
            for (int c = 0; c < data_set->num_columns; c++) {
                if (data_set->columns[c]->type == C_INT) {
                    int *intVal = get_row_int_data(row, table->columns + c);
                    *intVal = atoi(argv[c+3]);
                } else if (data_set->columns[c]->type == C_CHAR) {
                    char *charVal = get_row_char_data(row, table->columns + c);
                    strlcpy(charVal, argv[c+3], data_set->columns[c]->size);
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

