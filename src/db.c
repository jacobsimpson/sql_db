
#include <stdio.h>   /* C standard IO functions. */
#include <stdlib.h>  /* Prototypes of common lib functions.
                        exit(..)
                        Also, EXIT_FAILURE. */
#include <unistd.h>  /* Prototyes for some of the system functions. */
#include <fcntl.h>   /* O_RDONLY. */
#include <string.h>

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
 * ResultSet
 */
struct ResultSet {
  int num_columns;
  struct Column **columns;
  int num_rows;
  char **rows;
};

struct ResultSet *resultset_new() {
  struct ResultSet *resultset = (struct ResultSet*)malloc(sizeof(struct ResultSet));
  resultset->num_columns = 0;
  resultset->columns = NULL;
  resultset->num_rows = 0;
  resultset->rows = NULL;
  return resultset;
}

void resultset_free(struct ResultSet *resultset) {
  free(resultset->columns);
  free(resultset->rows);
  free(resultset);
}

void resultset_add_row(struct ResultSet *resultset, char *row) {
  char **old = resultset->rows;
  resultset->rows = (char**)calloc(resultset->num_rows + 1, sizeof(char*));
  int i;
  for (i = 0; i < resultset->num_rows; i++) {
    resultset->rows[i] = old[i];
  }
  resultset->rows[i] = row;
  resultset->num_rows++;
  free(old);
}

void resultset_add_column(struct ResultSet *resultset, struct Column *column) {
  struct Column **old = resultset->columns;
  resultset->columns = (struct Column **)calloc(resultset->num_columns + 1, sizeof(struct Column *));
  int i;
  for (i = 0; i < resultset->num_columns; i++) {
    resultset->columns[i] = old[i];
  }
  resultset->columns[i] = column;
  resultset->num_columns++;
  free(old);
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

struct ResultSet *select_statement(struct Table *table) {
  struct ResultSet *resultset = resultset_new();
  int row_size = table_get_row_size(table);

  lseek(table->fd, 0, SEEK_SET);

  for (int i = 0; i < table->num_columns; i++) {
    resultset_add_column(resultset, table->columns + i);
  }

  char *row = (char*)calloc(row_size, sizeof(char));
  while (read_row(table->fd, row, row_size) > 0) {
    resultset_add_row(resultset, row);
    row = (char*)calloc(row_size, sizeof(char));
  }
  free(row);
  return resultset;
}

void resultset_print(struct ResultSet *resultset) {
  int total_width = 0;
  int col_print_width[resultset->num_columns];
  char *format = (char*)alloca(20 * sizeof(char));
  for (int i = 0; i < resultset->num_columns; i++) {
    struct Column *column = *(resultset->columns + i);
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

  for (int i = 0; i < resultset->num_rows; i++) {
    char *row = *(resultset->rows + i);
    for (int j = 0; j < resultset->num_columns; j++) {
      struct Column *column = *(resultset->columns + j);
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

void describe_statement(struct Table *table) {
  printf("   Table:'%s'\n", table->name);
  printf("-----------------------------------------------------------------\n");
  printf("name                                     type   size\n");
  printf("-----------------------------------------------------------------\n");
  for (int i = 0; i < table->num_columns; i++) {
    printf("%-40s %d      %d\n", table->columns[i].name, table->columns[i].type, table->columns[i].size);
  }
  printf("\n");
}

int main() {
  struct Table *columns = table_new("columns");
  table_read_definition(columns, columns);
  table_open(columns);

  describe_statement(columns);

  struct ResultSet *resultset = select_statement(columns);
  resultset_print(resultset);
  resultset_free(resultset);

  struct Table *tables = table_new("tables");
  table_read_definition(tables, columns);
  table_open(tables);

  printf("\n\n\n");
  describe_statement(tables);

  resultset = select_statement(tables);
  resultset_print(resultset);
  resultset_free(resultset);

  table_free(tables);

  table_free(columns);
}

