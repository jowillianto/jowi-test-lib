#include <jowi/test_lib.hpp>
#include <chrono>
#include <print>
#include <stdexcept>
import jowi.test_lib;

namespace test_lib = jowi::test_lib;

JOWI_ADD_TEST(create_with_struct_name) {
  test_lib::assert_equal(
    test_lib::get_test_context().tests.get(0).value().get().name(), "create_with_struct_name"
  );
}

JOWI_ADD_TEST(custom_name) {
  test_lib::assert_equal(
    test_lib::get_test_context().tests.get(1).value().get().name(), "custom_name"
  );
}

JOWI_ADD_TEST(run_throw_runtime_error) {
  auto conf = test_lib::TestEntry{[]() { throw std::runtime_error("LOL"); }};
  auto res = conf.run_test();
  test_lib::assert_equal(res.is_error(), true);
  test_lib::assert_equal(res.get_error().value().name, "std::runtime_error");
  test_lib::assert_equal(res.get_error().value().message, "LOL");
}

JOWI_ADD_TEST(test_time_format) {
  test_lib::TestTimeUnit prev_unit = test_lib::get_test_context().time_unit;
  test_lib::get_test_context().time_unit = test_lib::TestTimeUnit::MICRO_SECONDS;
  test_lib::assert_equal(
    test_lib::get_test_context().get_time(
      std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::microseconds{1})
    ),
    "1.00 μs"
  );
  test_lib::get_test_context().time_unit = test_lib::TestTimeUnit::MILLI_SECONDS;
  test_lib::assert_equal(
    test_lib::get_test_context().get_time(
      std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::milliseconds{1})
    ),
    "1.00 ms"
  );
  test_lib::get_test_context().time_unit = test_lib::TestTimeUnit::SECONDS;
  test_lib::assert_equal(
    test_lib::get_test_context().get_time(
      std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::seconds{1})
    ),
    "1.00 s"
  );
  test_lib::get_test_context().time_unit = prev_unit;
}

JOWI_SETUP(argc, argv) {
  test_lib::get_test_context().thread_count = 1;
}

/*
  It is here for proof of concept.
*/
JOWI_TEARDOWN() {
  test_lib::get_test_context().thread_count = 0;
}