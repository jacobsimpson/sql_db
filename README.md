# sql_db

This is a superficial implementation of an SQL database. It uses a file per
table model.

## Getting Started

All the development and test was done on Mac OS X. It will probably work on an
Unix install, but the installation process is only described for a Mac.

```bash
brew install check
brew install valgrind
```

## Development

### Executing the Program

At this point, the program only:

* initializes the metadata table if the tables are not initialized
* print the table descriptions
* print the table contents

```bash
make
build/db
```

During development, run the db program using valgrind to confirm the memory is
managed correctly.

```bash
valgrind --leak-check=yes build/db
```

