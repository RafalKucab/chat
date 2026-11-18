#include "list.h"
#include <munit.h>

static MunitResult test_init(const MunitParameter params[], void *data) {
  (void)params;
  (void)data;

  chat_list_t *arr = init_arr(0);

  munit_assert_not_null(arr);

  return MUNIT_OK;
}

#define munit_test(name, func)                                                 \
  {name, func, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},

static MunitTest tests[] = {
    munit_test("/init", test_init)
    // {"/init", test_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

static const MunitSuite suite = {"/dynamic-array", tests, NULL, 1,
                                 MUNIT_SUITE_OPTION_NONE};

int main(int argc, char *argv[]) {
  return munit_suite_main(&suite, NULL, argc, argv);
}
