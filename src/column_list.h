
#ifndef __COLUMN_LIST_H__
#define __COLUMN_LIST_H__

#include "column.h"

typedef struct ColumnList ColumnList;

ColumnList *column_list_new();

void column_list_free(ColumnList *column_list);

Column *column_list_get(ColumnList *column_list, int i);

void column_list_add(ColumnList *column_list, Column *column);

int column_list_size(ColumnList *column_list);

void column_list_sort_position(ColumnList *column_list);

int column_list_find(ColumnList *column_list, const char *column_name);

#endif
