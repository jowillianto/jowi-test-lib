module;
#include <cmath>
#include <expected>
#include <format>
#include <ranges>
#include <source_location>
#include <string_view>
export module jowi.test_lib:assert;
import :exception;

namespace jowi::test_lib {
  /*
    Checks if two values are equal comparable.
  */
  template <typename T, typename V>
  concept equal_comp = requires(const T &x, const V &v) {
    { x == v } -> std::convertible_to<bool>;
  };
  /*
    Checks if two values are less then comparable.
  */
  template <typename T, typename V>
  concept lt_comp = requires(const T &x, const V &v) {
    { x < v } -> std::convertible_to<bool>;
  };

  /*
    Checks the equality of two values x and y. Throw a fail_assertion exception when the assertion
    fails. When used in the scope of a tester, exception will be caught and will invalidate the
    test.
  */
  export template <typename T, equal_comp<T> V>
  void assert_equal(
    const T &x, const V &y, const std::source_location &location = std::source_location::current()
  ) {
    if (x != y) {
      if constexpr (std::formattable<T, char> && std::formattable<V, char>) {
        throw fail_assertion(
          std::format(
            "At {} Line {} , {} is not equal to {}",
            std::string_view{std::string_view{location.file_name()}},
            location.line(),
            x,
            y
          )
        );
      } else {
        throw fail_assertion(
          std::format(
            "Assertion error at line {} file {}",
            location.line(),
            std::string_view{location.file_name()}
          )
        );
      }
    }
  }

  /*
    Assert that two values are not equal
  */
  export template <typename T, equal_comp<T> V>
  void assert_not_equal(
    const T &x, const V &y, const std::source_location &location = std::source_location::current()
  ) {
    if (x == y) {
      if constexpr (std::formattable<T, char> && std::formattable<V, char>) {
        throw fail_assertion(
          std::format(
            "At {} Line {} , {} is equal to {}",
            std::string_view{location.file_name()},
            location.line(),
            x,
            y
          )
        );
      } else {
        throw fail_assertion(
          std::format(
            "Assertion error at line {} file {}",
            location.line(),
            std::string_view{location.file_name()}
          )
        );
      }
    }
  }

  /*
    Checks if x is less than y. Throw a fail_assertion exception when the assertion
    fails. When used in the scope of a tester, exception will be caught and will invalidate the
    test.
  */
  export template <typename T, equal_comp<T> V>
  void assert_lt(
    const T &x, const V &y, const std::source_location &location = std::source_location::current()
  ) {
    if (x >= y) {
      if constexpr (std::formattable<T, char> && std::formattable<V, char>) {
        throw fail_assertion(
          std::format(
            "At {} Line {} , {} is not less than {}",
            std::string_view{location.file_name()},
            location.line(),
            x,
            y
          )
        );
      } else {
        throw fail_assertion(
          std::format(
            "Assertion error at line {} file {}",
            location.line(),
            std::string_view{location.file_name()}
          )
        );
      }
    }
  }

  export template <std::ranges::input_range T, std::ranges::input_range V, typename F>
    requires(
      std::invocable<F, std::ranges::range_value_t<T>, std::ranges::range_value_t<V>> ||
      std::invocable<
        F,
        std::ranges::range_value_t<T>,
        std::ranges::range_value_t<V>,
        std::source_location>
    )
  void assert_func(
    const T &x,
    const V &y,
    F &&comp,
    const std::source_location &location = std::source_location::current()
  ) {
    size_t idx = 0;
    for (const auto &[v_x, v_y] : std::ranges::zip_view(x, y)) {
      try {
        if constexpr (std::invocable<
                        F,
                        std::ranges::range_value_t<T>,
                        std::ranges::range_value_t<V>>) {
          comp(v_x, v_y);
        } else {
          comp(v_x, v_y, location);
        }
      } catch (const fail_assertion &e) {
        throw fail_assertion(std::format("{} at index {}", e.msg(), idx));
      }
      idx += 1;
    }
  }

  /*
    Cheks the equality of two containers supporting the ranges paradigm.
  */
  export template <std::ranges::input_range T, std::ranges::input_range V>
    requires(
      equal_comp<std::ranges::range_value_t<T>, std::ranges::range_value_t<V>> && !equal_comp<T, V>
    )
  void assert_equal(
    const T &x, const V &y, const std::source_location &location = std::source_location::current()
  ) {
    assert_func(
      x,
      y,
      [](const auto &x, const auto &y, const std::source_location &loc) {
        assert_equal(x, y, loc);
      },
      location
    );
  }

  /*
    Checks if two range containers are all less then the first one
  */
  export template <std::ranges::input_range T, std::ranges::input_range V>
    requires(
      lt_comp<std::ranges::range_value_t<T>, std::ranges::range_value_t<V>> && !lt_comp<T, V>
    )
  void assert_lt(
    const T &x, const V &y, const std::source_location &location = std::source_location::current()
  ) {
    assert_func(
      x,
      y,
      [](const auto &x, const auto &y, const std::source_location &loc) { assert_lt(x, y, loc); },
      location
    );
  }
  /*
    Assert non equality
  */
  export template <std::ranges::input_range T, std::ranges::input_range V>
    requires(
      equal_comp<std::ranges::range_value_t<T>, std::ranges::range_value_t<V>> && !equal_comp<T, V>
    )
  void assert_not_equal(
    const T &x, const V &y, const std::source_location &location = std::source_location::current()
  ) {
    assert_func(
      x,
      y,
      [](const auto &x, const auto &y, const std::source_location &loc) {
        assert_not_equal(x, y, loc);
      },
      location
    );
  }

  /*
    Checks if a statement is true.
  */
  export void assert_true(
    bool expr,
    std::string_view err_msg = "expr is not true",
    const std::source_location &location = std::source_location::current()
  ) {
    if (!expr) {
      throw fail_assertion(
        std::format(
          "At {} Line {} , {}", std::string_view{location.file_name()}, location.line(), err_msg
        )
      );
    }
  }
  export void assert_false(
    bool expr,
    std::string_view err_msg = "expr is true",
    const std::source_location &location = std::source_location::current()
  ) {
    if (expr) {
      throw fail_assertion(
        std::format(
          "At {} Line {} , {}", std::string_view{location.file_name()}, location.line(), err_msg
        )
      );
    }
  }

  export void assert_close(double x, double y, double tolerance = 1e-6) {
    if (std::abs(x - y) > tolerance) {
      throw fail_assertion(std::format("{} {} are not close within {}", x, y, tolerance));
    }
  }

  export template <typename T, typename E>
  void assert_expected(
    const std::expected<T, E> &e, const std::source_location &loc = std::source_location::current()
  ) {
    if (!e.has_value()) {
      if constexpr (requires() { e.error().what(); }) {
        test_lib::assert_true(e.has_value(), e.error().what(), loc);
      } else if constexpr (std::formattable<E, char>) {
        test_lib::assert_true(e.has_value(), std::format("{}", e.error()), loc);
      } else {
        test_lib::assert_true(e.has_value(), "Expected Value is not true", loc);
      }
    }
  }

  export template <typename T, typename E>
  T assert_expected_value(
    std::expected<T, E> &&res, const std::source_location &loc = std::source_location::current()
  ) {
    if (!res.has_value()) {
      // This is going to throw
      assert_expected(res, loc);
      throw;
    } else if constexpr (!std::same_as<T, void>) {
      return std::move(res.value());
    }
  }

  export template <is_exception... exceptions>
  void assert_throw(
    std::invocable auto &&f, const std::source_location &loc = std::source_location::current()
  ) {
    if constexpr (sizeof...(exceptions)) {
      auto catcher = exception_catcher<exceptions...>::make();
      auto res = catcher.safely_run_invocable(std::forward<std::decay_t<decltype(f)>>(f));
      test_lib::assert_true(!res.has_value(), "No exception thrown", loc);
    } else {
      try {
        f();
        test_lib::assert_true(false, "No exception thrown", loc);
      } catch (...) {
      }
    }
  }

  template <std::floating_point number_type>
  void assert_close(
    number_type l,
    number_type r,
    number_type tol = 1e-6,
    const std::source_location &loc = std::source_location::current()
  ) {
    number_type rms_diff = std::sqrt((l - r) * (l - r));
    assert_lt(rms_diff, tol, loc);
  }
}
