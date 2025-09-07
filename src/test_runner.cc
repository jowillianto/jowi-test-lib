module;
#include <algorithm>
#include <atomic>
#include <chrono>
#include <expected>
#include <optional>
#include <print>
#include <ranges>
#include <thread>
#include <vector>
export module jowi.test_lib:test_runner;
import :test_suite;
import :test_entry;
import :test_context;
import :exception;

namespace jowi::test_lib {
  struct multithreaded_runner {
    multithreaded_runner(size_t pool_size) : __pool_size{pool_size} {}

    std::vector<test_result> run_test_suite(const test_suite &suite) const {
      std::atomic<size_t> cur_id;
      std::atomic<bool> is_init{false};
      std::vector<std::thread> thread_pool;
      std::vector<std::optional<test_result>> opt_results{suite.size(), std::nullopt};
      thread_pool.reserve(__pool_size);
      for (size_t i = 0; i < __pool_size; i += 1) {
        thread_pool.emplace_back(std::thread{[&]() {
          is_init.wait(false, std::memory_order_acquire);
          size_t test_id = cur_id.fetch_add(1, std::memory_order_relaxed);
          while (test_id < suite.size()) {
            auto test_entry = suite.get(test_id).value();
            auto res = test_entry.get().run_test();
            opt_results[test_id] = res;
            test_id = cur_id.fetch_add(1, std::memory_order_relaxed);
          }
        }});
      }
      is_init.store(true, std::memory_order_release);
      is_init.notify_all();
      for (size_t i = 0; i < __pool_size; i += 1) {
        thread_pool[i].join();
      }
      std::vector<test_result> results;
      results.reserve(suite.size());
      std::ranges::transform(opt_results, std::back_inserter(results), [](const auto &opt) {
        return opt.value();
      });
      return results;
    }

  private:
    size_t __pool_size;
  };

  struct sequential_runner {
    std::vector<test_result> run_test_suite(const test_suite &suite) const {
      std::vector<test_result> results;
      results.reserve(suite.size());
      for (const auto &test : suite) {
        results.push_back(test->run_test());
      }
      return results;
    }
  };

  struct test_runner {
    std::optional<size_t> thread_count;
    test_runner &set_thread_count(size_t count) {
      thread_count = count;
      return *this;
    }
    test_runner &unset_thread_count() {
      thread_count = std::nullopt;
      return *this;
    }
    std::optional<size_t> get_thread_count() const {
      return thread_count;
    }
    std::vector<test_result> run_test_suite(const test_suite &suite) const {
      size_t t_count = thread_count.value_or(1);
      if (t_count == 1) {
        auto runner = sequential_runner{};
        return runner.run_test_suite(suite);
      } else {
        auto runner = multithreaded_runner{t_count};
        return runner.run_test_suite(suite);
      }
    }
  };

  export std::vector<test_result> run_tests(
    const test_suite &suite = get_test_context().tests,
    size_t thread_count = get_test_context().thread_count
  ) {
    if (thread_count == 1) {
      return sequential_runner{}.run_test_suite(suite);
    } else {
      return multithreaded_runner{thread_count}.run_test_suite(suite);
    }
  }

  export std::expected<std::pair<size_t, size_t>, fail_assertion> print_test_result(
    const std::vector<test_result> &vec, const test_suite &suite
  ) {
    if (vec.size() != suite.size()) {
      return std::unexpected{fail_assertion{"Test suite and test result vector size mismatch"}};
    }
    size_t i = 1;
    size_t ok_count =
      std::ranges::count_if(vec, [](const test_result &res) { return res.is_ok(); });
    size_t err_count =
      std::ranges::count_if(vec, [](const test_result &res) { return res.is_error(); });
    std::println("Run {} tests", suite.size());
    std::println("OK: {}, Err: {}", ok_count, err_count);
    for (const auto &[res, test] : std::ranges::zip_view{vec, suite}) {
      if (res.is_ok()) {
        std::println(
          "{}. Test {} ({}) OK\n", i, test->name(), get_test_context().get_time(res.running_time())
        );
      } else {
        std::println(
          "{}. Test {} ERR.\n{}({})\n",
          i,
          test->name(),
          res.get_error().value().name,
          res.get_error().value().message
        );
      }
      i += 1;
    }
    return std::pair{ok_count, err_count};
  }
  export int run_all_and_exit(
    size_t thread_count = 10, const test_suite &suite = get_test_context().tests
  ) {
    auto results = run_tests(suite);
    auto [ok_count, err_count] = print_test_result(results, suite).value();
    return err_count;
  }
}