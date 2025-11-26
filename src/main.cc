#include <algorithm>
#include <expected>
#include <format>
#include <optional>
#include <print>
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
    std::print(
      "{}",
      tui::Layout{}
        .append_child(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(tui::RgbColor::bright_blue()))
            .append_child(tui::Paragraph{"[{:3}]", i}.no_newline())
        )
        .append_child(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(tui::RgbColor::bright_green()))
            .append_child(tui::Paragraph{"[{:4}]", "OK!"}.no_newline())
        )
        .append_child(tui::Paragraph{"{} ({})", name, ctx.get_time(res.running_time())})
    );
  } else {
    std::print(
      "{}",
      tui::Layout{}
        .append_child(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(tui::RgbColor::bright_blue()))
            .append_child(tui::Paragraph{"[{:3}]", i}.no_newline())
        )
        .append_child(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(tui::RgbColor::bright_red()))
            .append_child(tui::Paragraph{"[{:4}]", "ERR!"}.no_newline())
        )
        .append_child(tui::Paragraph{"{} ({})", name, ctx.get_time(res.running_time())})
        .append_child(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(tui::RgbColor::bright_yellow()))
            .append_child(tui::Paragraph{"{:=<80}, "})
        )
        .append_child(tui::Paragraph{})
        .append_child(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(tui::RgbColor::bright_red()))
            .append_child(tui::Paragraph{"{}", res.get_error().value().name})
        )
        .append_child(tui::Paragraph{})
        .append_child(tui::Paragraph{})
        .append_child(tui::Paragraph{"{}", res.get_error().value().message})
        .append_child(tui::Paragraph{})
        .append_child(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(tui::RgbColor::bright_yellow()))
            .append_child(tui::Paragraph{"{:=<80}, "})
        )
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
    std::print("{}", tui::Paragraph{"Found {} tests: ", ctx.tests.size()});
    for (const auto &t : ctx.tests) {
      std::print(
        "{}",
        tui::Layout{}
          .append_child(
            tui::Layout{}
              .style(tui::DomStyle{}.fg(tui::RgbColor::bright_blue()))
              .append_child(tui::Paragraph{"[{:3}]", i}.no_newline())
          )
          .append_child(tui::Paragraph{" {}", t->name()})
      );
      i += 1;
    }
    return 0;
  }

  std::print(
    "{}",
    tui::Layout{}
      .style(tui::DomStyle{}.fg(tui::RgbColor::bright_cyan()))
      .append_child(tui::Paragraph{"{:=<80}", ""})
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
      std::print(
        "{}",
        tui::Layout{}
          .append_child(
            tui::Layout{}
              .style(tui::DomStyle{}.fg(tui::RgbColor::bright_blue()))
              .append_child(tui::Paragraph{"[{:3}]", i}.no_newline())
          )
          .append_child(
            tui::Layout{}
              .style(tui::DomStyle{}.fg(tui::RgbColor::bright_yellow()))
              .append_child(tui::Paragraph{"[{:4}]", "OK!"}.no_newline())
          )
          .append_child(tui::Paragraph{"{}", test.get()->name()})
      );
    }
    i += 1;
  }
  ctx.tear_down();
  /*
    Print Statistics
  */
  std::print(
    "{}",
    tui::DomNode::vstack(
      tui::Layout{}
        .append_child(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(tui::RgbColor::bright_cyan()))
            .append_child(tui::Paragraph{"{:=<80}", ""})
        )
        .append_child(tui::Paragraph{"Ran {:3} tests", succ_count + err_count})
        .append_child(
          tui::Layout{}
            .append_child(
              tui::Layout{}
                .style(tui::DomStyle{}.fg(tui::RgbColor::bright_green()))
                .append_child(tui::Paragraph{"[{:4}]", "OK!"}.no_newline())
            )
            .append_child(tui::Paragraph{" {:3} tests", succ_count})
        )
        .append_child(
          tui::Layout{}
            .append_child(
              tui::Layout{}
                .style(tui::DomStyle{}.fg(tui::RgbColor::bright_red()))
                .append_child(tui::Paragraph{"[{:4}]", "ERR!"}.no_newline())
            )
            .append_child(tui::Paragraph{" {:3} tests", err_count})
        )
        .append_child(
          tui::Layout{}
            .append_child(
              tui::Layout{}
                .style(tui::DomStyle{}.fg(tui::RgbColor::bright_yellow()))
                .append_child(tui::Paragraph{"[{:4}]", "EXC!"}.no_newline())
            )
            .append_child(tui::Paragraph{" {:3} tests", i - succ_count - err_count})
        )
    )
  );
  return err_count;
}
