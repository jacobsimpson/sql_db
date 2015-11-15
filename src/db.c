
#include <stdio.h>   /* C standard IO functions. */
#include <stdlib.h>  /* Prototypes of common lib functions.
                        exit(..)
                        Also, EXIT_FAILURE. */
#include <unistd.h>  /* Prototyes for some of the system functions. */
#include <fcntl.h>   /* O_RDONLY. */
#include <string.h>
#include "map.h"

int max(int i1, int i2) {
  if (i1 < i2) return i2;
  return i1;
}

int min(int i1, int i2) {
  if (i1 < i2) return i1;
  return i2;
}

/*
 * Rows.
 */
void write_row(int table_handle, char *row, int row_size) {
  write(table_handle, row, row_size);
}

ssize_t read_row(int table_handle, char *row, int row_size) {
  return read(table_handle, row, row_size);
}

/*
 * Columns.
 */
#define C_CHAR 1
#define C_INT 2

struct Column {
  char *name;
  int type;
  int position;
  int size; // The number of bytes occupied by the data in this column.
  int offset; // The location of this column data within a row.
};

void column_free_array(int num_columns, struct Column *columns) {
  for (int i = 0; i < num_columns; i++) {
    free(columns[i].name);
  }
  free(columns);
}

int* get_row_int_data(char *row, struct Column *column) {
  return (int*)(row + column->offset);
}

char* get_row_char_data(char *row, struct Column *column) {
  return row + column->offset;
}

/*
 * DataSet
 */
struct DataSet {
  int num_columns;
  struct Column **columns;
  int num_rows;
  char **rows;
};

struct DataSet *dataset_new() {
  struct DataSet *data_set = (struct DataSet*)malloc(sizeof(struct DataSet));
  data_set->num_columns = 0;
  data_set->columns = NULL;
  data_set->num_rows = 0;
  data_set->rows = NULL;
  return data_set;
}

void dataset_free(struct DataSet *data_set) {
  free(data_set->columns);
  free(data_set->rows);
  free(data_set);
}

void dataset_add_row(struct DataSet *data_set, char *row) {
  char **old = data_set->rows;
  data_set->rows = (char**)calloc(data_set->num_rows + 1, sizeof(char*));
  int i;
  for (i = 0; i < data_set->num_rows; i++) {
    data_set->rows[i] = old[i];
  }
  data_set->rows[i] = row;
  data_set->num_rows++;
  free(old);
}

void dataset_add_column(struct DataSet *data_set, struct Column *column) {
  struct Column **old = data_set->columns;
  data_set->columns = (struct Column **)calloc(data_set->num_columns + 1, sizeof(struct Column *));
  int i;
  for (i = 0; i < data_set->num_columns; i++) {
    data_set->columns[i] = old[i];
  }
  data_set->columns[i] = column;
  data_set->num_columns++;
  free(old);
}

void dataset_print(struct DataSet *data_set) {
  int total_width = 0;
  int col_print_width[data_set->num_columns];
  char *format = (char*)alloca(20 * sizeof(char));
  for (int i = 0; i < data_set->num_columns; i++) {
    struct Column *column = *(data_set->columns + i);
    switch (column->type) {
      case C_CHAR:
        col_print_width[i] = min(20, max(strlen(column->name), column->size)) + 1;
        break;
      case C_INT:
        col_print_width[i] = min(20, max(strlen(column->name), 7)) + 1;
        break;
      default:
        break;
    }
    snprintf(format, 20, "%%-%ds", col_print_width[i]);
    printf(format, column->name);
    total_width += col_print_width[i];
  }

  printf("\n");
  for (int i = 0; i < total_width; i++) printf("-");
  printf("\n");

  for (int i = 0; i < data_set->num_rows; i++) {
    char *row = *(data_set->rows + i);
    for (int j = 0; j < data_set->num_columns; j++) {
      struct Column *column = *(data_set->columns + j);
      switch (column->type) {
        case C_CHAR:
          snprintf(format, 20, "%%-%ds", col_print_width[j]);
          printf(format, get_row_char_data(row, column));
          break;
        case C_INT:
          snprintf(format, 20, "%%-%dd", col_print_width[j]);
          printf(format, *get_row_int_data(row, column));
          break;
        default:
          break;
      }
    }
    printf("\n");
  }
}


/*
 * Tables.
 */
struct Table {
  char *name;
  int fd;
  int num_columns;
  struct Column *columns;
};

int table_get_row_size(struct Table *table) {
  int result = 0;
  for (int i = 0; i < table->num_columns; i++) {
    result += table->columns[i].size;
  }
  return result;
}

void table_create_columns(struct Table *table) {
  lseek(table->fd, 0, SEEK_SET);

  int row_size = table_get_row_size(table);
  char *row = (char*)alloca(row_size * sizeof(char));
  memset(row, 0, row_size);
  int *id = get_row_int_data(row, table->columns);
  char *table_name = get_row_char_data(row, table->columns + 1);
  char *column_name = get_row_char_data(row, table->columns + 2);
  char *data_type = get_row_char_data(row, table->columns + 3);
  int *position = get_row_int_data(row, table->columns + 4);
  int *size = get_row_int_data(row, table->columns + 5);
  int *next_value = get_row_int_data(row, table->columns + 6);

  int i;
  for (i = 0; i < table->num_columns; i++) {
    *id = i;
    strlcpy(table_name, table->name, table->columns[1].size);
    strlcpy(column_name, table->columns[i].name, table->columns[2].size);
    if (table->columns[i].type == C_CHAR) {
      strlcpy(data_type, "char", table->columns[3].size);
    } else {
      strlcpy(data_type, "int", table->columns[3].size);
    }
    *position = table->columns[i].position;
    *size = table->columns[i].size;
    *next_value = 0;
    write_row(table->fd, row, row_size);
  }

  *id = i++;
  strlcpy(table_name, "tables", table->columns[1].size);
  strlcpy(column_name, "id", table->columns[2].size);
  strlcpy(data_type, "int", table->columns[3].size);
  *position = 1;
  *size = 4;
  *next_value = 0;
  write_row(table->fd, row, row_size);

  *id = i++;
  strlcpy(table_name, "tables", table->columns[1].size);
  strlcpy(column_name, "table_name", table->columns[2].size);
  strlcpy(data_type, "char", table->columns[3].size);
  *position = 2;
  *size = 101;
  *next_value = 0;
  write_row(table->fd, row, row_size);
}

void table_create_tables(struct Table *table) {
  lseek(table->fd, 0, SEEK_SET);

  int row_size = table_get_row_size(table);
  char *row = (char*)alloca(row_size * sizeof(char));
  memset(row, 0, row_size);
  int *id = get_row_int_data(row, table->columns);
  char *table_name = get_row_char_data(row, table->columns + 1);

  *id = 0;
  strlcpy(table_name, table->name, table->columns[1].size);
  write_row(table->fd, row, row_size);

  *id = 1;
  strlcpy(table_name, "columns", 8);
  write_row(table->fd, row, row_size);
}

/* A database file contains a single table. */
void table_create(struct Table *table) {
  table->fd = open(table->name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (table->fd == -1) {
    perror("create table file");
    exit(EXIT_FAILURE);
  }

  // The columns table is special. All other tables will be created empty,
  // but the columns table, in effect, defines the structure of the database,
  // so an empty columns table needs some basic information about the tables
  // every database has, 'columns' and 'tables'.
  if (strcmp(table->name, "columns") == 0) {
    table_create_columns(table);
  } else if (strcmp(table->name, "tables") == 0) {
    table_create_tables(table);
  }
}

void table_open(struct Table* table) {
  table->fd = open(table->name, O_RDWR);
  if (table->fd == -1) {
    perror("open table");
    table_create(table);
  }
}

struct Table *table_new(const char* table_name) {
  struct Table *table = (struct Table*)malloc(sizeof(struct Table));
  int table_name_size = (strlen(table_name) + 1);
  table->name = (char*)calloc(table_name_size, sizeof(char));
  strlcpy(table->name, table_name, table_name_size);
  table->num_columns = 0;
  return table;
}

void table_free(struct Table *table) {
  close(table->fd);
  free(table->name);
  column_free_array(table->num_columns, table->columns);
  free(table);
}


void table_read_definition_tables(struct Table *table) {
    table->num_columns = 2;

    table->columns = (struct Column*)calloc(table->num_columns, sizeof(struct Column));

    int offset = 0;
    // Structure of the 'id' column.
    table->columns[0].name = (char*)calloc(3, sizeof(char));
    strcpy(table->columns[0].name, "id");
    table->columns[0].type = C_INT;
    table->columns[0].size = 4;
    table->columns[0].offset = offset;
    offset += table->columns[0].size;

    // Structure of the 'table_name' column.
    table->columns[1].name = (char*)calloc(11, sizeof(char));
    strcpy(table->columns[1].name, "table_name");
    table->columns[1].type = C_CHAR;
    table->columns[1].size = 101; // An extra byte to record the string null termination.
    table->columns[1].offset = offset;
    offset += table->columns[1].size;
}

void table_read_definition_columns(struct Table *table) {
    table->num_columns = 7;

    table->columns = (struct Column*)calloc(table->num_columns, sizeof(struct Column));

    int offset = 0;
    // Structure of the 'id' column.
    table->columns[0].name = (char*)calloc(3, sizeof(char));
    strcpy(table->columns[0].name, "id");
    table->columns[0].type = C_INT;
    table->columns[0].position = 1;
    table->columns[0].size = 4;
    table->columns[0].offset = offset;
    offset += table->columns[0].size;

    // Structure of the 'table_name' column.
    table->columns[1].name = (char*)calloc(11, sizeof(char));
    strcpy(table->columns[1].name, "table_name");
    table->columns[1].type = C_CHAR;
    table->columns[1].position = 2;
    table->columns[1].size = 101; // An extra byte to record the string null termination.
    table->columns[1].offset = offset;
    offset += table->columns[1].size;

    // Structure of the 'column_name' column.
    table->columns[2].name = (char*)calloc(12, sizeof(char));
    strcpy(table->columns[2].name, "column_name");
    table->columns[2].type = C_CHAR;
    table->columns[2].position = 3;
    table->columns[2].size = 101; // An extra byte to record the string null termination.
    table->columns[2].offset = offset;
    offset += table->columns[2].size;

    // Structure of the 'data_type' column.
    table->columns[3].name = (char*)calloc(10, sizeof(char));
    strcpy(table->columns[3].name, "data_type");
    table->columns[3].type = C_CHAR;
    table->columns[3].position = 4;
    table->columns[3].size = 5; // An extra byte to record the string null termination.
    table->columns[3].offset = offset;
    offset += table->columns[3].size;

    // Structure of the 'ordinal_position' column.
    table->columns[4].name = (char*)calloc(17, sizeof(char));
    strcpy(table->columns[4].name, "ordinal_position");
    table->columns[4].type = C_INT;
    table->columns[4].position = 5;
    table->columns[4].size = 4;
    table->columns[4].offset = offset;
    offset += table->columns[4].size;

    // Structure of the 'size' column.
    table->columns[5].name = (char*)calloc(5, sizeof(char));
    strcpy(table->columns[5].name, "size");
    table->columns[5].type = C_INT;
    table->columns[5].position = 6;
    table->columns[5].size = 4;
    table->columns[5].offset = offset;
    offset += table->columns[5].size;

    // Structure of the 'next_value' column.
    table->columns[6].name = (char*)calloc(11, sizeof(char));
    strcpy(table->columns[6].name, "next_value");
    table->columns[6].type = C_INT;
    table->columns[6].position = 7;
    table->columns[6].size = 4;
    table->columns[6].offset = offset;
    offset += table->columns[6].size;
}

void table_read_definition(struct Table *table, struct Table *columns) {
  // The two tables that contain the structure of all the other tables are
  // statically defined.
  if (strcmp(table->name, "tables") == 0) {
    table_read_definition_tables(table);
  } else if (strcmp(table->name, "columns") == 0) {
    table_read_definition_columns(table);
  } else {
    int row_size = table_get_row_size(table);
    char *row = (char*)calloc(row_size, sizeof(char));
    memset(row, 0, row_size);

    lseek(columns->fd, 0, SEEK_SET);

    while (read_row(columns->fd, row, row_size) > 0) {
      for (int i = 0; i < table->num_columns; i++) {
        switch (table->columns[i].type) {
          case C_CHAR:
            printf("%-20s ", get_row_char_data(row, table->columns + i));
            break;
          case C_INT:
            printf("%-7d ", *get_row_int_data(row, table->columns + i));
            break;
          default:
            break;
        }
      }
      printf("\n");
    }
    free(row);
  }
}

struct DataSet *select_statement(struct Table *table) {
  struct DataSet *data_set = dataset_new();
  int row_size = table_get_row_size(table);

  lseek(table->fd, 0, SEEK_SET);

  for (int i = 0; i < table->num_columns; i++) {
    dataset_add_column(data_set, table->columns + i);
  }

  char *row = (char*)calloc(row_size, sizeof(char));
  while (read_row(table->fd, row, row_size) > 0) {
    dataset_add_row(data_set, row);
    row = (char*)calloc(row_size, sizeof(char));
  }
  free(row);
  return data_set;
}

int find_column(struct Table *table, const char *column_name) {
  for (int i = 0; i < table->num_columns; i++) {
    if (strcmp(table->columns[i].name, column_name) == 0) {
      return i;
    }
  }
  return -1;
}

void insert_statement(struct Table *table, struct DataSet *data_set) {
  lseek(table->fd, 0, SEEK_END);

  int row_size = table_get_row_size(table);
  char *row = (char*)alloca(row_size * sizeof(char));
  for (int r = 0; r < data_set->num_rows; r++) {
    memset(row, 0, row_size);
    for (int c = 0; c < data_set->num_columns; c++) {
      int i = find_column(table, data_set->columns[c]->name);
      if (i < 0) {
        printf("Unable to find the column '%s' in the table '%s'\n",
                data_set->columns[c]->name, table->name);
        return;
      }
      if (data_set->columns[c]->type == C_INT) {
        int *intVal = get_row_int_data(row, table->columns + i);
        *intVal = *get_row_int_data(data_set->rows[r], data_set->columns[c]);
      } else if (data_set->columns[c]->type == C_CHAR) {
        char *charVal = get_row_char_data(row, table->columns + i);
        strlcpy(charVal, get_row_char_data(data_set->rows[r], data_set->columns[c]),
                data_set->columns[c]->size);
      }
    }
    write_row(table->fd, row, row_size);
  }
}

void describe_statement(struct Table *table) {
  printf("   Table:'%s'\n", table->name);
  printf("-----------------------------------------------------------------\n");
  printf("name                                     type   size\n");
  printf("-----------------------------------------------------------------\n");
  for (int i = 0; i < table->num_columns; i++) {
    printf("%-40s %d      %d\n",
            table->columns[i].name, table->columns[i].type, table->columns[i].size);
  }
  printf("\n");
}

void print_help() {
  printf("db\n");
  printf("\n");
  printf("describe <table-name>\n");
  printf("    - describe the structure of the given table\n");
  printf("select <table-name>\n");
  printf("    - select all the rows of the given table\n");
  printf("insert <table-name> <value>+\n");
  printf("    - insert a new row into the given table\n");
  printf("      One value must be supplied for each column in the table\n");
  printf("      in the order the columns appear in the table.\n");
  printf("\n");
}

Map *open_all_tables() {
  Map *open_tables = map_new();

  struct Table *columns = table_new("columns");
  table_read_definition(columns, columns);
  table_open(columns);
  map_put(open_tables, "columns", columns);

  struct Table *tables = table_new("tables");
  table_read_definition(tables, columns);
  table_open(tables);
  map_put(open_tables, "tables", tables);

  return open_tables;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    print_help();
    exit(EXIT_SUCCESS);
  }
  Map *open_tables = open_all_tables();

  if (strcmp(argv[1], "describe") == 0) {
    struct Table *table = map_get(open_tables, argv[2]);
    if (table == NULL) {
      printf("There is no table '%s'.", argv[2]);
    } else {
      describe_statement(table);
    }
  } else if (strcmp(argv[1], "select") == 0) {
    struct Table *table = map_get(open_tables, argv[2]);
    if (table == NULL) {
      printf("There is no table '%s'.", argv[2]);
    } else {
      struct DataSet *data_set = select_statement(table);
      dataset_print(data_set);
      dataset_free(data_set);
    }
  } else if (strcmp(argv[1], "insert") == 0) {
    struct Table *table = map_get(open_tables, argv[2]);
    if (table == NULL) {
      printf("There is no table '%s'.", argv[2]);
    } else if (table->num_columns != (argc - 3)) {
      printf("The table has %d columns but %d data items were passed in.\n",
              table->num_columns,
              argc - 3);
    } else {
      struct DataSet *data_set = dataset_new();
      for (int i = 0; i < table->num_columns; i++) {
        dataset_add_column(data_set, table->columns + i);
      }

      int row_size = table_get_row_size(table);
      char *row = (char*)alloca(row_size * sizeof(char));
      memset(row, 0, row_size);
      for (int c = 0; c < data_set->num_columns; c++) {
        if (data_set->columns[c]->type == C_INT) {
          int *intVal = get_row_int_data(row, table->columns + c);
          *intVal = atoi(argv[c+3]);
        } else if (data_set->columns[c]->type == C_CHAR) {
          char *charVal = get_row_char_data(row, table->columns + c);
          strlcpy(charVal, argv[c+3], data_set->columns[c]->size);
        }
      }
      dataset_add_row(data_set, row);

      insert_statement(table, data_set);
      dataset_free(data_set);
    }
  }

  // Close all the tables.
  char **keys = map_keys(open_tables);
  for (int i = 0; i < map_size(open_tables); i++) {
    table_free(map_get(open_tables, keys[i]));
  }
  map_free(open_tables);
}

