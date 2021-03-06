/*
 * Copyright (c) 2021 Jia Tan
 * Copyright (c) 2010 Keith Nicholas
 */

#include "../src/stest.h"

#ifndef STEST_INTERNAL_TESTS
#error STEST_INTERNAL_TESTS must be defined when compiling tests for stest itself.
#endif

// clang-format off
#define without_logging(X) stest_disable_logging(); X; stest_enable_logging();
#define assert_test_passes(X) without_logging(X); stest_assert_last_passed(__FUNCTION__, __LINE__);
#define assert_test_fails(X) without_logging(X); stest_assert_last_failed(__FUNCTION__, __LINE__);
// clang-format on
