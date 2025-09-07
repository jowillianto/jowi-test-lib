import jowi.test_lib;
#include <concepts>

namespace test_lib = jowi::test_lib;

int main(int argc, const char **argv) {
  test_lib::get_test_context().setup(argc, argv);
  int err_count = test_lib::run_all_and_exit();
  test_lib::get_test_context().tear_down();
  return err_count;
}