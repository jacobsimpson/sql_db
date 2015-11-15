
#ifndef __TABLE_H__
#define __TABLE_H__

#include "column.h"

typedef struct Table {
    char *name;
    int fd;
    int num_columns;
    Column *columns;
} Table;

int table_get_row_size(Table *table);

void table_create_columns(Table *table);

void table_create_tables(Table *table);

/* A database file contains a single table. */
void table_create(Table *table);

void table_open(Table* table);

Table *table_new(const char* table_name);

void table_free(Table *table);

void table_read_definition_tables(Table *table);

void table_read_definition_columns(Table *table);

void table_read_definition(Table *table, Table *columns);

#endif

