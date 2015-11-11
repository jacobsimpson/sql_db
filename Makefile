BUILD=build
SRC=src
TEST=test

db::
	mkdir -p $(BUILD)
	gcc -g -O0 -o $(BUILD)/db $(SRC)/db.c

test::
	mkdir -p $(BUILD)
	gcc -l check -o $(BUILD)/test_columns_table $(TEST)/test_columns_table.c
	valgrind --leak-check=full $(BUILD)/test_columns_table

clean::
	rm -Rf $(BUILD)

