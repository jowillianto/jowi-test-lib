module;
#include <exception>
#include <expected>
#include <string>
export module jowi.test_lib:exception;
import :reflection;

namespace jowi::test_lib {
  template <class exception_t>
  concept is_exception = requires(const exception_t &e) {
    { e.what() } -> std::convertible_to<std::string>;
  };

  export class fail_assertion : public std::exception {
    std::string _message;

  public:
    fail_assertion(std::string message) : _message(std::move(message)) {}
    const char *what() const noexcept {
      return _message.c_str();
    }
    std::string_view msg() const {
      return _message;
    }
  };

  template <class value_t, class... values>
  concept is_in_values = (std::same_as<value_t, values> || ...);
  template <class value_t, class... values>
  concept is_parent_of =
    ((std::derived_from<values, value_t> && !std::same_as<value_t, values>) || ...);

  export struct exception_info {
    std::string name;
    std::string message;

    exception_info(const is_exception auto &e) :
      name{std::string{get_type_name<std::decay_t<decltype(e)>>()}}, message{e.what()} {}
  };

  export template <is_exception... exceptions> struct exception_catcher;

  /*
    An exception catcher that will catch exceptions in the following order :
    from the last element in the argument pack to the first element in the argument pack.
  */
  template <is_exception current_exception_t, is_exception... exceptions>
  struct exception_catcher<current_exception_t, exceptions...> {
    template <std::invocable F>
    std::expected<std::invoke_result_t<F>, exception_info> safely_run_invocable(F &&f) {
      try {
        if constexpr (sizeof...(exceptions) == 0) {
          if constexpr (std::same_as<std::invoke_result_t<F>, void>) {
            std::invoke(std::forward<F>(f));
            return {};
          } else {
            return std::invoke(std::forward<F>(f));
          }
        } else {
          return exception_catcher<exceptions...>{}.safely_run_invocable(f);
        }
      } catch (const current_exception_t &e) {
        return std::unexpected{exception_info{e}};
      }
    }

    /*
      Creates an exception_catcher, sanitising the argument pack. This also reorders the argument
      pack so that when the exception_catcher is used, the exceptions are caught in the correct
      order. This includes problem such as catching subclasses of an exception throught the parent
      class.
    */
    template <is_exception... pack_exceptions> static consteval auto make() {
      if constexpr (is_parent_of<current_exception_t, exceptions...>) {
        return exception_catcher<exceptions..., current_exception_t>::template make<
          pack_exceptions...>();
      } else if constexpr (is_in_values<current_exception_t, exceptions...>) {
        return exception_catcher<exceptions...>::template make<pack_exceptions...>();
      } else {
        if constexpr (sizeof...(exceptions) == 0) {
          return exception_catcher<current_exception_t, pack_exceptions...>{};
        } else {
          return exception_catcher<
            exceptions...>::template make<current_exception_t, pack_exceptions...>();
        }
      }
    }
  };

  template <is_exception... exceptions> struct exception_pack {};
}