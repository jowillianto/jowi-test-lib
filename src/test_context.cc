module;
#include <array>
#include <chrono>
#include <functional>
export module jowi.test_lib:test_context;
import :test_suite;

namespace jowi::test_lib {

  export enum struct test_time_unit { MICRO_SECONDS, MILLI_SECONDS, SECONDS };

  export struct test_context {
    test_context() : __setup{[](int argc, const char **argv) {}}, __teardown{[]() {}} {}
    int thread_count = 1;
    test_suite tests = test_suite{};
    test_time_unit time_unit = test_time_unit::MICRO_SECONDS;

    test_context &add_setup(std::invocable<int, const char **> auto &&f) {
      __setup = f;
      return *this;
    }
    test_context &add_teardown(std::invocable<> auto &&f) {
      __teardown = f;
      return *this;
    }
    test_context &set_time_unit(test_time_unit unit) {
      time_unit = unit;
      return *this;
    }
    test_context &set_thread_count(int thread_count) {
      this->thread_count = thread_count;
      return *this;
    }
    std::string get_time(const std::chrono::system_clock::duration &dur) const {
      auto converted_dur =
        static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count());
      if (time_unit == test_time_unit::MICRO_SECONDS) {
        return std::format("{:.2f} μs", converted_dur / 1e3);
      } else if (time_unit == test_time_unit::MILLI_SECONDS) {
        return std::format("{:.2f} ms", converted_dur / 1e6);
      } else {
        return std::format("{:.2f} s", converted_dur / 1e9);
      }
    }
    void setup(int argc, const char **argv) const {
      __setup(argc, argv);
    }
    void tear_down() const {
      __teardown();
    }

  private:
    std::function<void(int, const char **)> __setup;
    std::function<void()> __teardown;
  };

  auto ctx = test_context{};

  export test_context &get_test_context() {
    return ctx;
  }
}