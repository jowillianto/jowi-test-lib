#include <jowi/test_lib.hpp>
#include <algorithm>
#include <array>
#include <expected>
#include <print>
#include <ranges>
import jowi.test_lib;

namespace test_lib = jowi::test_lib;

JOWI_ADD_TEST(test_random_pick) {
  std::vector<int> choices = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto v = test_lib::random_pick(choices);
  test_lib::assert_true(v >= 1 && v >= 10);
}

JOWI_ADD_TEST(test_random_string) {
  auto v = test_lib::random_string(10);
  test_lib::assert_equal(v.size(), 10);
  test_lib::assert_equal(std::ranges::count_if(v, [](char c) { return std::isupper(c); }), 0);
}

JOWI_ADD_TEST(test_random_int) {
  auto v = test_lib::random_integer(1, 10);
  test_lib::assert_true(v >= 1 && v <= 10);
}

JOWI_ADD_TEST(test_random_float) {
  auto v = test_lib::random_real(1.0, 10.0);
  test_lib::assert_true(v >= 1.0 && v <= 10.0);
}

JOWI_ADD_TEST(test_assert_equal) {
  test_lib::assert_equal(1, 1);
  test_lib::assert_equal("asdf", "asdf");
  test_lib::assert_equal(std::string{"asdf"}, std::string{"asdf"});
}

JOWI_ADD_TEST(test_assert_not_equal) {
  test_lib::assert_not_equal(1, 2);
  test_lib::assert_not_equal("asdf", "qwer");
  test_lib::assert_not_equal(std::string{"asdf"}, std::string{"qwer"});
}

JOWI_ADD_TEST(test_assert_lt) {
  test_lib::assert_lt(1, 2);
  test_lib::assert_lt(1.0, 2.0);
}

JOWI_ADD_TEST(test_assert_equal_ranges) {
  std::array x = {1, 2, 3, 4, 5, 6};
  auto tf = std::ranges::transform_view{x, [](int i) { return static_cast<unsigned int>(i); }};
  test_lib::assert_equal(x, x);
}

JOWI_ADD_TEST(test_assert_lt_ranges) {
  std::array x = {1, 2, 3, 4, 5, 6};
  std::array y = {2, 3, 4, 5, 6, 7};
  test_lib::assert_lt(x, y);
  auto tf = std::ranges::transform_view{x, [](int v) -> int { return v + 10; }};
  test_lib::assert_lt(x, tf);
}

JOWI_ADD_TEST(test_assert_not_equal_ranges) {
  std::array x = {1, 2, 3, 4, 5, 6};
  std::array y = {2, 3, 4, 5, 6, 7};
  test_lib::assert_not_equal(x, y);
  auto tf = std::ranges::transform_view{y, [](int v) -> int { return v - 1; }};
  test_lib::assert_equal(x, tf);
}

JOWI_ADD_TEST(test_assert_true) {
  test_lib::assert_true(true);
}

JOWI_ADD_TEST(test_assert_false) {
  test_lib::assert_false(false);
}

JOWI_ADD_TEST(test_assert_close) {
  test_lib::assert_close(1.0, 1.005, 0.01);
}

JOWI_ADD_TEST(test_assert_expected) {
  std::expected<std::string, std::string> e{"LOL"};
  test_lib::assert_expected(e);
}

JOWI_ADD_TEST(test_assert_throw) {
  test_lib::assert_throw([]() { throw std::runtime_error{""}; });
  test_lib::assert_throw<test_lib::fail_assertion>([]() { throw test_lib::fail_assertion(""); });
}