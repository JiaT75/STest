/*
 * Copyright (c) 2010 Keith Nicholas
 * Copyright (c) 2021 Jia Tan
 */

#ifndef STEST_H
#define STEST_H

#include <stdio.h>

/*
Defines
*/

#define STEST_PRINT_BUFFER_SIZE 10000

/*
Typedefs
*/

typedef void (*stest_void_void)(void);
typedef void (*stest_void_string)(const char *);

/*
Declarations
*/

extern void (*stest_simple_test_result)(int passed, const char *reason,
                                        const char *function,
                                        unsigned int line);
void stest_test_fixture_start(const char *filepath);
void stest_test_fixture_end(void);
void stest_simple_test_result_log(int passed, const char *reason,
                                  const char *function, unsigned int line);
void stest_assert_true(int test, const char *function, unsigned int line);
void stest_assert_false(int test, const char *function, unsigned int line);
void stest_assert_int_equal(int expected, int actual, const char *function,
                            unsigned int line);
void stest_assert_ulong_equal(unsigned long expected, unsigned long actual,
                              const char *function, unsigned int line);
void stest_assert_float_equal(float expected, float actual, float delta,
                              const char *function, unsigned int line);
void stest_assert_double_equal(double expected, double actual, double delta,
                               const char *function, unsigned int line);
void stest_assert_string_equal(const char *expected, const char *actual,
                               const char *function, unsigned int line);
void stest_assert_string_ends_with(const char *expected, const char *actual,
                                   const char *function, unsigned int line);
void stest_assert_string_starts_with(const char *expected, const char *actual,
                                     const char *function, unsigned int line);
void stest_assert_string_contains(const char *expected, const char *actual,
                                  const char *function, unsigned int line);
void stest_assert_string_not_contains(const char *expected, const char *actual,
                                      const char *function, unsigned int line);
int stest_should_run_fixture(const char *fixture);
int stest_should_run_test(const char *test);
void stest_before_run(const char *fixture, const char *test);
void stest_setup(void);
void stest_teardown(void);
void stest_suite_teardown(void);
void stest_suite_setup(void);
void stest_test(const char *test, void (*test_function)(void));

/*
Assert Macros
*/

// clang-format off
#define assert_true(test) do { stest_assert_true(test, __func__, __LINE__); } while (0)
#define assert_false(test) do {  stest_assert_false(test, __func__, __LINE__); } while (0)
#define assert_int_equal(expected, actual) do {  stest_assert_int_equal(expected, actual, __func__, __LINE__); } while (0)
#define assert_ulong_equal(expected, actual) do {  stest_assert_ulong_equal(expected, actual, __func__, __LINE__); } while (0)
#define assert_string_equal(expected, actual) do {  stest_assert_string_equal(expected, actual, __func__, __LINE__); } while (0)
#define assert_n_array_equal(expected, actual, n) do { size_t stest_count; for(stest_count=0; stest_count<n; stest_count++) { char s_stest[STEST_PRINT_BUFFER_SIZE]; sprintf(s_stest,"Expected %d to be %d at position %d", actual[stest_count], expected[stest_count], (int)stest_count); stest_simple_test_result((expected[stest_count] == actual[stest_count]), s_stest, __func__, __LINE__);} } while (0)
#define assert_bit_set(bit_number, value) { stest_simple_test_result(((1 << bit_number) & value), " Expected bit to be set" ,  __func__, __LINE__); } while (0)
#define assert_bit_not_set(bit_number, value) { stest_simple_test_result(!((1 << bit_number) & value), " Expected bit not to to be set" ,  __func__, __LINE__); } while (0)
#define assert_bit_mask_matches(value, mask) { stest_simple_test_result(((value & mask) == mask), " Expected all bits of mask to be set" ,  __func__, __LINE__); } while (0)
#define assert_fail(message) { stest_simple_test_result(0, message,  __func__, __LINE__); } while (0)
#define assert_float_equal(expected, actual, delta) do {  stest_assert_float_equal(expected, actual, delta, __func__, __LINE__); } while (0)
#define assert_double_equal(expected, actual, delta) do {  stest_assert_double_equal(expected, actual, delta, __func__, __LINE__); } while (0)
#define assert_string_contains(expected, actual) do {  stest_assert_string_contains(expected, actual, __func__, __LINE__); } while (0)
#define assert_string_not_contains(expected, actual) do {  stest_assert_string_not_contains(expected, actual, __func__, __LINE__); } while (0)
#define assert_string_starts_with(expected, actual) do {  stest_assert_string_starts_with(expected, actual, __func__, __LINE__); } while (0)
#define assert_string_ends_with(expected, actual) do {  stest_assert_string_ends_with(expected, actual, __func__, __LINE__); } while (0)

/*
Fixture / Test Management
*/

void fixture_setup(void (*setup)( void ));
void fixture_teardown(void (*teardown)( void ));
#define run_test(test) do { stest_test(#test, test);} while (0)
#define test_fixture_start() do { stest_test_fixture_start(__FILE__); } while (0)
#define test_fixture_end() do { stest_test_fixture_end();} while (0)
void fixture_filter(const char* filter);
void test_filter(const char* filter);
void suite_teardown(stest_void_void teardown);
void suite_setup(stest_void_void setup);
int run_tests(stest_void_void tests);
int stest_testrunner(int argc, char** argv, stest_void_void tests, stest_void_void setup, stest_void_void teardown);
#endif
//clang-format on

#ifdef STEST_INTERNAL_TESTS
void stest_simple_test_result_nolog(int passed, const char* reason, const char* function, unsigned int line);
void stest_assert_last_passed(const char* function, unsigned int line);
void stest_assert_last_failed(const char* function, unsigned int line);
void stest_enable_logging(void);
void stest_disable_logging(void);
#endif
