module;
#include <concepts>
#include <memory>
#include <optional>
#include <vector>
export module jowi.test_lib:test_suite;
import :test_entry;
import :reflection;

namespace jowi::test_lib {
  struct test_suite {
    test_suite(std::string name = "") : __tests{}, __name{std::move(name)} {}
    test_suite &rename(std::string new_name) {
      __name = std::move(new_name);
      return *this;
    }
    template <std::invocable F, is_exception... exceptions>
    test_suite &add_test(
      F &&f,
      std::string_view test_name = get_type_name<F>(),
      exception_pack<exceptions...> p = exception_pack<>{}
    ) {
      __tests.emplace_back(make_test_entry(std::forward<F>(f), test_name, p));
      return *this;
    }

    void done() {}

    std::optional<std::reference_wrapper<const generic_test_entry>> get(size_t id) const {
      if (id < __tests.size()) {
        return std::cref(*__tests[id]);
      }
      return std::nullopt;
    }

    auto begin() const {
      return __tests.begin();
    }
    auto end() const {
      return __tests.end();
    }
    size_t size() const {
      return __tests.size();
    }

  private:
    std::string __name;
    std::vector<std::unique_ptr<generic_test_entry>> __tests;
  };

  // /*
  //   This is the singleton object that will be used to run tests.
  // */
  // export test_suite global_test_suite{};

  // /*
  //   These functions can be used to manipulate the test suite
  // */
  // export template <std::invocable F, is_exception... exceptions>
  // test_suite &add_test(
  //   F &&f,
  //   std::string_view test_name = get_type_name<F>(),
  //   exception_pack<exceptions...> p = exception_pack<>{},
  //   test_suite &suite = global_test_suite
  // ) {
  //   suite.add_test(std::forward<F>(f), test_name, p);
  //   return suite;
  // }

  // export test_suite &get_global_test() {
  //   return global_test_suite;
  // }
}