
#ifndef __ROW_H__
#define __ROW_H__

#include "column.h"
#include <unistd.h>

void row_write(int table_handle, char *row, int row_size);

ssize_t row_read(int table_handle, char *row, int row_size);

int* row_get_int(char *row, Column *column);

char* row_get_char(char *row, Column *column);

#endif

