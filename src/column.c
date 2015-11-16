
#include "column.h"
#include <stdlib.h>
#include <string.h>

Column *column_new_w_name(const char *name) {
    Column *column = malloc(sizeof(Column));
    int len = strlen(name) + 1;
    column->name = malloc(len * sizeof(char));
    strlcpy(column->name, name, len);
    return column;
}

void column_free(Column *column) {
    free(column->name);
    free(column);
}

void column_free_array(int num_columns, Column *columns) {
    for (int i = 0; i < num_columns; i++) {
        free(columns[i].name);
    }
    free(columns);
}

