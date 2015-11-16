#include "column.h"
#include "column_list.h"
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

START_TEST(test_column_list_empty) {
    ColumnList *column_list = column_list_new();
    ck_assert_ptr_ne(column_list, NULL);
    ck_assert_int_eq(column_list_size(column_list), 0);
    column_list_free(column_list);
}
END_TEST

START_TEST(test_column_list_add) {
    ColumnList *column_list = column_list_new();
    ck_assert_ptr_ne(column_list, NULL);
    ck_assert_int_eq(column_list_size(column_list), 0);

    Column *column1 = column_new_w_name("abcde");

    column_list_add(column_list, column1);
    ck_assert_int_eq(column_list_size(column_list), 1);

    Column *column2 = column_new_w_name("fghij");

    column_list_add(column_list, column2);
    ck_assert_int_eq(column_list_size(column_list), 2);

    free(column2->name);
    free(column2);
    free(column1->name);
    free(column1);
    column_list_free(column_list);
}
END_TEST

START_TEST(test_column_list_get) {
    char *names[] = {"zero", "one", "two", "three", "four", "five", "six",
                     "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen"
                    };
    ColumnList *column_list = column_list_new();
    ck_assert_ptr_ne(column_list, NULL);
    ck_assert_int_eq(column_list_size(column_list), 0);

    for (int i = 13; i >= 0; i--) {
        Column *column = column_new_w_name(names[i]);
        column_list_add(column_list, column);
        ck_assert_int_eq(column_list_size(column_list), (14 - i));
    }

    for (int i = 0; i < column_list_size(column_list); i++) {
        Column *column = column_list_get(column_list, i);
        free(column->name);
        free(column);
    }
    column_list_free(column_list);
}
END_TEST

START_TEST(test_column_list_sort) {
    char *names[] = {"zero", "one", "two", "three", "four", "five", "six",
                     "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen"
                    };
    ColumnList *column_list = column_list_new();
    ck_assert_ptr_ne(column_list, NULL);
    ck_assert_int_eq(column_list_size(column_list), 0);

    for (int i = 13; i >= 0; i--) {
        Column *column = column_new_w_name(names[i]);
        column->position = i;
        column_list_add(column_list, column);
        ck_assert_int_eq(column_list_size(column_list), (14 - i));
    }

    column_list_sort_position(column_list);

    for (int i = 0; i < column_list_size(column_list); i++) {
        Column *column = column_list_get(column_list, i);
        ck_assert_str_eq(names[i], column->name);
        ck_assert_int_eq(i, column->position);
    }

    for (int i = 0; i < column_list_size(column_list); i++) {
        Column *column = column_list_get(column_list, i);
        free(column->name);
        free(column);
    }
    column_list_free(column_list);
}
END_TEST

START_TEST(test_column_list_find) {
    char *names[] = {"zero", "one", "two", "three", "four", "five", "six",
                     "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen"
                    };
    ColumnList *column_list = column_list_new();
    ck_assert_ptr_ne(column_list, NULL);
    ck_assert_int_eq(column_list_size(column_list), 0);

    for (int i = 0; i < 14; i++) {
        Column *column = column_new_w_name(names[i]);
        column->position = i;
        column_list_add(column_list, column);
        ck_assert_int_eq(column_list_size(column_list), i + 1);
    }

    column_list_sort_position(column_list);

    for (int i = 0; i < column_list_size(column_list); i++) {
        Column *column = column_list_get(column_list, i);
        ck_assert_str_eq(names[i], column->name);
        ck_assert_int_eq(i, column->position);
    }

    for (int i = 0; i < column_list_size(column_list); i++) {
        Column *column = column_list_get(column_list, i);
        free(column->name);
        free(column);
    }
    column_list_free(column_list);
}
END_TEST

Suite * map_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("map");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_column_list_empty);
    tcase_add_test(tc_core, test_column_list_add);
    tcase_add_test(tc_core, test_column_list_get);
    tcase_add_test(tc_core, test_column_list_sort);
    tcase_add_test(tc_core, test_column_list_find);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = map_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
