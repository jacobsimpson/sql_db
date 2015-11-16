# sql_db

This is a superficial implementation of a table based database. It uses a file
per table model. It allows a tables to be:
* described
* selected from
* inserted into

## Installation

All the development and test was done on Mac OS X. It will probably work on an
Unix install, but the installation process is only described for a Mac.

These are the basic development tools used on top of XCode:

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

Create a new table by inserting the correct rows into the metadata tables,
'columns' and 'tables'.  When inserting a row, specify a value for all the
columns, in the order the columns appear in the output of the select statement.

```bash
build/db insert columns  9 dogs id         int  1 4  0
build/db insert columns 10 dogs dog_name   char 2 51 0
build/db insert columns 11 dogs age        int  4 4  0
build/db insert tables   2 dogs
```

## Development

```bash
make
make test
```

Execute the tests monitoring for memory leaks:

```bash
make test_valgrind
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

