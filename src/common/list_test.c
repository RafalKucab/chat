#include "list.h"
#include <munit.h>

static MunitResult test_arr_init_zero_capacity(const MunitParameter params[],
                                               void *data) {
  (void)params;
  (void)data;

  chat_list_t *arr = arr_init(0);
  munit_assert_null(arr);
  arr_free(arr);

  return MUNIT_OK;
}

static MunitResult test_arr_init_some_capacity(const MunitParameter params[],
                                               void *data) {
  (void)params;
  (void)data;

  chat_list_t *arr = arr_init(20);
  munit_assert_not_null(arr);
  munit_assert_size(arr->size, ==, 0);
  munit_assert_size(arr->capacity, ==, 20);
  munit_assert_not_null(arr->data);
  arr_free(arr);

  return MUNIT_OK;
}

static MunitResult test_arr_push(const MunitParameter params[], void *data) {
  (void)params;
  (void)data;

  msg_t value_1 = {.user = "tom", .message = "msg 1"};
  chat_list_t *arr = arr_init(1);
  msg_t *added_item_1 = arr_push(arr, &value_1);
  munit_assert_size(arr->size, ==, 1);
  munit_assert_size(arr->capacity, ==, 1);
  munit_assert_ptr_not_equal(added_item_1, &value_1);
  munit_assert_string_equal(added_item_1->user, value_1.user);
  munit_assert_string_equal(added_item_1->message, value_1.message);

  msg_t value_2 = {.user = "joe", .message = "msg 2"};
  msg_t *added_item_2 = arr_push(arr, &value_2);
  munit_assert_size(arr->size, ==, 2);
  munit_assert_size(arr->capacity, ==, 2);
  munit_assert_ptr_not_equal(added_item_2, &value_2);
  munit_assert_string_equal(added_item_2->user, value_2.user);
  munit_assert_string_equal(added_item_2->message, value_2.message);

  msg_t value_3 = {.user = "bob", .message = "msg 3"};
  msg_t *added_item_3 = arr_push(arr, &value_3);
  munit_assert_size(arr->size, ==, 3);
  munit_assert_size(arr->capacity, ==, 4);
  munit_assert_ptr_not_equal(added_item_3, &value_3);
  munit_assert_string_equal(added_item_3->user, value_3.user);
  munit_assert_string_equal(added_item_3->message, value_3.message);

  arr_free(arr);

  return MUNIT_OK;
}

static MunitResult test_arr_get(const MunitParameter params[], void *data) {
  (void)params;
  (void)data;

  msg_t value_1 = {.user = "tom", .message = "msg 1"};
  msg_t value_2 = {.user = "joe", .message = "msg 2"};
  msg_t value_3 = {.user = "bob", .message = "msg 3"};

  chat_list_t *arr = arr_init(1);
  arr_push(arr, &value_1);
  arr_push(arr, &value_2);
  arr_push(arr, &value_3);

  msg_t *added_item_1 = arr_get(arr, 0);
  munit_assert_ptr_not_equal(added_item_1, &value_1);
  munit_assert_string_equal(added_item_1->user, value_1.user);
  munit_assert_string_equal(added_item_1->message, value_1.message);

  msg_t *added_item_2 = arr_get(arr, 1);
  munit_assert_ptr_not_equal(added_item_2, &value_2);
  munit_assert_string_equal(added_item_2->user, value_2.user);
  munit_assert_string_equal(added_item_2->message, value_2.message);

  msg_t *added_item_3 = arr_get(arr, 2);
  munit_assert_ptr_not_equal(added_item_3, &value_3);
  munit_assert_string_equal(added_item_3->user, value_3.user);
  munit_assert_string_equal(added_item_3->message, value_3.message);

  arr_free(arr);

  return MUNIT_OK;
}

#define munit_test(name, func)                                                 \
  {name, func, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

#define munit_null_test {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

#define munit_suite(name, tests) {name, tests, NULL, 1, MUNIT_SUITE_OPTION_NONE}

static MunitTest tests[] = {
    munit_test("/init-zero-capacity", test_arr_init_zero_capacity),
    munit_test("/init-non-zero-capacity", test_arr_init_some_capacity),
    munit_test("/arr-push", test_arr_push),
    munit_test("/arr-get", test_arr_get), munit_null_test};
static const MunitSuite suite = munit_suite("/dynamic-array", tests);

int main(int argc, char *argv[]) {
  return munit_suite_main(&suite, NULL, argc, argv);
}
