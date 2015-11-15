
#include "dataset.h"
#include "row.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DataSet *dataset_new() {
    DataSet *data_set = (DataSet*)malloc(sizeof(DataSet));
    data_set->num_columns = 0;
    data_set->columns = NULL;
    data_set->num_rows = 0;
    data_set->rows = NULL;
    return data_set;
}

int max(int i1, int i2) {
    if (i1 < i2) return i2;
    return i1;
}

int min(int i1, int i2) {
    if (i1 < i2) return i1;
    return i2;
}

void dataset_free(DataSet *data_set) {
    free(data_set->columns);
    free(data_set->rows);
    free(data_set);
}

void dataset_add_row(DataSet *data_set, char *row) {
    char **old = data_set->rows;
    data_set->rows = (char**)calloc(data_set->num_rows + 1, sizeof(char*));
    int i;
    for (i = 0; i < data_set->num_rows; i++) {
        data_set->rows[i] = old[i];
    }
    data_set->rows[i] = row;
    data_set->num_rows++;
    free(old);
}

void dataset_add_column(DataSet *data_set, Column *column) {
    Column **old = data_set->columns;
    data_set->columns = (Column **)calloc(data_set->num_columns + 1, sizeof(Column *));
    int i;
    for (i = 0; i < data_set->num_columns; i++) {
        data_set->columns[i] = old[i];
    }
    data_set->columns[i] = column;
    data_set->num_columns++;
    free(old);
}

void dataset_print(DataSet *data_set) {
    int total_width = 0;
    int col_print_width[data_set->num_columns];
    char *format = (char*)alloca(20 * sizeof(char));
    for (int i = 0; i < data_set->num_columns; i++) {
        Column *column = *(data_set->columns + i);
        switch (column->type) {
        case C_CHAR:
            col_print_width[i] = min(20, max(strlen(column->name), column->size)) + 1;
            break;
        case C_INT:
            col_print_width[i] = min(20, max(strlen(column->name), 7)) + 1;
            break;
        default:
            break;
        }
        snprintf(format, 20, "%%-%ds", col_print_width[i]);
        printf(format, column->name);
        total_width += col_print_width[i];
    }

    printf("\n");
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    for (int i = 0; i < data_set->num_rows; i++) {
        char *row = *(data_set->rows + i);
        for (int j = 0; j < data_set->num_columns; j++) {
            Column *column = *(data_set->columns + j);
            switch (column->type) {
            case C_CHAR:
                snprintf(format, 20, "%%-%ds", col_print_width[j]);
                printf(format, get_row_char_data(row, column));
                break;
            case C_INT:
                snprintf(format, 20, "%%-%dd", col_print_width[j]);
                printf(format, *get_row_int_data(row, column));
                break;
            default:
                break;
            }
        }
        printf("\n");
    }
}

