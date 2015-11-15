
#ifndef __ROW_H__
#define __ROW_H__

#include "column.h"
#include <unistd.h>

void write_row(int table_handle, char *row, int row_size);

ssize_t read_row(int table_handle, char *row, int row_size);

int* get_row_int_data(char *row, Column *column);

char* get_row_char_data(char *row, Column *column);

#endif

