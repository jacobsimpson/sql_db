
#include "row.h"

void row_write(int table_handle, char *row, int row_size) {
    write(table_handle, row, row_size);
}

ssize_t row_read(int table_handle, char *row, int row_size) {
    return read(table_handle, row, row_size);
}

int* row_get_int(char *row, Column *column) {
    return (int*)(row + column->offset);
}

char* row_get_char(char *row, Column *column) {
    return row + column->offset;
}

