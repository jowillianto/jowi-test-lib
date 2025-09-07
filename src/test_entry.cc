module;
#include <chrono>
#include <concepts>
#include <expected>
#include <memory>
#include <optional>
#include <string_view>
export module jowi.test_lib:test_entry;
import :exception;
import :reflection;

namespace jowi::test_lib {
  /*
    A structure that contains the test result.
  */
  export struct test_result {
  public:
    test_result(
      std::chrono::system_clock::duration dur, std::optional<exception_info> err = std::nullopt
    ) : __runtime{dur}, __err{std::move(err)} {}

    std::chrono::system_clock::duration running_time() const {
      return __runtime;
    }

    std::optional<exception_info> get_error() const {
      return __err;
    }

    bool is_ok() const {
      return !__err.has_value();
    }
    bool is_error() const {
      return __err.has_value();
    }

  private:
    std::chrono::system_clock::duration __runtime;
    std::optional<exception_info> __err;
  };

  /*
    The base class for all tests. This allows a class to basically be saved in a vector such that a
    lists of tests can be saved in memory.
  */
  export struct generic_test_entry {
    virtual std::string_view name() const = 0;
    virtual test_result run_test() const = 0;
    virtual ~generic_test_entry() = default;
  };

  /*
    Creates a test configuration that will run a test based on a lambda. This includes exceptions,
    including custom ones, that can be caught by the runner.
  */
  export template <std::invocable F, is_exception... exceptions>
  struct test_entry : public generic_test_entry {
  public:
    std::string_view name() const override {
      return __name;
    }
    constexpr test_entry(
      F &&f,
      std::string_view test_name = get_type_name<F>(),
      const exception_pack<exceptions...> &p = exception_pack<>{}
    ) : __f{f}, __name{test_name} {}
    test_result run_test() const override {
      auto beg = std::chrono::system_clock::now();
      auto res =
        exception_catcher<exceptions..., fail_assertion, std::runtime_error, std::exception>::make()
          .safely_run_invocable(__f);
      auto end = std::chrono::system_clock::now();
      auto dur = end - beg;
      return res.transform_error(
                  [&](auto &&e) { return test_result{dur, std::move(e)}; }
      ).error_or(test_result{dur});
    }

  private:
    F __f;
    std::string __name;
  };

  export template <std::invocable F, is_exception... exceptions>
  constexpr std::unique_ptr<generic_test_entry> make_test_entry(
    F &&f,
    std::string_view test_name = get_type_name<F>(),
    const exception_pack<exceptions...> &p = exception_pack<>{}
  ) {
    return std::make_unique<test_entry<F, exceptions...>>(std::forward<F>(f), test_name, p);
  }

  export template <std::invocable F>
  constexpr std::unique_ptr<generic_test_entry> make_test_entry(
    F &&f, std::string_view test_name = get_type_name<F>()
  ) {
    return std::make_unique<test_entry<F>>(std::forward<F>(f), test_name);
  }
}