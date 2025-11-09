#include <algorithm>
#include <expected>
#include <format>
#include <optional>
#include <string>
import jowi.test_lib;
import jowi.cli;
import jowi.tui;

namespace test_lib = jowi::test_lib;
namespace cli = jowi::cli;
namespace tui = jowi::tui;

auto app_id = cli::AppIdentity{
  .name = "Jowi Test Utility",
  .description = "Command Line Test Program for a set of predefined tests",
  .version = cli::AppVersion{1, 0, 0}
};

struct FilterExcludeValidator {
  std::expected<void, cli::ParseError> post_validate(
    std::optional<std::reference_wrapper<const cli::ArgKey>> k, cli::ParsedArg &args
  ) const {
    if (args.contains("--filter") && args.contains("--exclude")) {
      return std::unexpected{cli::ParseError{
        cli::ParseErrorType::TOO_MANY_VALUE_GIVEN,
        "only one of '--filter' or '--exclude' can be given"
      }};
    }
    return {};
  }
};

struct ValidTestNameValidator {
  std::reference_wrapper<const test_lib::TestSuite> tests;

  std::expected<void, cli::ParseError> validate(std::optional<std::string_view> v) const {
    if (!v) {
      return std::unexpected{cli::ParseError{cli::ParseErrorType::NO_VALUE_GIVEN, ""}};
    }
    if (!tests.get().get(v.value())) {
      return std::unexpected{cli::ParseError{
        cli::ParseErrorType::INVALID_VALUE,
        "'{}' is not a valid test name. Use --list for the full list of tests",
        v.value()
      }};
    }
    return {};
  }
};

void print_test_output(
  cli::App &app,
  std::string_view name,
  size_t i,
  test_lib::TestResult &res,
  test_lib::TestContext &ctx
) {
  if (res.is_ok()) {
    app.out(
      "{} {} {} ({})",
      tui::CliNodes{
        tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_blue())),
        tui::CliNode::text("[{:3}]", i),
        tui::CliNode::format_end()
      },
      tui::CliNodes{
        tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_green())),
        tui::CliNode::text("[{:4}]", "OK!"),
        tui::CliNode::format_end()
      },
      name,
      ctx.get_time(res.running_time())
    );
  } else {
    app.out(
      "{} {} {} ({})\n{}",
      tui::CliNodes{
        tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_blue())),
        tui::CliNode::text("[{:3}]", i),
        tui::CliNode::format_end()
      },
      tui::CliNodes{
        tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_red())),
        tui::CliNode::text("[{:4}]", "ERR!"),
        tui::CliNode::format_end()
      },
      name,
      ctx.get_time(res.running_time()),
      tui::CliNodes{
        tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_yellow())),
        tui::CliNode::text("{:=<80}", ""),
        tui::CliNode::format_end(),
        tui::CliNode::new_line(),
        tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_red())),
        tui::CliNode::text("{}", res.get_error().value().name),
        tui::CliNode::format_end(),
        tui::CliNode::new_line(),
        tui::CliNode::new_line(),
        tui::CliNode::text("{}", res.get_error().value().message),
        tui::CliNode::new_line(),
        tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_yellow())),
        tui::CliNode::text("{:=<80}", ""),
        tui::CliNode::format_end()
      }
    );
  }
}

bool should_run_test(std::string_view name, cli::App &app) {
  if (app.args().contains("--filter")) {
    auto ic = app.args().filter("--filter");
    return std::ranges::find(ic, name) != ic.end();
  } else {
    auto ex = app.args().filter("--exclude");
    return std::ranges::find(ex, name) == ex.end();
  }
}

int main(int argc, const char **argv) {
  auto &ctx = test_lib::get_test_context();
  auto app = cli::App{app_id, argc, argv};
  /*
    Arguments
  */
  app.add_argument("--filter")
    .help("A list of tests to run. This argument can be given multiple times")
    .require_value()
    .n_at_least(0)
    .add_validator(FilterExcludeValidator{})
    .add_validator(ValidTestNameValidator{ctx.tests});
  app.add_argument("--exclude")
    .help("A list of tests to exclude. This argument can be given multiple times")
    .require_value()
    .n_at_least(0)
    .add_validator(FilterExcludeValidator{})
    .add_validator(ValidTestNameValidator{ctx.tests});
  app.add_argument("--list")
    .help("Lists all the available tests, this will ignore all previous arguments")
    .as_flag()
    .optional();
  app.parse_args();

  /*
    Don't run tests but list all tests
  */
  if (app.args().contains("--list")) {
    uint64_t i = 0;
    app.out("Found {} tests: ", ctx.tests.size());
    for (const auto &t : ctx.tests) {
      app.out(
        "{} {}",
        tui::CliNodes{
          tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_blue())),
          tui::CliNode::text("[{:3}]", i),
          tui::CliNode::format_end()
        },
        t->name()
      );
      i += 1;
    }
    return 0;
  }

  app.out(
    "{}",
    tui::CliNodes{
      tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_cyan())),
      tui::CliNode::text("{:=<80}", ""),
      tui::CliNode::format_end()
    }
  );
  /*
    Run tests based on --filter and --exclude. When both are given --filter will be applied.
  */
  ctx.setup(argc, argv);
  /*
    Run every tests
  */
  uint64_t i = 0;
  uint64_t succ_count = 0;
  uint64_t err_count = 0;
  for (const auto &test : ctx.tests) {
    if (should_run_test(test->name(), app)) {
      auto res = test->run_test();
      if (res.is_ok()) {
        succ_count += 1;
      } else {
        err_count += 1;
      }
      print_test_output(app, test.get()->name(), i, res, ctx);
    } else {
      app.out(
        "{} {} {}",
        tui::CliNodes{
          tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_blue())),
          tui::CliNode::text("[{:3}]", i),
          tui::CliNode::format_end()
        },
        tui::CliNodes{
          tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_yellow())),
          tui::CliNode::text("[{:4}]", "EXC!"),
          tui::CliNode::format_end()
        },
        test->name()
      );
    }
    i += 1;
  }
  ctx.tear_down();
  /*
    Print Statistics
  */
  app.out(
    "{}",
    tui::CliNodes{
      tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_cyan())),
      tui::CliNode::text("{:=<80}", ""),
      tui::CliNode::format_end()
    }
  );
  app.out("Ran {:3} tests", succ_count + err_count);
  app.out(
    "{} {:3} tests",
    tui::CliNodes{
      tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_green())),
      tui::CliNode::text("[{:4}]", "OK!"),
      tui::CliNode::format_end()
    },
    succ_count
  );
  app.out(
    "{} {:3} tests",
    tui::CliNodes{
      tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_red())),
      tui::CliNode::text("[{:4}]", "ERR!"),
      tui::CliNode::format_end()
    },
    err_count
  );
  app.out(
    "{} {:3} tests",
    tui::CliNodes{
      tui::CliNode::format_begin(tui::TextFormat{}.fg(tui::Color::bright_yellow())),
      tui::CliNode::text("[{:4}]", "EXC!"),
      tui::CliNode::format_end()
    },
    i - succ_count - err_count
  );
  return err_count;
}
