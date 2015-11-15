
#include "column.h"
#include <stdlib.h>

#define C_CHAR 1
#define C_INT 2

void column_free_array(int num_columns, Column *columns) {
    for (int i = 0; i < num_columns; i++) {
        free(columns[i].name);
    }
    free(columns);
}

