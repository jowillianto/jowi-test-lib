module;
#include <chrono>
#include <concepts>
#include <expected>
#include <memory>
#include <optional>
#include <string_view>
export module jowi.test_lib:TestEntry;
import :exception;
import :reflection;

namespace jowi::test_lib {
  /*
    A structure that contains the test result.
  */
  export struct TestResult {
  public:
    TestResult(
      std::chrono::system_clock::duration dur, std::optional<ExceptionInfo> err = std::nullopt
    ) : __runtime{dur}, __err{std::move(err)} {}

    std::chrono::system_clock::duration running_time() const {
      return __runtime;
    }

    std::optional<ExceptionInfo> get_error() const {
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
    std::optional<ExceptionInfo> __err;
  };

  /*
    The base class for all tests. This allows a class to basically be saved in a vector such that a
    lists of tests can be saved in memory.
  */
  export struct GenericTestEntry {
    virtual std::string_view name() const = 0;
    virtual TestResult run_test() const = 0;
    virtual ~GenericTestEntry() = default;
  };

  /*
    Creates a test configuration that will run a test based on a lambda. This includes exceptions,
    including custom ones, that can be caught by the runner.
  */
  export template <std::invocable F, is_exception... exceptions>
  struct TestEntry : public GenericTestEntry {
  public:
    std::string_view name() const override {
      return __name;
    }
    constexpr TestEntry(
      F &&f,
      std::string_view test_name = get_type_name<F>(),
      const ExceptionPack<exceptions...> &p = ExceptionPack<>{}
    ) : __f{f}, __name{test_name} {}
    TestResult run_test() const override {
      auto beg = std::chrono::system_clock::now();
      auto res =
        ExceptionCatcher<exceptions..., FailAssertion, std::runtime_error, std::exception>::make()
          .safely_run_invocable(__f);
      auto end = std::chrono::system_clock::now();
      auto dur = end - beg;
      return res.transform_error(
                  [&](auto &&e) { return TestResult{dur, std::move(e)}; }
      ).error_or(TestResult{dur});
    }

  private:
    F __f;
    std::string __name;
  };

  export template <std::invocable F, is_exception... exceptions>
  constexpr std::unique_ptr<GenericTestEntry> make_test_entry(
    F &&f,
    std::string_view test_name = get_type_name<F>(),
    const ExceptionPack<exceptions...> &p = ExceptionPack<>{}
  ) {
    return std::make_unique<TestEntry<F, exceptions...>>(std::forward<F>(f), test_name, p);
  }

  export template <std::invocable F>
  constexpr std::unique_ptr<GenericTestEntry> make_test_entry(
    F &&f, std::string_view test_name = get_type_name<F>()
  ) {
    return std::make_unique<TestEntry<F>>(std::forward<F>(f), test_name);
  }
}