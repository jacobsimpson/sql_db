
#include "row.h"

void write_row(int table_handle, char *row, int row_size) {
    write(table_handle, row, row_size);
}

ssize_t read_row(int table_handle, char *row, int row_size) {
    return read(table_handle, row, row_size);
}

int* get_row_int_data(char *row, struct Column *column) {
    return (int*)(row + column->offset);
}

char* get_row_char_data(char *row, struct Column *column) {
    return row + column->offset;
}

