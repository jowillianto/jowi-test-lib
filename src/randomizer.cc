module;
#include <random>
#include <ranges>
#include <string>

export module jowi.test_lib:randomizer;

namespace jowi::test_lib {

  /*
    Hard coded string for randomness
  */

  /*
    All readable ascii letters.
  */
  export constexpr auto ascii_letters = std::string_view{
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~`!@#$%^&*()-=_+{}[]\\|:;\"\'<>,.?/"
  };
  /*
    All readable ascii lowercase letters
  */
  export constexpr auto ascii_lowercase =
    std::string_view{ascii_letters.begin(), ascii_letters.begin() + 26};
  /*
    All readable ascii uppercase letters
  */
  export constexpr auto ascii_uppercase =
    std::string_view{ascii_letters.begin() + 26, ascii_letters.begin() + 52};
  /*
    All readable ascii numbers.
  */
  export constexpr auto ascii_numbers =
    std::string_view{ascii_letters.begin() + 52, ascii_letters.begin() + 62};

  /*
    This creates an internal random number Generator.
  */
  export struct Generator {
    mutable std::random_device rd;
    mutable std::mt19937 gen;

    Generator() : rd{}, gen{rd()} {}
  };
  /*
    Random Algorithms using the Generator defined above.
  */

  /*
    Randomly picks an element bounded by the iterators begin, end. Returns a reference to the
    element specified by the iterator bounds.
  */
  export template <std::ranges::forward_range T>
    requires(std::convertible_to<std::ranges::range_size_t<T>, size_t>)
  const std::ranges::range_value_t<T> &random_pick(
    const T &container, const Generator &gen = Generator{}
  ) {
    const auto begin = std::ranges::begin(container);
    const auto end = std::ranges::end(container);
    const auto dist = std::ranges::size(container);
    std::uniform_int_distribution<std::ranges::range_size_t<T>> distrib{0, dist - 1};
    const size_t picked_dist = distrib(gen.gen);
    return *(begin + picked_dist);
  }

  export std::string random_string(
    size_t length, std::string_view choices = ascii_lowercase, const Generator &gen = Generator{}
  ) {
    std::string generated_str;
    generated_str.reserve(length);
    for (size_t i = 0; i < length; i += 1)
      generated_str.push_back(random_pick(choices));
    return generated_str;
  }
  export template <std::integral T> T random_integer(T a, T b, const Generator &gen = Generator{}) {
    std::uniform_int_distribution<T> distribution{a, b};
    return distribution(gen.gen);
  }
  export template <std::floating_point T>
  T random_real(T a, T b, const Generator &gen = Generator{}) {
    std::uniform_real_distribution<T> distribution{a, b};
    return distribution(gen.gen);
  }
}