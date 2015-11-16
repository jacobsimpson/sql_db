
#ifndef __column_h__
#define __column_h__

#define C_CHAR 1
#define C_INT 2

typedef struct Column {
    char *name;
    int type;
    int position;
    int size; // The number of bytes occupied by the data in this column.
    int offset; // The location of this column data within a row.
} Column;

Column *column_new_w_name(const char *name);

void column_free(Column *column);

void column_free_array(int num_columns, Column *columns);

#endif
