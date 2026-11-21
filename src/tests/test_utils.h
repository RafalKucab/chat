#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <munit.h>

typedef struct TestStruct {
  int integer;
  char character;
  char *string;
  float number;
} test_struct_t;

#define munit_assert_test_struct_equal(actual, expected)                       \
  do {                                                                         \
    const test_struct_t *actual_ = (actual);                                   \
    const test_struct_t *expected_ = (expected);                               \
    munit_assert_not_null(actual_);                                            \
    munit_assert_int(actual_->integer, ==, expected_->integer);                \
    munit_assert_char(actual_->character, ==, expected_->character);           \
    munit_assert_string_equal(actual_->string, expected_->string);             \
    munit_assert_float(actual_->number, ==, expected_->number);                \
  } while (0)

#define munit_test(name, func)                                                 \
  {name, func, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

#define munit_null_test {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

#define munit_suite(name, tests) {name, tests, NULL, 1, MUNIT_SUITE_OPTION_NONE}

#define munit_null_suite {NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE}

extern const MunitSuite dyn_arr_suite;
extern const MunitSuite dlink_list_suite;

#endif
