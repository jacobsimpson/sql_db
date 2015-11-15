#include "map.h"
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

START_TEST(test_map_new_free)
{
    Map *map = map_new();
    ck_assert_ptr_ne(map, NULL);
    ck_assert_int_eq(map_size(map), 0);
    map_free(map);
}
END_TEST

START_TEST(test_map_put)
{
    Map *map = map_new();
    ck_assert_ptr_ne(map, NULL);
    ck_assert_int_eq(map_size(map), 0);

    int *value = malloc(sizeof(int));
    *value = 5;
    map_put(map, "five", value);
    ck_assert_int_eq(map_size(map), 1);

    map_free(map);
    free(value);
}
END_TEST

START_TEST(test_map_remove_non_existing)
{
    Map *map = map_new();
    ck_assert_ptr_ne(map, NULL);
    ck_assert_int_eq(map_size(map), 0);

    int *result = map_remove(map, "five");
    ck_assert_int_eq(map_size(map), 0);
    ck_assert_ptr_eq(result, NULL);

    map_free(map);
}
END_TEST

START_TEST(test_map_remove)
{
    Map *map = map_new();
    ck_assert_ptr_ne(map, NULL);
    ck_assert_int_eq(map_size(map), 0);

    int *value = malloc(sizeof(int));
    *value = 5;
    map_put(map, "five", value);
    ck_assert_int_eq(map_size(map), 1);

    int *result = map_remove(map, "five");
    ck_assert_int_eq(map_size(map), 0);
    ck_assert_int_eq(*value, *result);

    map_free(map);
    free(value);
}
END_TEST

START_TEST(test_map_put_key_multiple)
{
    Map *map = map_new();
    ck_assert_ptr_ne(map, NULL);
    ck_assert_int_eq(map_size(map), 0);

    int *value_1 = malloc(sizeof(int));
    *value_1 = 5;
    map_put(map, "five", value_1);
    ck_assert_int_eq(map_size(map), 1);
    int* result = map_get(map, "five");
    ck_assert_int_eq(*value_1, *result);

    int *value_2 = malloc(sizeof(int));
    *value_2 = 50;
    map_put(map, "five", value_2);
    ck_assert_int_eq(map_size(map), 1);
    result = map_get(map, "five");
    ck_assert_int_eq(*value_2, *result);

    map_free(map);
    free(value_1);
    free(value_2);
}
END_TEST

START_TEST(test_map_put_multiple_keys)
{
    char *numbers[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight"};

    Map *map = map_new_capacity(10);
    ck_assert_ptr_ne(map, NULL);
    ck_assert_int_eq(map_size(map), 0);
    for (int i = 0; i < 9; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        map_put(map, numbers[i], value);
        ck_assert_int_eq(map_size(map), i+1);
    }

    for (int i = 0; i < 9; i++) {
        int *result = map_get(map, numbers[i]);
        ck_assert_int_eq(i, *result);
    }

    for (int i = 0; i < 9; i++) {
        free(map_remove(map, numbers[i]));
    }
    map_free(map);
}
END_TEST

START_TEST(test_map_put_too_many_keys)
{
    char *numbers[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight"};

    Map *map = map_new_capacity(5);
    ck_assert_ptr_ne(map, NULL);
    ck_assert_int_eq(map_size(map), 0);
    for (int i = 0; i < 9; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        map_put(map, numbers[i], value);
        ck_assert_int_eq(map_size(map), i+1);
    }

    for (int i = 0; i < 9; i++) {
        int *result = map_get(map, numbers[i]);
        ck_assert_int_eq(i, *result);
    }

    for (int i = 0; i < 9; i++) {
        free(map_remove(map, numbers[i]));
    }
    map_free(map);
}
END_TEST

START_TEST(test_map_keys)
{
    char *numbers[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight"};

    Map *map = map_new();
    ck_assert_ptr_ne(map, NULL);
    ck_assert_int_eq(map_size(map), 0);
    for (int i = 0; i < 3; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        map_put(map, numbers[i], value);
        ck_assert_int_eq(map_size(map), i+1);
    }

    char **keys = map_keys(map);
    // The order of the results of map_keys are not guaranteed so this might be
    // fragile.
    strcmp(numbers[0], keys[0]);
    strcmp(numbers[1], keys[1]);
    strcmp(numbers[2], keys[2]);

    free(keys);

    int *value = map_get(map, numbers[0]);
    ck_assert_int_eq(0, *value);

    for (int i = 0; i < 3; i++) {
        free(map_remove(map, numbers[i]));
    }

    map_free(map);
}
END_TEST

Suite * map_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("map");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_map_new_free);
    tcase_add_test(tc_core, test_map_put);
    tcase_add_test(tc_core, test_map_remove_non_existing);
    tcase_add_test(tc_core, test_map_remove);
    tcase_add_test(tc_core, test_map_put_key_multiple);
    tcase_add_test(tc_core, test_map_put_multiple_keys);
    tcase_add_test(tc_core, test_map_put_too_many_keys);
    tcase_add_test(tc_core, test_map_keys);

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
