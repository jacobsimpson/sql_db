# sql_db

This is a superficial implementation of an SQL database. It uses a file per
table model.

## Installation

All the development and test was done on Mac OS X. It will probably work on an
Unix install, but the installation process is only described for a Mac.

```bash
brew install check
brew install valgrind
```

## Getting Started

```bash
make
```

At this point, the program only:

* initializes the metadata table if the tables are not initialized
* print the table descriptions (describes a table)
* print the table contents (select * from table)
* insert a complete row

Select all the rows and columns from the 'tables' table.

```bash
build/db select tables
```

Describe the structure of the 'columns' table.

```bash
build/db describe columns
```

Select all the rows and columns in the 'columns' table.

```bash
build/db select columns
```

Insert a new row. You must specify a value for all the columns, in the order
the columns appear in the output of the select statement.

```bash
build/db insert tables 3 the_table_name
```

## Development

```bash
make
make test
build/db
```

You can run individual tests like so:

```bash
make test_map
```

See which individual tests are available like so:

```bash
ls test/test_*.c
```

During development, run the db program using valgrind to confirm the memory is
managed correctly.

```bash
valgrind --leak-check=yes build/db
```

