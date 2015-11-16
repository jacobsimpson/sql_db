BUILD=build
SRC=src
TEST=test
SRC_FILES := $(wildcard $(SRC)/*.c)
OBJ_FILES := $(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(wildcard $(SRC)/*.c))
TEST_OBJ_FILES := $(patsubst $(TEST)/%.c, $(BUILD)/test/%, $(wildcard $(TEST)/*.c))

# A couple of notes on makefile automatic variables.
#     $@ - the target of the build rule.
#     $< - the first dependency of the build rule.
# https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html

$(BUILD)/db: $(OBJ_FILES)
	gcc -g -O0 -o $@ $^

test: $(TEST_OBJ_FILES)
	for f in $^; do ./$$f; done

test_valgrind: $(TEST_OBJ_FILES)
	@mkdir -p $(BUILD)/test_output
	@for f in $^; do \
		output=$${f/test/test_output} ; \
		valgrind --suppressions=test/check-suppression.txt \
				 --leak-check=full \
				 ./$$f >& $${output}.txt ; \
		if grep "ERROR SUMMARY:" $${output}.txt | grep -v ": 0 errors " >& /dev/null ; then \
			echo "Memory error in $$f" ; \
		fi \
	done

test_%: $(BUILD)/test/test_%
	$(BUILD)/test/$@

$(BUILD)/%.o: $(SRC)/%.c $(BUILD)
	gcc -g -O0 -c -o $@ $<

$(BUILD)/test/%: $(TEST)/%.c $(OBJ_FILES) $(BUILD)/test
	@# Build the object file for a test.
	gcc -g -O0 -I $(SRC) -c -o $@.o $<
	@# Link everything except the program object file containing the main
	@# function, because that would cause a linker conflict with the main
	@# function in the test file.
	gcc -g -O0 -lcheck -o $@ $(filter-out $(BUILD)/main.o,$(OBJ_FILES)) $@.o

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/test:
	mkdir -p $(BUILD)/test

style:
	astyle --style=google src/*.c src/*.h test/*.c

clean::
	rm -Rf $(BUILD)

