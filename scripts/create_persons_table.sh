#! /bin/bash

# This script creates a new table in the database by populating the metadata
# tables with the correct rows.

build/db insert columns  9 persons id         int  1 4  0
build/db insert columns 10 persons first_name char 2 51 0
build/db insert columns 11 persons last_name  char 3 51 0
build/db insert columns 12 persons age        int  4 4  0
build/db insert tables   2 persons

