
#include "column_list.h"
#include <stdlib.h>
#include <string.h>

struct ColumnList {
    int capacity;
    int size;
    Column **columns;
};

ColumnList *column_list_new() {
    ColumnList *column_list = malloc(sizeof(ColumnList));
    column_list->capacity = 10;
    column_list->size = 0;
    column_list->columns = malloc(column_list->capacity * sizeof(Column*));
    return column_list;
}

void column_list_free(ColumnList *column_list) {
    free(column_list->columns);
    free(column_list);
}

int column_list_size(ColumnList *column_list) {
    return column_list->size;
}

Column *column_list_get(ColumnList *column_list, int i) {
    if (i >= column_list->size) {
        return NULL;
    }
    return column_list->columns[i];
}

void column_list_add(ColumnList *column_list, Column *column) {
    if (column_list->size == column_list->capacity) {
        column_list->capacity *= 2;
        Column **old = column_list->columns;
        column_list->columns = malloc(column_list->capacity * sizeof(Column*));
        for (int i = 0; i < column_list->size; i++) {
            column_list->columns[i] = old[i];
        }
        free(old);
    }
    column_list->columns[column_list->size] = column;
    column_list->size++;
}

static void swap(ColumnList *column_list, int i, int j) {
    Column *tmp = column_list->columns[i];
    column_list->columns[i] = column_list->columns[j];
    column_list->columns[j] = tmp;
}

static int partition(ColumnList *column_list, int lo, int hi) {
    int pivot = column_list->columns[hi]->position;
    int i = lo;
    for (int j = lo; j < hi; j++) {
        if (column_list->columns[j]->position < pivot) {
            swap(column_list, i++, j);
        }
    }
    swap(column_list, i, hi);
    return i;
}

static void sort(ColumnList *column_list, int lo, int hi) {
    if (lo < hi) {
        int p = partition(column_list, lo, hi);
        sort(column_list, lo, p - 1);
        sort(column_list, p + 1, hi);
    }
}

void column_list_sort_position(ColumnList *column_list) {
    sort(column_list, 0, column_list->size - 1);
}

int column_list_find(ColumnList *column_list, const char *column_name) {
    for (int i = 0; i < column_list_size(column_list); i++) {
        if (strcmp(column_list_get(column_list, i)->name, column_name) == 0) {
            return i;
        }
    }
    return -1;
}

