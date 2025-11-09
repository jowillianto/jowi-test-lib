module;
#include <concepts>
export module jowi.test_lib;
export import :randomizer;
export import :exception;
export import :assert;
export import :TestSuite;
export import :TestEntry;
export import :TestContext;

namespace jowi::test_lib {
  export enum struct SetupMode { SET_UP, TEAR_DOWN };

  export template <SetupMode mode> struct TestSetup {
    static void set_up(int argc, const char **argv) {}
    static void tear_down() {}
  };
}