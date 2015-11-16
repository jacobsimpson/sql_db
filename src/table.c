
#include "map.h"
#include "row.h"
#include "table.h"
#include "column_list.h"
#include <fcntl.h>   /* O_RDONLY. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  /* Prototyes for some of the system functions. */

Table *table_new(const char* table_name) {
    Table *table = (Table*)malloc(sizeof(Table));
    int table_name_size = (strlen(table_name) + 1);
    table->name = (char*)calloc(table_name_size, sizeof(char));
    strlcpy(table->name, table_name, table_name_size);
    table->columns = column_list_new();
    return table;
}

void table_free(Table *table) {
    close(table->fd);
    free(table->name);
    column_list_free(table->columns);
    free(table);
}

int table_get_row_size(Table *table) {
    int result = 0;
    for (int i = 0; i < column_list_size(table->columns); i++) {
        result += column_list_get(table->columns, i)->size;
    }
    return result;
}

static void init_columns_table(Table *table) {
    lseek(table->fd, 0, SEEK_SET);

    int row_size = table_get_row_size(table);
    char *row = (char*)alloca(row_size * sizeof(char));
    memset(row, 0, row_size);
    int *id = row_get_int(row, column_list_get(table->columns, 0));
    char *table_name = row_get_char(row, column_list_get(table->columns, 1));
    char *column_name = row_get_char(row, column_list_get(table->columns, 2));
    char *data_type = row_get_char(row, column_list_get(table->columns, 3));
    int *position = row_get_int(row, column_list_get(table->columns, 4));
    int *size = row_get_int(row, column_list_get(table->columns, 5));
    int *next_value = row_get_int(row, column_list_get(table->columns, 6));

    int i;
    for (i = 0; i < column_list_size(table->columns); i++) {
        *id = i;
        strlcpy(table_name, table->name, column_list_get(table->columns, 1)->size);
        strlcpy(column_name, column_list_get(table->columns, i)->name,
                column_list_get(table->columns, 2)->size);
        if (column_list_get(table->columns, i)->type == C_CHAR) {
            strlcpy(data_type, "char", column_list_get(table->columns, 3)->size);
        } else {
            strlcpy(data_type, "int", column_list_get(table->columns, 3)->size);
        }
        *position = column_list_get(table->columns, i)->position;
        *size = column_list_get(table->columns, i)->size;
        *next_value = 0;
        row_write(table->fd, row, row_size);
    }

    *id = i++;
    strlcpy(table_name, "tables", column_list_get(table->columns, 1)->size);
    strlcpy(column_name, "id", column_list_get(table->columns, 2)->size);
    strlcpy(data_type, "int", column_list_get(table->columns, 3)->size);
    *position = 1;
    *size = 4;
    *next_value = 0;
    row_write(table->fd, row, row_size);

    *id = i++;
    strlcpy(table_name, "tables", column_list_get(table->columns, 1)->size);
    strlcpy(column_name, "table_name", column_list_get(table->columns, 2)->size);
    strlcpy(data_type, "char", column_list_get(table->columns, 3)->size);
    *position = 2;
    *size = 101;
    *next_value = 0;
    row_write(table->fd, row, row_size);
}

static void init_tables_table(Table *table) {
    lseek(table->fd, 0, SEEK_SET);

    int row_size = table_get_row_size(table);
    char *row = (char*)alloca(row_size * sizeof(char));
    memset(row, 0, row_size);
    int *id = row_get_int(row, column_list_get(table->columns, 0));
    char *table_name = row_get_char(row, column_list_get(table->columns, 1));

    *id = 0;
    strlcpy(table_name, table->name, column_list_get(table->columns, 1)->size);
    row_write(table->fd, row, row_size);

    *id = 1;
    strlcpy(table_name, "columns", 8);
    row_write(table->fd, row, row_size);
}

/* A database file contains a single table. */
void table_create(Table *table) {
    table->fd = open(table->name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (table->fd == -1) {
        perror("create table file");
        exit(EXIT_FAILURE);
    }

    // The 'columns' and the 'tables' tables are special. All other tables will
    // be created empty, but the 'columns' table, in effect, defines the
    // structure of the database, so an empty columns table needs some basic
    // information about the tables every database has, 'columns' and 'tables'.
    if (strcmp(table->name, "columns") == 0) {
        init_columns_table(table);
    } else if (strcmp(table->name, "tables") == 0) {
        init_tables_table(table);
    }
}

void table_open(Table* table) {
    table->fd = open(table->name, O_RDWR);
    if (table->fd == -1) {
        perror("open table");
        table_create(table);
    }
}

DataSet *table_scan(Table *table) {
    DataSet *data_set = dataset_new();
    int row_size = table_get_row_size(table);

    lseek(table->fd, 0, SEEK_SET);

    for (int i = 0; i < column_list_size(table->columns); i++) {
        column_list_add(data_set->columns, column_list_get(table->columns, i));
    }

    char *row = (char*)calloc(row_size, sizeof(char));
    while (row_read(table->fd, row, row_size) > 0) {
        dataset_add_row(data_set, row);
        row = (char*)calloc(row_size, sizeof(char));
    }
    free(row);
    return data_set;
}

static void table_read_definition_tables(Table *table) {
    Column *column;
    int offset = 0;

    // Structure of the 'id' column.
    column = column_new_w_name("id");
    column->type = C_INT;
    column->size = 4;
    column->offset = offset;
    offset += column->size;
    column_list_add(table->columns, column);

    // Structure of the 'table_name' column.
    column = column_new_w_name("table_name");
    column->type = C_CHAR;
    column->size = 101; // An extra byte to record the string null termination.
    column->offset = offset;
    offset += column->size;
    column_list_add(table->columns, column);
}

static void table_read_definition_columns(Table *table) {
    Column *column;
    int offset = 0;

    // Structure of the 'id' column.
    column = column_new_w_name("id");
    column->type = C_INT;
    column->position = 1;
    column->size = 4;
    column->offset = offset;
    offset += column->size;
    column_list_add(table->columns, column);

    // Structure of the 'table_name' column.
    column = column_new_w_name("table_name");
    column->type = C_CHAR;
    column->position = 2;
    column->size = 101; // An extra byte to record the string null termination.
    column->offset = offset;
    offset += column->size;
    column_list_add(table->columns, column);

    // Structure of the 'column_name' column.
    column = column_new_w_name("column_name");
    column->type = C_CHAR;
    column->position = 3;
    column->size = 101; // An extra byte to record the string null termination.
    column->offset = offset;
    offset += column->size;
    column_list_add(table->columns, column);

    // Structure of the 'data_type' column.
    column = column_new_w_name("data_type");
    column->type = C_CHAR;
    column->position = 4;
    column->size = 5; // An extra byte to record the string null termination.
    column->offset = offset;
    offset += column->size;
    column_list_add(table->columns, column);

    // Structure of the 'ordinal_position' column.
    column = column_new_w_name("ordinal_position");
    column->type = C_INT;
    column->position = 5;
    column->size = 4;
    column->offset = offset;
    offset += column->size;
    column_list_add(table->columns, column);

    // Structure of the 'size' column.
    column = column_new_w_name("size");
    column->type = C_INT;
    column->position = 6;
    column->size = 4;
    column->offset = offset;
    offset += column->size;
    column_list_add(table->columns, column);

    // Structure of the 'next_value' column.
    column = column_new_w_name("next_value");
    column->type = C_INT;
    column->position = 7;
    column->size = 4;
    column->offset = offset;
    offset += column->size;
    column_list_add(table->columns, column);
}

void table_read_definition(Table *table, Table *columns) {
    // The two tables that contain the structure of all the other tables are
    // statically defined.
    if (strcmp(table->name, "tables") == 0) {
        table_read_definition_tables(table);
    } else if (strcmp(table->name, "columns") == 0) {
        table_read_definition_columns(table);
    } else {
        DataSet *data_set = table_scan(columns);

        Column *table_name_column = column_list_get(data_set->columns,
                                    column_list_find(columns->columns, "table_name"));
        Column *column_name_column = column_list_get(data_set->columns,
                                     column_list_find(columns->columns, "column_name"));
        Column *data_type_column = column_list_get(data_set->columns,
                                   column_list_find(columns->columns, "data_type"));
        Column *position_column = column_list_get(data_set->columns,
                                  column_list_find(columns->columns, "ordinal_position"));
        Column *size_column = column_list_get(data_set->columns,
                                              column_list_find(columns->columns, "size"));

        for (int i = 0; i < data_set->num_rows; i++) {
            char *row = *(data_set->rows + i);
            char *table_name = row_get_char(row, table_name_column);
            if (strcmp(table->name, table_name) == 0) {
                char *column_name = row_get_char(row, column_name_column);
                Column *column = column_new_w_name(column_name);
                char *data_type = row_get_char(row, data_type_column);
                if (strcmp(data_type, "int") == 0) {
                    column->type = C_INT;
                } else {
                    column->type = C_CHAR;
                }
                column->position = *row_get_int(row, position_column);
                column->size = *row_get_int(row, size_column);
                column_list_add(table->columns, column);
            }
        }

        column_list_sort_position(table->columns);
        int offset = 0;
        for (int i = 0; i < column_list_size(table->columns); i++) {
            Column *column = column_list_get(table->columns, i);
            column->offset = offset;
            offset += column->size;
        }

        dataset_free(data_set);
    }
}

