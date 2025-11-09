module;
#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
export module jowi.test_lib:TestSuite;
import :TestEntry;
import :reflection;

namespace jowi::test_lib {
  export struct TestSuite {
  private:
    std::vector<std::unique_ptr<GenericTestEntry>> __tests;

  public:
    TestSuite() {}
    template <std::invocable F, is_exception... exceptions>
    TestSuite &add_test(
      F &&f,
      std::string_view test_name = get_type_name<F>(),
      ExceptionPack<exceptions...> p = ExceptionPack<>{}
    ) {
      __tests.emplace_back(make_test_entry(std::forward<F>(f), test_name, p));
      return *this;
    }

    std::optional<std::reference_wrapper<const GenericTestEntry>> get(size_t id) const {
      if (id < __tests.size()) {
        return std::cref(*__tests[id]);
      }
      return std::nullopt;
    }

    std::optional<std::reference_wrapper<const GenericTestEntry>> get(
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