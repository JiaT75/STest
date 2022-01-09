/*
 * Copyright (c) 2021 Jia Tan
 * Copyright (c) 2010 Keith Nicholas
 */

#include "stest.h"
#include <string.h>

#ifdef STEST_INTERNAL_TESTS
static int stest_last_passed = 0;
#endif

#define STEST_RET_ERROR (-1)
#define STEST_RET_OK 0
#define STEST_RET_FAILED_COUNT(tests_failed_count) (tests_failed_count)

#define STEST_GREEN "\e[0;32m"
#define STEST_RED "\e[0;31m"
#define STEST_COLOR_RESET "\e[0m"

typedef enum {
  STEST_DISPLAY_TESTS,
  STEST_RUN_TESTS,
  STEST_DO_NOTHING,
  STEST_DO_ABORT
} stest_action_t;

typedef struct {
  int argc;
  char **argv;
  stest_action_t action;
} stest_testrunner_t;

static int stest_screen_width = 70;
static int stests_run = 0;
static int stests_passed = 0;
static int stests_failed = 0;
static int stest_display_only = 0;
static int stest_verbose = 0;
static int vs_mode = 0;
static int stest_machine_readable = 0;
static int stest_color_output = 0;
static const char *stest_current_fixture;
static const char *stest_current_fixture_path;
static char stest_magic_marker[20];
static int stest_fixture_tests_run = 0;
static int stest_fixture_tests_failed = 0;
static const char *stest_fixture_filter;
static const char *stest_test_filter;

static stest_void_void stest_suite_setup_func = 0;
static stest_void_void stest_suite_teardown_func = 0;
static stest_void_void stest_fixture_setup = 0;
static stest_void_void stest_fixture_teardown = 0;

unsigned int GetTickCount(void);
int stest_is_string_equal_i(const char *s1, const char *s2);
int stest_is_display_only(void);
const char *test_file_name(const char *path);
static int stest_can_color(void);
void stest_header_printer(const char *s, int s_len, int length, char f);
void set_magic_marker(const char *marker);
void stest_show_help(void);
int stest_commandline_has_value_after(stest_testrunner_t *runner, int arg);
int stest_parse_commandline_option_with_value(stest_testrunner_t *runner,
                                              int arg, const char *option,
                                              stest_void_string setter);
void stest_interpret_commandline(stest_testrunner_t *runner);
void stest_testrunner_create(stest_testrunner_t *runner, int argc, char **argv);

#define SECONDS_TO_MILLISECONDS(sec) sec * 1000
#define MICRO_SECONDS_TO_MILLISECONDS(microsec) microsec / 1000

#ifdef WIN32
#include "windows.h"
int stest_is_string_equal_i(const char *s1, const char *s2) {
#pragma warning(disable : 4996)
  return stricmp(s1, s2) == 0;
}

#else
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>

unsigned int GetTickCount(void) {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  return SECONDS_TO_MILLISECONDS(current_time.tv_sec) +
         MICRO_SECONDS_TO_MILLISECONDS(current_time.tv_usec);
}

int stest_is_string_equal_i(const char *s1, const char *s2) {
  return strcasecmp(s1, s2) == 0;
}

#endif

void (*stest_simple_test_result)(int passed, const char *reason,
                                 const char *function, unsigned int line) =
    stest_simple_test_result_log;

void suite_setup(stest_void_void setup) { stest_suite_setup_func = setup; }
void suite_teardown(stest_void_void teardown) {
  stest_suite_teardown_func = teardown;
}

int stest_is_display_only(void) { return stest_display_only; }

void stest_suite_setup(void) {
  if(stest_suite_setup_func != 0)
    stest_suite_setup_func();
}

void stest_suite_teardown(void) {
  if(stest_suite_teardown_func != 0)
    stest_suite_teardown_func();
}

void fixture_setup(void (*setup)(void)) { stest_fixture_setup = setup; }
void fixture_teardown(void (*teardown)(void)) {
  stest_fixture_teardown = teardown;
}

void stest_setup(void) {
  if(stest_fixture_setup != 0)
    stest_fixture_setup();
}

void stest_teardown(void) {
  if(stest_fixture_teardown != 0)
    stest_fixture_teardown();
}

const char *test_file_name(const char *path) {
  char *file = (char *)path + strlen(path);
  while(file != path && *file != '\\' && *file != '/')
    file--;
  if(*file == '\\' || *file == '/')
    file++;
  return file;
}

static int stest_can_color(void) { return stest_color_output; }

static void stest_determine_color_output(FILE *standard_out) {
#ifdef WIN32
  // Output coloring is not supported on windows
  stest_color_output = 0;
#else
  stest_color_output = isatty(fileno(standard_out));
#endif
}

static void stest_add_color(char *outstr, const char *instr,
                            const char *color) {
  if(stest_can_color()) {
    sprintf(outstr, "%s%s%s", color, instr, STEST_COLOR_RESET);
  }
  else {
    strcpy(outstr, instr);
  }
}

static void stest_log_failure(const char *reason, const char *function,
                              unsigned int line) {
  char failed[100];
  stest_add_color(failed, reason, STEST_RED);
  if(vs_mode) {
    printf("%s (%u)		%s,%s\r\n", stest_current_fixture_path, line,
           function, failed);
  }
  else {
    printf("%-30s Line %-5d %s\r\n", function, line, failed);
  }
}

static void stest_log_success(const char *function, unsigned int line) {
  char passed[30];
  stest_add_color(passed, "Passed", STEST_GREEN);
  printf("%-30s Line %-5d %s\r\n", function, line, passed);
}

void stest_simple_test_result_log(int passed, const char *reason,
                                  const char *function, unsigned int line) {
  if(!passed) {

    if(stest_machine_readable) {
      if(vs_mode) {
        printf("%s (%u)		%s,%s\r\n", stest_current_fixture_path, line,
               function, reason);
      }
      else {
        printf("%s%s,%s,%u,%s\r\n", stest_magic_marker,
               stest_current_fixture_path, function, line, reason);
      }
    }
    else {
      stest_log_failure(reason, function, line);
    }
    stests_failed++;

#ifdef ABORT_TEST_IF_ASSERT_FAIL
    printf("Test has been finished with failure.\r\n");
    longjmp(env, 1);
#endif
  }
  else {
    if(stest_verbose) {
      if(stest_machine_readable) {
        printf("%s%s,%s,%u,Passed\r\n", stest_magic_marker,
               stest_current_fixture_path, function, line);
      }
      else {
        stest_log_success(function, line);
      }
    }
    stests_passed++;
  }
}

void stest_assert_true(int test, const char *function, unsigned int line) {
  stest_simple_test_result(test, "Should have been true", function, line);
}

void stest_assert_false(int test, const char *function, unsigned int line) {
  stest_simple_test_result(!test, "Should have been false", function, line);
}

void stest_assert_int_equal(int expected, int actual, const char *function,
                            unsigned int line) {
  char s[STEST_PRINT_BUFFER_SIZE];
  sprintf(s, "Expected %d but was %d", expected, actual);
  stest_simple_test_result(expected == actual, s, function, line);
}

void stest_assert_ulong_equal(unsigned long expected, unsigned long actual,
                              const char *function, unsigned int line) {
  char s[STEST_PRINT_BUFFER_SIZE];
  sprintf(s, "Expected %lu but was %lu", expected, actual);
  stest_simple_test_result(expected == actual, s, function, line);
}

void stest_assert_float_equal(float expected, float actual, float delta,
                              const char *function, unsigned int line) {
  char s[STEST_PRINT_BUFFER_SIZE];
  float result = expected - actual;
  sprintf(s, "Expected %f but was %f", expected, actual);
  if(result < 0.0)
    result = 0.0f - result;
  stest_simple_test_result(result <= delta, s, function, line);
}

void stest_assert_double_equal(double expected, double actual, double delta,
                               const char *function, unsigned int line) {
  char s[STEST_PRINT_BUFFER_SIZE];
  double result = expected - actual;
  sprintf(s, "Expected %f but was %f", expected, actual);
  if(result < 0.0)
    result = 0.0 - result;
  stest_simple_test_result(result <= delta, s, function, line);
}

void stest_assert_string_equal(const char *expected, const char *actual,
                               const char *function, unsigned int line) {
  int comparison;
  char s[STEST_PRINT_BUFFER_SIZE];

  if((expected == (char *)0) && (actual == (char *)0)) {
    sprintf(s, "Expected <NULL> but was <NULL>");
    comparison = 1;
  }
  else if(expected == (char *)0) {
    sprintf(s, "Expected <NULL> but was %s", actual);
    comparison = 0;
  }
  else if(actual == (char *)0) {
    sprintf(s, "Expected %s but was <NULL>", expected);
    comparison = 0;
  }
  else {
    comparison = strcmp(expected, actual) == 0;
    sprintf(s, "Expected %s but was %s", expected, actual);
  }

  stest_simple_test_result(comparison, s, function, line);
}

void stest_assert_string_ends_with(const char *expected, const char *actual,
                                   const char *function, unsigned int line) {
  char s[STEST_PRINT_BUFFER_SIZE];
  sprintf(s, "Expected %s to end with %s", actual, expected);
  stest_simple_test_result(
      strcmp(expected, actual + (strlen(actual) - strlen(expected))) == 0, s,
      function, line);
}

void stest_assert_string_starts_with(const char *expected, const char *actual,
                                     const char *function, unsigned int line) {
  char s[STEST_PRINT_BUFFER_SIZE];
  sprintf(s, "Expected %s to start with %s", actual, expected);
  stest_simple_test_result(strncmp(expected, actual, strlen(expected)) == 0, s,
                           function, line);
}

void stest_assert_string_contains(const char *expected, const char *actual,
                                  const char *function, unsigned int line) {
  char s[STEST_PRINT_BUFFER_SIZE];
  sprintf(s, "Expected %s to be in %s", expected, actual);
  stest_simple_test_result(strstr(actual, expected) != 0, s, function, line);
}

void stest_assert_string_not_contains(const char *expected, const char *actual,
                                      const char *function, unsigned int line) {
  char s[STEST_PRINT_BUFFER_SIZE];
  sprintf(s, "Expected %s not to have %s in it", actual, expected);
  stest_simple_test_result(strstr(actual, expected) == 0, s, function, line);
}

void stest_header_printer(const char *s, int s_len, int length, char f) {
  int d = (length - (s_len + 2)) / 2;
  int i;
  if(stest_is_display_only() || stest_machine_readable)
    return;
  for(i = 0; i < d; i++)
    printf("%c", f);
  if(s_len == 0)
    printf("%c%c", f, f);
  else
    printf(" %s ", s);
  for(i = (d + s_len + 2); i < length; i++)
    printf("%c", f);
  printf("\r\n");
}

void stest_test_fixture_start(const char *filepath) {
  stest_current_fixture_path = filepath;
  stest_current_fixture = test_file_name(filepath);

  if(!stest_should_run_fixture(stest_current_fixture)) {
    return;
  }

  if(stest_is_display_only()) {
    printf("Fixture: %s\n", stest_current_fixture);
  }
  else {
    stest_header_printer(stest_current_fixture, strlen(stest_current_fixture),
                         stest_screen_width, '-');
    stest_fixture_tests_failed = stests_failed;
    stest_fixture_tests_run = stests_run;
  }

  stest_fixture_teardown = 0;
  stest_fixture_setup = 0;
}

void stest_test_fixture_end(void) {
  char s[STEST_PRINT_BUFFER_SIZE];
  sprintf(s, "%d run %d failed", stests_run - stest_fixture_tests_run,
          stests_failed - stest_fixture_tests_failed);
  stest_header_printer(s, strlen(s), stest_screen_width, ' ');
  if(stest_is_display_only() || stest_machine_readable)
    return;
  printf("\r\n");
}

void fixture_filter(const char *filter) { stest_fixture_filter = filter; }

void test_filter(const char *filter) { stest_test_filter = filter; }

void set_magic_marker(const char *marker) {
  if(marker == NULL)
    return;
  strcpy(stest_magic_marker, marker);
}

int stest_should_run_test(const char *test) {
  int run = 1;

  if(stest_fixture_filter) {
    if(strncmp(stest_fixture_filter, stest_current_fixture,
               strlen(stest_fixture_filter)) != 0)
      run = 0;
  }

  if(stest_test_filter && test != NULL) {
    if(strncmp(stest_test_filter, test, strlen(stest_test_filter)) != 0)
      run = 0;
  }

  return run;
}

int stest_should_run_fixture(const char *fixture) {
  int run = 1;

  if(stest_fixture_filter) {
    if(strncmp(stest_fixture_filter, fixture, strlen(stest_fixture_filter)) !=
       0)
      run = 0;
  }

  return run;
}

void stest_test(const char *test, void (*test_function)(void)) {
  if(!stest_should_run_test(test)) {
    return;
  }

  if(stest_is_display_only()) {
    printf("%s\n", test);
  }

  stest_suite_setup();
  stest_setup();

#ifdef ABORT_TEST_IF_ASSERT_FAIL
  skip_failed_test = setjmp(env);
  if(!skip_failed_test)
    test_function();
#else
  test_function();
#endif

  stest_teardown();
  stest_suite_teardown();
  stests_run++;
}

int run_tests(stest_void_void tests) {
  unsigned long end;
  unsigned long start = GetTickCount();
  char s[40];
  tests();
  end = GetTickCount();

  if(stest_is_display_only() || stest_machine_readable)
    return STEST_RET_OK;
  printf("\r\n");
  if(stests_failed > 0) {
    if(stest_machine_readable) {
      stest_header_printer("Failed", sizeof("Failed") - 1, stest_screen_width,
                           ' ');
    }
    else {
      char failed[30];
      stest_add_color(failed, "failed", STEST_RED);
      stest_header_printer(failed, sizeof("Failed") - 1, stest_screen_width,
                           ' ');
    }
  }
  else {
    if(stest_machine_readable) {
      stest_header_printer("ALL TESTS PASSED", sizeof("ALL TESTS PASSED") - 1,
                           stest_screen_width, ' ');
    }
    else {
      char passed[30];
      stest_add_color(passed, "ALL TESTS PASSED", STEST_GREEN);
      stest_header_printer(passed, sizeof("ALL TESTS PASSED") - 1,
                           stest_screen_width, ' ');
    }
  }
  if(stests_run == 1) {
    strcpy(s, "1 test run");
  }
  else {
    sprintf(s, "%d tests run", stests_run);
  }
  stest_header_printer(s, strlen(s), stest_screen_width, ' ');
  sprintf(s, "in %lu ms", end - start);
  stest_header_printer(s, strlen(s), stest_screen_width, ' ');
  printf("\r\n");
  stest_header_printer("", sizeof("") - 1, stest_screen_width, '=');

  return STEST_RET_FAILED_COUNT(stests_failed);
}

void stest_show_help(void) {
  printf("Usage: [-t <testname>] [-f <fixturename>] [-d] [-h | --help] [-v] "
         "[-m] [-k <marker>]\r\n");
  printf("Flags:\r\n");
  printf("\thelp:\twill display this help\r\n");
  printf("\t-t:\twill only run tests that match <testname>\r\n");
  printf("\t-f:\twill only run fixtures that match <fixturename>\r\n");
  printf("\t-d:\twill just display test names and fixtures without\r\n");
  printf("\t\trunning the test\r\n");
  printf("\t-v:\twill print a more verbose version of the test run\r\n");
  printf("\t-m:\twill print a machine readable format of the test run, ie :- "
         "\r\n");
  printf("\t   \t<textfixture>,<testname>,<linenumber>,<testresult><EOL>\r\n");
  printf("\t-k:\twill prepend <marker> before machine readable output \r\n");
  printf("\t   \t<marker> cannot start with a '-'\r\n");
}

int stest_commandline_has_value_after(stest_testrunner_t *runner, int arg) {
  if(!((arg + 1) < runner->argc))
    return 0;
  if(runner->argv[arg + 1][0] == '-')
    return 0;
  return 1;
}

int stest_parse_commandline_option_with_value(stest_testrunner_t *runner,
                                              int arg, const char *option,
                                              stest_void_string setter) {
  if(stest_is_string_equal_i(runner->argv[arg], option)) {
    if(!stest_commandline_has_value_after(runner, arg)) {
      printf("Error: The %s option expects to be followed by a value\r\n",
             option);
      runner->action = STEST_DO_ABORT;
      return 0;
    }
    setter(runner->argv[arg + 1]);
    return 1;
  }
  return 0;
}

void stest_interpret_commandline(stest_testrunner_t *runner) {
  int arg;
  for(arg = 1; (arg < runner->argc) && (runner->action != STEST_DO_ABORT);
      arg++) {
    if(stest_is_string_equal_i(runner->argv[arg], "--help") ||
       stest_is_string_equal_i(runner->argv[arg], "-h")) {
      stest_show_help();
      runner->action = STEST_DO_NOTHING;
      return;
    }
    else if(stest_is_string_equal_i(runner->argv[arg], "-d"))
      runner->action = STEST_DISPLAY_TESTS;
    else if(stest_is_string_equal_i(runner->argv[arg], "-v"))
      stest_verbose = 1;
    else if(stest_is_string_equal_i(runner->argv[arg], "-vs"))
      vs_mode = 1;
    else if(stest_is_string_equal_i(runner->argv[arg], "-m"))
      stest_machine_readable = 1;
    else if(stest_parse_commandline_option_with_value(runner, arg, "-t",
                                                      test_filter))
      arg++;
    else if(stest_parse_commandline_option_with_value(runner, arg, "-f",
                                                      fixture_filter))
      arg++;
    else if(stest_parse_commandline_option_with_value(runner, arg, "-k",
                                                      set_magic_marker))
      arg++;
    else {
      printf("Error: %s option is not supported. Here is the help menu:\n",
             runner->argv[arg]);
      stest_show_help();
      runner->action = STEST_DO_NOTHING;
      return;
    }
  }
}

void stest_testrunner_create(stest_testrunner_t *runner, int argc,
                             char **argv) {
  runner->action = STEST_RUN_TESTS;
  runner->argc = argc;
  runner->argv = argv;
  stest_interpret_commandline(runner);
}

int stest_testrunner(int argc, char **argv, stest_void_void tests,
                     stest_void_void setup, stest_void_void teardown) {
  stest_testrunner_t runner;
  stest_testrunner_create(&runner, argc, argv);
  stest_determine_color_output(stdout);
  switch(runner.action) {
  case STEST_DISPLAY_TESTS: {
    stest_display_only = 1;
    run_tests(tests);
    return STEST_RET_OK;
  }
  case STEST_RUN_TESTS: {
    stest_display_only = 0;
    suite_setup(setup);
    suite_teardown(teardown);
    return run_tests(tests);
  }
  case STEST_DO_NOTHING: {
    return STEST_RET_OK;
  }
  case STEST_DO_ABORT:
  default: {
    /* there was an error which should of been already printed out. */
    return STEST_RET_ERROR;
  }
  }
  return STEST_RET_ERROR;
}

#ifdef STEST_INTERNAL_TESTS
void stest_simple_test_result_nolog(int passed, const char *reason,
                                    const char *function, unsigned int line) {
  stest_last_passed = passed;
}

void stest_assert_last_passed(const char *function, unsigned int line) {
  stest_assert_true(stest_last_passed, function, line);
}

void stest_assert_last_failed(const char *function, unsigned int line) {
  stest_assert_false(stest_last_passed, function, line);
}

void stest_disable_logging() {
  stest_simple_test_result = stest_simple_test_result_nolog;
}

void stest_enable_logging() {
  stest_simple_test_result = stest_simple_test_result_log;
}
#endif
