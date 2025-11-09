#define JOWI_ADD_TEST(name) \
  struct name { \
    void operator()() const; \
  }; \
  struct name##_initiator { \
    name##_initiator() { \
      jowi::test_lib::get_test_context().tests.add_test(name{}); \
    } \
  }; \
  static name##_initiator name##_var{}; \
  void name::operator()() const

#define JOWI_SETUP(argc, argv) \
  template <> struct jowi::test_lib::TestSetup<jowi::test_lib::SetupMode::SET_UP> { \
    TestSetup() { \
      jowi::test_lib::get_test_context().add_setup(set_up); \
    } \
    static void set_up(int argc, const char **argv); \
  }; \
  static jowi::test_lib::TestSetup<jowi::test_lib::SetupMode::SET_UP> __moderna_test_lib_setup; \
  void jowi::test_lib::TestSetup<jowi::test_lib::SetupMode::SET_UP>::set_up( \
    int argc, const char **argv \
  )

#define JOWI_TEARDOWN() \
  template <> struct jowi::test_lib::TestSetup<jowi::test_lib::SetupMode::TEAR_DOWN> { \
    TestSetup() { \
      jowi::test_lib::get_test_context().add_teardown(tear_down); \
    } \
    static void tear_down(); \
  }; \
  static jowi::test_lib::TestSetup<jowi::test_lib::SetupMode::TEAR_DOWN> \
    __moderna_test_lib_teardown; \
  void jowi::test_lib::TestSetup<jowi::test_lib::SetupMode::TEAR_DOWN>::tear_down()
