
#ifndef __TABLE_H__
#define __TABLE_H__

#include "column.h"
#include "column_list.h"
#include "dataset.h"

typedef struct Table {
    char *name;
    int fd;
    ColumnList *columns;
} Table;

Table *table_new(const char* table_name);

void table_free(Table *table);

int table_get_row_size(Table *table);

void table_create(Table *table);

void table_open(Table* table);

void table_read_definition(Table *table, Table *columns);

DataSet *table_scan(Table *table);

#endif

