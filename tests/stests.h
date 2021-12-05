/* 
 * Copyright (c) 2010 Keith Nicholas
*/

#include "../src/stest.h"

#ifndef STEST_INTERNAL_TESTS
#error STEST_INTERNAL_TESTS must be defined when compiling tests for stest itself.
#endif

#define without_logging(X) stest_disable_logging(); X; stest_enable_logging();
#define assert_test_passes(X) without_logging(X); stest_assert_last_passed();
#define assert_test_fails(X) without_logging(X); stest_assert_last_failed();

void test_assert_true();
void test_assert_false();
void test_assert_int_equal();
void test_assert_ulong_equal();
void test_assert_string_equal();
void test_assert_n_array_equal();

void test_fixture_stest();
void all_tests();

