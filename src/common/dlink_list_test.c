#include "test_utils.h"

#include "dlink_list.h"

static MunitResult test_dlink_list_init(const MunitParameter params[],
                                        void *data) {
  (void)params;
  (void)data;

  dlink_list_t *dlink_list = dlink_list_init(sizeof(test_struct_t));
  munit_assert_not_null(dlink_list);
  dlink_list_free(dlink_list);

  return MUNIT_OK;
}

static MunitResult test_dlink_list_init_zero_size(const MunitParameter params[],
                                                  void *data) {
  (void)params;
  (void)data;

  dlink_list_t *dlink_list = dlink_list_init(0);

  munit_assert_null(dlink_list);

  return MUNIT_OK;
}

static MunitResult
test_dlink_list_insert_get_delete(const MunitParameter params[], void *data) {
  (void)params;
  (void)data;

  test_struct_t value_1 = {
      .integer = 1, .character = 'a', .string = "text-1", .number = 1.1};
  test_struct_t value_2 = {
      .integer = 2, .character = 'b', .string = "text-2", .number = 2.2};
  test_struct_t value_3 = {
      .integer = 3, .character = 'c', .string = "text-3", .number = 3.3};

  dlink_list_t *dlink_list = dlink_list_init(sizeof(test_struct_t));

  dlink_list_insert_at_end(dlink_list, &value_1);
  munit_assert_int(1, ==, dlink_list_size(dlink_list));
  dlink_list_insert_at_end(dlink_list, &value_2);
  munit_assert_int(2, ==, dlink_list_size(dlink_list));
  dlink_list_insert_at_end(dlink_list, &value_3);
  munit_assert_int(3, ==, dlink_list_size(dlink_list));

  test_struct_t *added_item_1 = dlink_list_get_from(dlink_list, 0);
  munit_assert_ptr_not_equal(added_item_1, &value_1);
  munit_assert_test_struct_equal(added_item_1, &value_1);

  test_struct_t *added_item_2 = dlink_list_get_from(dlink_list, 1);
  munit_assert_ptr_not_equal(added_item_2, &value_2);
  munit_assert_test_struct_equal(added_item_2, &value_2);

  test_struct_t *added_item_3 = dlink_list_get_from(dlink_list, 2);
  munit_assert_ptr_not_equal(added_item_3, &value_3);
  munit_assert_test_struct_equal(added_item_3, &value_3);

  dlink_list_delete_from(dlink_list, 1);
  munit_assert_int(2, ==, dlink_list_size(dlink_list));

  test_struct_t *item_1 = dlink_list_get_from(dlink_list, 0);
  munit_assert_ptr_not_equal(item_1, &value_1);
  munit_assert_test_struct_equal(item_1, &value_1);
  test_struct_t *item_2 = dlink_list_get_from(dlink_list, 1);
  munit_assert_ptr_not_equal(item_2, &value_3);
  munit_assert_test_struct_equal(item_2, &value_3);

  dlink_list_free(dlink_list);

  return MUNIT_OK;
}

static MunitTest tests[] = {
    munit_test("/init-free-empty-list", test_dlink_list_init),
    munit_test("/init-zero-size", test_dlink_list_init_zero_size),
    munit_test("/insert-get-delete", test_dlink_list_insert_get_delete),
    munit_null_test};

const MunitSuite dlink_list_suite = munit_suite("/dlink-list", tests);
