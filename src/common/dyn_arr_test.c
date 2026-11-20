#include <munit.h>

#include "dyn_arr.h"

typedef struct TestStruct {
  int integer;
  char character;
  char *string;
  float number;
} test_struct_t;

static MunitResult
test_dyn_arr_init_zero_capacity(const MunitParameter params[], void *data) {
  (void)params;
  (void)data;

  dyn_arr_t *list = dyn_arr_init(0, sizeof(test_struct_t));
  munit_assert_null(list);
  dyn_arr_free(list);

  return MUNIT_OK;
}

static MunitResult
test_dyn_arr_init_some_capacity(const MunitParameter params[], void *data) {
  (void)params;
  (void)data;

  dyn_arr_t *list = dyn_arr_init(20, sizeof(test_struct_t));
  munit_assert_not_null(list);
  munit_assert_size(dyn_arr_size(list), ==, 0);
  munit_assert_size(dyn_arr_capacity(list), ==, 20);
  dyn_arr_free(list);

  return MUNIT_OK;
}

static MunitResult test_dyn_arr_push(const MunitParameter params[],
                                     void *data) {
  (void)params;
  (void)data;

  test_struct_t value_1 = {
      .integer = 1, .character = 'a', .string = "text-1", .number = 1.1};
  dyn_arr_t *list = dyn_arr_init(1, sizeof(test_struct_t));
  test_struct_t *added_item_1 = dyn_arr_push(list, &value_1);
  munit_assert_size(dyn_arr_size(list), ==, 1);
  munit_assert_size(dyn_arr_capacity(list), ==, 1);
  munit_assert_ptr_not_equal(added_item_1, &value_1);
  munit_assert_char(added_item_1->character, ==, value_1.character);
  munit_assert_int(added_item_1->integer, ==, value_1.integer);
  munit_assert_string_equal(added_item_1->string, value_1.string);
  munit_assert_float(added_item_1->number, ==, value_1.number);

  test_struct_t value_2 = {
      .integer = 2, .character = 'b', .string = "text-2", .number = 2.2};
  test_struct_t *added_item_2 = dyn_arr_push(list, &value_2);
  munit_assert_size(dyn_arr_size(list), ==, 2);
  munit_assert_size(dyn_arr_capacity(list), ==, 2);
  munit_assert_ptr_not_equal(added_item_2, &value_2);
  munit_assert_char(added_item_2->character, ==, value_2.character);
  munit_assert_int(added_item_2->integer, ==, value_2.integer);
  munit_assert_string_equal(added_item_2->string, value_2.string);
  munit_assert_float(added_item_2->number, ==, value_2.number);

  test_struct_t value_3 = {
      .integer = 3, .character = 'c', .string = "text-3", .number = 3.3};
  test_struct_t *added_item_3 = dyn_arr_push(list, &value_3);
  munit_assert_size(dyn_arr_size(list), ==, 3);
  munit_assert_size(dyn_arr_capacity(list), ==, 4);
  munit_assert_ptr_not_equal(added_item_3, &value_3);
  munit_assert_char(added_item_3->character, ==, value_3.character);
  munit_assert_int(added_item_3->integer, ==, value_3.integer);
  munit_assert_string_equal(added_item_3->string, value_3.string);
  munit_assert_float(added_item_3->number, ==, value_3.number);

  dyn_arr_free(list);

  return MUNIT_OK;
}

static MunitResult test_dyn_arr_get(const MunitParameter params[], void *data) {
  (void)params;
  (void)data;

  test_struct_t value_1 = {
      .integer = 1, .character = 'a', .string = "text-1", .number = 1.1};
  test_struct_t value_2 = {
      .integer = 2, .character = 'b', .string = "text-2", .number = 2.2};
  test_struct_t value_3 = {
      .integer = 3, .character = 'c', .string = "text-3", .number = 3.3};

  dyn_arr_t *list = dyn_arr_init(1, sizeof(test_struct_t));
  dyn_arr_push(list, &value_1);
  dyn_arr_push(list, &value_2);
  dyn_arr_push(list, &value_3);

  test_struct_t *added_item_1 = dyn_arr_get(list, 0);
  munit_assert_ptr_not_equal(added_item_1, &value_1);
  munit_assert_char(added_item_1->character, ==, value_1.character);
  munit_assert_int(added_item_1->integer, ==, value_1.integer);
  munit_assert_string_equal(added_item_1->string, value_1.string);
  munit_assert_float(added_item_1->number, ==, value_1.number);

  test_struct_t *added_item_2 = dyn_arr_get(list, 1);
  munit_assert_ptr_not_equal(added_item_2, &value_2);
  munit_assert_char(added_item_2->character, ==, value_2.character);
  munit_assert_int(added_item_2->integer, ==, value_2.integer);
  munit_assert_string_equal(added_item_2->string, value_2.string);
  munit_assert_float(added_item_2->number, ==, value_2.number);

  test_struct_t *added_item_3 = dyn_arr_get(list, 2);
  munit_assert_ptr_not_equal(added_item_3, &value_3);
  munit_assert_char(added_item_3->character, ==, value_3.character);
  munit_assert_int(added_item_3->integer, ==, value_3.integer);
  munit_assert_string_equal(added_item_3->string, value_3.string);
  munit_assert_float(added_item_3->number, ==, value_3.number);

  dyn_arr_free(list);

  return MUNIT_OK;
}

#define munit_test(name, func)                                                 \
  {name, func, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

#define munit_null_test {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

#define munit_suite(name, tests) {name, tests, NULL, 1, MUNIT_SUITE_OPTION_NONE}

static MunitTest tests[] = {
    munit_test("/init-zero-capacity", test_dyn_arr_init_zero_capacity),
    munit_test("/init-non-zero-capacity", test_dyn_arr_init_some_capacity),
    munit_test("/list-push", test_dyn_arr_push),
    munit_test("/list-get", test_dyn_arr_get), munit_null_test};
static const MunitSuite suite = munit_suite("/dynamic-listay", tests);

int main(int argc, char *argv[]) {
  return munit_suite_main(&suite, NULL, argc, argv);
}
