#include "test_utils.h"

#include "dyn_arr.h"

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
  munit_assert_test_struct_equal(added_item_1, &value_1);

  test_struct_t value_2 = {
      .integer = 2, .character = 'b', .string = "text-2", .number = 2.2};
  test_struct_t *added_item_2 = dyn_arr_push(list, &value_2);
  munit_assert_size(dyn_arr_size(list), ==, 2);
  munit_assert_size(dyn_arr_capacity(list), ==, 2);
  munit_assert_ptr_not_equal(added_item_2, &value_2);
  munit_assert_test_struct_equal(added_item_2, &value_2);

  test_struct_t value_3 = {
      .integer = 3, .character = 'c', .string = "text-3", .number = 3.3};
  test_struct_t *added_item_3 = dyn_arr_push(list, &value_3);
  munit_assert_size(dyn_arr_size(list), ==, 3);
  munit_assert_size(dyn_arr_capacity(list), ==, 4);
  munit_assert_ptr_not_equal(added_item_3, &value_3);
  munit_assert_test_struct_equal(added_item_3, &value_3);

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
  munit_assert_test_struct_equal(added_item_1, &value_1);

  test_struct_t *added_item_2 = dyn_arr_get(list, 1);
  munit_assert_ptr_not_equal(added_item_2, &value_2);
  munit_assert_test_struct_equal(added_item_2, &value_2);

  test_struct_t *added_item_3 = dyn_arr_get(list, 2);
  munit_assert_ptr_not_equal(added_item_3, &value_3);
  munit_assert_test_struct_equal(added_item_3, &value_3);

  dyn_arr_free(list);

  return MUNIT_OK;
}

static MunitTest tests[] = {
    munit_test("/init-zero-capacity", test_dyn_arr_init_zero_capacity),
    munit_test("/init-non-zero-capacity", test_dyn_arr_init_some_capacity),
    munit_test("/list-push", test_dyn_arr_push),
    munit_test("/list-get", test_dyn_arr_get), munit_null_test};
const MunitSuite dyn_arr_suite = munit_suite("/dyn-arr", tests);
