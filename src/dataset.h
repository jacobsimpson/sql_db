
#ifndef __DATASET_H__
#define __DATASET_H__

#include "column.h"
#include "column_list.h"

typedef struct DataSet {
    ColumnList *columns;
    int num_rows;
    char **rows;
} DataSet;

DataSet *dataset_new();

void dataset_free(DataSet *data_set);

void dataset_add_row(DataSet *data_set, char *row);

void dataset_add_column(DataSet *data_set, Column *column);

void dataset_print(DataSet *data_set);

#endif

