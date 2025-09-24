module;
#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
export module jowi.test_lib:test_suite;
import :test_entry;
import :reflection;

namespace jowi::test_lib {
  export struct test_suite {
  private:
    std::vector<std::unique_ptr<generic_test_entry>> __tests;

  public:
    test_suite() {}
    template <std::invocable F, is_exception... exceptions>
    test_suite &add_test(
      F &&f,
      std::string_view test_name = get_type_name<F>(),
      exception_pack<exceptions...> p = exception_pack<>{}
    ) {
      __tests.emplace_back(make_test_entry(std::forward<F>(f), test_name, p));
      return *this;
    }

    std::optional<std::reference_wrapper<const generic_test_entry>> get(size_t id) const {
      if (id < __tests.size()) {
        return std::cref(*__tests[id]);
      }
      return std::nullopt;
    }

    std::optional<std::reference_wrapper<const generic_test_entry>> get(
      std::string_view name
    ) const {
      auto it =
        std::ranges::find_if(__tests, [&](const auto &test) { return test->name() == name; });
      if (it != __tests.end()) {
        return std::cref(**it);
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
  };
}