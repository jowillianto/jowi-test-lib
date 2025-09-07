module;
#include <source_location>
#include <string_view>
export module jowi.test_lib:reflection;

namespace jowi::test_lib {
  /*
    This function is copied from
    https://github.com/getml/reflect-cpp

    include/internal/get_type_name.hpp
  */
  template <class T> consteval auto get_type_name() {
#if __cpp_lib_source_location >= 201907L
    const auto func_name = std::string_view{std::source_location::current().function_name()};
#elif defined(_MSC_VER)
    // Officially, we only support MSVC versions that are modern enough to contain
    // <source_location>, but inofficially, this might work.
    const auto func_name = std::string_view{__FUNCSIG__};
#else
    const auto func_name = std::string_view{__PRETTY_FUNCTION__};
#endif
#if defined(__clang__)
    const auto split = func_name.substr(0, func_name.size() - 1);
    return split.substr(split.find("T = ") + 4);
#elif defined(__GNUC__)
    const auto split = func_name.substr(0, func_name.size() - 1);
    return split.substr(split.find("T = ") + 4);
#elif defined(_MSC_VER)
    auto split = func_name.substr(0, func_name.size() - 7);
    split = split.substr(split.find("get_type_name_str_view<") + 23);
    auto pos = split.find(" ");
    if (pos != std::string_view::npos) {
      return split.substr(pos + 1);
    }
    return split;
#else
    static_assert(
      false,
      "You are using an unsupported compiler. Please use GCC, Clang "
      "or MSVC or explicitly tag your structs using 'Tag' or 'Name'."
    );
#endif
  }
};