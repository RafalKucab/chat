#include "test_utils.h"

int main(int argc, char *argv[]) {
  MunitSuite suites[] = {dyn_arr_suite, dlink_list_suite, munit_null_suite};
  const MunitSuite root = {"", NULL, suites, 1, MUNIT_SUITE_OPTION_NONE};

  return munit_suite_main(&root, NULL, argc, argv);
}
