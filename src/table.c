
#include "row.h"
#include "table.h"
#include <fcntl.h>   /* O_RDONLY. */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  /* Prototyes for some of the system functions. */
#include <stdio.h>

int table_get_row_size(Table *table) {
    int result = 0;
    for (int i = 0; i < table->num_columns; i++) {
        result += table->columns[i].size;
    }
    return result;
}

void table_create_columns(Table *table) {
    lseek(table->fd, 0, SEEK_SET);

    int row_size = table_get_row_size(table);
    char *row = (char*)alloca(row_size * sizeof(char));
    memset(row, 0, row_size);
    int *id = get_row_int_data(row, table->columns);
    char *table_name = get_row_char_data(row, table->columns + 1);
    char *column_name = get_row_char_data(row, table->columns + 2);
    char *data_type = get_row_char_data(row, table->columns + 3);
    int *position = get_row_int_data(row, table->columns + 4);
    int *size = get_row_int_data(row, table->columns + 5);
    int *next_value = get_row_int_data(row, table->columns + 6);

    int i;
    for (i = 0; i < table->num_columns; i++) {
        *id = i;
        strlcpy(table_name, table->name, table->columns[1].size);
        strlcpy(column_name, table->columns[i].name, table->columns[2].size);
        if (table->columns[i].type == C_CHAR) {
            strlcpy(data_type, "char", table->columns[3].size);
        } else {
            strlcpy(data_type, "int", table->columns[3].size);
        }
        *position = table->columns[i].position;
        *size = table->columns[i].size;
        *next_value = 0;
        write_row(table->fd, row, row_size);
    }

    *id = i++;
    strlcpy(table_name, "tables", table->columns[1].size);
    strlcpy(column_name, "id", table->columns[2].size);
    strlcpy(data_type, "int", table->columns[3].size);
    *position = 1;
    *size = 4;
    *next_value = 0;
    write_row(table->fd, row, row_size);

    *id = i++;
    strlcpy(table_name, "tables", table->columns[1].size);
    strlcpy(column_name, "table_name", table->columns[2].size);
    strlcpy(data_type, "char", table->columns[3].size);
    *position = 2;
    *size = 101;
    *next_value = 0;
    write_row(table->fd, row, row_size);
}

void table_create_tables(Table *table) {
    lseek(table->fd, 0, SEEK_SET);

    int row_size = table_get_row_size(table);
    char *row = (char*)alloca(row_size * sizeof(char));
    memset(row, 0, row_size);
    int *id = get_row_int_data(row, table->columns);
    char *table_name = get_row_char_data(row, table->columns + 1);

    *id = 0;
    strlcpy(table_name, table->name, table->columns[1].size);
    write_row(table->fd, row, row_size);

    *id = 1;
    strlcpy(table_name, "columns", 8);
    write_row(table->fd, row, row_size);
}

/* A database file contains a single table. */
void table_create(Table *table) {
    table->fd = open(table->name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (table->fd == -1) {
        perror("create table file");
        exit(EXIT_FAILURE);
    }

    // The columns table is special. All other tables will be created empty,
    // but the columns table, in effect, defines the structure of the database,
    // so an empty columns table needs some basic information about the tables
    // every database has, 'columns' and 'tables'.
    if (strcmp(table->name, "columns") == 0) {
        table_create_columns(table);
    } else if (strcmp(table->name, "tables") == 0) {
        table_create_tables(table);
    }
}

void table_open(Table* table) {
    table->fd = open(table->name, O_RDWR);
    if (table->fd == -1) {
        perror("open table");
        table_create(table);
    }
}

Table *table_new(const char* table_name) {
    Table *table = (Table*)malloc(sizeof(Table));
    int table_name_size = (strlen(table_name) + 1);
    table->name = (char*)calloc(table_name_size, sizeof(char));
    strlcpy(table->name, table_name, table_name_size);
    table->num_columns = 0;
    return table;
}

void table_free(Table *table) {
    close(table->fd);
    free(table->name);
    column_free_array(table->num_columns, table->columns);
    free(table);
}


void table_read_definition_tables(Table *table) {
    table->num_columns = 2;

    table->columns = (struct Column*)calloc(table->num_columns, sizeof(struct Column));

    int offset = 0;
    // Structure of the 'id' column.
    table->columns[0].name = (char*)calloc(3, sizeof(char));
    strcpy(table->columns[0].name, "id");
    table->columns[0].type = C_INT;
    table->columns[0].size = 4;
    table->columns[0].offset = offset;
    offset += table->columns[0].size;

    // Structure of the 'table_name' column.
    table->columns[1].name = (char*)calloc(11, sizeof(char));
    strcpy(table->columns[1].name, "table_name");
    table->columns[1].type = C_CHAR;
    table->columns[1].size = 101; // An extra byte to record the string null termination.
    table->columns[1].offset = offset;
    offset += table->columns[1].size;
}

void table_read_definition_columns(Table *table) {
    table->num_columns = 7;

    table->columns = (struct Column*)calloc(table->num_columns, sizeof(struct Column));

    int offset = 0;
    // Structure of the 'id' column.
    table->columns[0].name = (char*)calloc(3, sizeof(char));
    strcpy(table->columns[0].name, "id");
    table->columns[0].type = C_INT;
    table->columns[0].position = 1;
    table->columns[0].size = 4;
    table->columns[0].offset = offset;
    offset += table->columns[0].size;

    // Structure of the 'table_name' column.
    table->columns[1].name = (char*)calloc(11, sizeof(char));
    strcpy(table->columns[1].name, "table_name");
    table->columns[1].type = C_CHAR;
    table->columns[1].position = 2;
    table->columns[1].size = 101; // An extra byte to record the string null termination.
    table->columns[1].offset = offset;
    offset += table->columns[1].size;

    // Structure of the 'column_name' column.
    table->columns[2].name = (char*)calloc(12, sizeof(char));
    strcpy(table->columns[2].name, "column_name");
    table->columns[2].type = C_CHAR;
    table->columns[2].position = 3;
    table->columns[2].size = 101; // An extra byte to record the string null termination.
    table->columns[2].offset = offset;
    offset += table->columns[2].size;

    // Structure of the 'data_type' column.
    table->columns[3].name = (char*)calloc(10, sizeof(char));
    strcpy(table->columns[3].name, "data_type");
    table->columns[3].type = C_CHAR;
    table->columns[3].position = 4;
    table->columns[3].size = 5; // An extra byte to record the string null termination.
    table->columns[3].offset = offset;
    offset += table->columns[3].size;

    // Structure of the 'ordinal_position' column.
    table->columns[4].name = (char*)calloc(17, sizeof(char));
    strcpy(table->columns[4].name, "ordinal_position");
    table->columns[4].type = C_INT;
    table->columns[4].position = 5;
    table->columns[4].size = 4;
    table->columns[4].offset = offset;
    offset += table->columns[4].size;

    // Structure of the 'size' column.
    table->columns[5].name = (char*)calloc(5, sizeof(char));
    strcpy(table->columns[5].name, "size");
    table->columns[5].type = C_INT;
    table->columns[5].position = 6;
    table->columns[5].size = 4;
    table->columns[5].offset = offset;
    offset += table->columns[5].size;

    // Structure of the 'next_value' column.
    table->columns[6].name = (char*)calloc(11, sizeof(char));
    strcpy(table->columns[6].name, "next_value");
    table->columns[6].type = C_INT;
    table->columns[6].position = 7;
    table->columns[6].size = 4;
    table->columns[6].offset = offset;
    offset += table->columns[6].size;
}

void table_read_definition(Table *table, Table *columns) {
    // The two tables that contain the structure of all the other tables are
    // statically defined.
    if (strcmp(table->name, "tables") == 0) {
        table_read_definition_tables(table);
    } else if (strcmp(table->name, "columns") == 0) {
        table_read_definition_columns(table);
    } else {
        int row_size = table_get_row_size(table);
        char *row = (char*)calloc(row_size, sizeof(char));
        memset(row, 0, row_size);

        lseek(columns->fd, 0, SEEK_SET);

        while (read_row(columns->fd, row, row_size) > 0) {
            for (int i = 0; i < table->num_columns; i++) {
                switch (table->columns[i].type) {
                case C_CHAR:
                    printf("%-20s ", get_row_char_data(row, table->columns + i));
                    break;
                case C_INT:
                    printf("%-7d ", *get_row_int_data(row, table->columns + i));
                    break;
                default:
                    break;
                }
            }
            printf("\n");
        }
        free(row);
    }
}

