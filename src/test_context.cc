module;
#include <array>
#include <chrono>
#include <functional>
export module jowi.test_lib:TestContext;
import :TestSuite;

namespace jowi::test_lib {

  export enum struct TestTimeUnit { MICRO_SECONDS, MILLI_SECONDS, SECONDS };

  export struct TestContext {
    TestContext() : __setup{[](int argc, const char **argv) {}}, __teardown{[]() {}} {}
    int thread_count = 1;
    TestSuite tests = TestSuite{};
    TestTimeUnit time_unit = TestTimeUnit::MICRO_SECONDS;

    TestContext &add_setup(std::invocable<int, const char **> auto &&f) {
      __setup = f;
      return *this;
    }
    TestContext &add_teardown(std::invocable<> auto &&f) {
      __teardown = f;
      return *this;
    }
    TestContext &set_time_unit(TestTimeUnit unit) {
      time_unit = unit;
      return *this;
    }
    TestContext &set_thread_count(int thread_count) {
      this->thread_count = thread_count;
      return *this;
    }
    std::string get_time(const std::chrono::system_clock::duration &dur) const {
      auto converted_dur =
        static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count());
      if (time_unit == TestTimeUnit::MICRO_SECONDS) {
        return std::format("{:.2f} μs", converted_dur / 1e3);
      } else if (time_unit == TestTimeUnit::MILLI_SECONDS) {
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

  auto ctx = TestContext{};

  export TestContext &get_test_context() {
    return ctx;
  }
}