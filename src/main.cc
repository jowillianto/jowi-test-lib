#include <algorithm>
#include <expected>
#include <format>
#include <optional>
#include <string>
import jowi.test_lib;
import jowi.cli;
import jowi.cli.ui;

namespace test_lib = jowi::test_lib;
namespace cli = jowi::cli;
namespace ui = jowi::cli::ui;

auto app_id = cli::app_identity{
  .name = "Jowi Test Utility",
  .description = "Command Line Test Program for a set of predefined tests",
  .version = cli::app_version{1, 0, 0}
};

struct filter_exclude_validator {
  std::expected<void, cli::parse_error> post_validate(
    std::optional<std::reference_wrapper<const cli::arg_key>> k, cli::parsed_arg &args
  ) const {
    if (args.contains("--filter") && args.contains("--exclude")) {
      return std::unexpected{cli::parse_error{
        cli::parse_error_type::TOO_MANY_VALUE_GIVEN,
        "only one of '--filter' or '--exclude' can be given"
      }};
    }
    return {};
  }
};

struct valid_test_name_validator {
  std::reference_wrapper<const test_lib::test_suite> tests;

  std::expected<void, cli::parse_error> validate(std::optional<std::string_view> v) const {
    if (!v) {
      return std::unexpected{cli::parse_error{cli::parse_error_type::NO_VALUE_GIVEN, ""}};
    }
    if (!tests.get().get(v.value())) {
      return std::unexpected{cli::parse_error{
        cli::parse_error_type::INVALID_VALUE,
        "'{}' is not a valid test name. Use --list for the full list of tests",
        v.value()
      }};
    }
    return {};
  }
};

void print_test_output(cli::app &app, std::string_view name, size_t i, test_lib::test_result &res) {
  if (res.is_ok()) {
    app.out(
      "{} {} {} ({})",
      ui::cli_nodes{
        ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_blue())),
        ui::cli_node::text("[{:3}]", i),
        ui::cli_node::format_end()
      },
      ui::cli_nodes{
        ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_green())),
        ui::cli_node::text("[{:4}]", "OK!"),
        ui::cli_node::format_end()
      },
      name,
      res.running_time()
    );
  } else {
    app.out(
      "{} {} {} ({})\n{}",
      ui::cli_nodes{
        ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_blue())),
        ui::cli_node::text("[{:3}]", i),
        ui::cli_node::format_end()
      },
      ui::cli_nodes{
        ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_red())),
        ui::cli_node::text("[{:4}]", "ERR!"),
        ui::cli_node::format_end()
      },
      name,
      res.running_time(),
      ui::cli_nodes{
        ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_yellow())),
        ui::cli_node::text("{:=<80}", ""),
        ui::cli_node::format_end(),
        ui::cli_node::new_line(),
        ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_red())),
        ui::cli_node::text("{}", res.get_error().value().name),
        ui::cli_node::format_end(),
        ui::cli_node::new_line(),
        ui::cli_node::new_line(),
        ui::cli_node::text("{}", res.get_error().value().message),
        ui::cli_node::new_line(),
        ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_yellow())),
        ui::cli_node::text("{:=<80}", ""),
        ui::cli_node::format_end()
      }
    );
  }
}

bool should_run_test(std::string_view name, cli::app &app) {
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
  auto app = cli::app{app_id, argc, argv};
  /*
    Arguments
  */
  app.add_argument("--filter")
    .help("A list of tests to run. This argument can be given multiple times")
    .require_value()
    .n_at_least(0)
    .add_validator(filter_exclude_validator{})
    .add_validator(valid_test_name_validator{ctx.tests});
  app.add_argument("--exclude")
    .help("A list of tests to exclude. This argument can be given multiple times")
    .require_value()
    .n_at_least(0)
    .add_validator(filter_exclude_validator{})
    .add_validator(valid_test_name_validator{ctx.tests});
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
        ui::cli_nodes{
          ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_blue())),
          ui::cli_node::text("[{:3}]", i),
          ui::cli_node::format_end()
        },
        t->name()
      );
      i += 1;
    }
    return 0;
  }

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
      print_test_output(app, test.get()->name(), i, res);
    } else {
      app.out(
        "{} {} {}",
        ui::cli_nodes{
          ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_blue())),
          ui::cli_node::text("[{:3}]", i),
          ui::cli_node::format_end()
        },
        ui::cli_nodes{
          ui::cli_node::format_begin(ui::text_format{}.fg(ui::color::bright_yellow())),
          ui::cli_node::text("[{:4}]", "EXC!"),
          ui::cli_node::format_end()
        },
        test->name()
      );
    }
    i += 1;
  }
  ctx.tear_down();
  return err_count;
}