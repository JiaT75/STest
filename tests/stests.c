/*
 * Copyright (c) 2021 Jia Tan
 * Copyright (c) 2010 Keith Nicholas
 */

#include "stests.h"

void test_assert_n_array_equal() {
  int array_1[4] = {0, 1, 2, 3};
  int array_2[4] = {0, 1, 2, 4};
  int array_3[4] = {0, 1, 2, 3};

  assert_test_passes(assert_n_array_equal(array_1, array_1, 4));
  assert_test_passes(assert_n_array_equal(array_1, array_3, 4));
  assert_test_passes(assert_n_array_equal(array_1, array_2, 3));
  assert_test_fails(assert_n_array_equal(array_1, array_2, 4));
  assert_test_fails(assert_n_array_equal(array_1, array_2, 0));
}

void test_assert_string_equal() {
  assert_test_passes(assert_string_equal((char *)0, (char *)0));
  assert_test_passes(assert_string_equal("", ""));
  assert_test_passes(assert_string_equal("foo", "foo"));
  assert_test_fails(assert_string_equal((char *)0, "bar"));
  assert_test_fails(assert_string_equal("foo", (char *)0));
  assert_test_fails(assert_string_equal("foo", "bar"));
  assert_test_fails(assert_string_equal("foo", "Foo"));
  assert_test_fails(assert_string_equal("foo", "foo\n"));
}

void test_assert_ulong_equal() {
  assert_test_passes(assert_ulong_equal(1, 1));
  assert_test_passes(assert_ulong_equal(-2, -2));
  assert_test_fails(assert_ulong_equal(1, 0));
  assert_test_fails(assert_ulong_equal(-2, 2));
}

void test_assert_int_equal() {
  assert_test_passes(assert_int_equal(1, 1));
  assert_test_passes(assert_int_equal(-2, -2));
  assert_test_fails(assert_int_equal(1, 0));
  assert_test_fails(assert_int_equal(-2, 2));
}

void test_assert_true() {
  assert_test_passes(assert_true(1));
  assert_test_fails(assert_true(0));
}

void test_assert_false() {
  assert_test_passes(assert_false(0));
  assert_test_fails(assert_false(1));
}

void test_assert_fail() { assert_test_fails(assert_fail("")); }

void test_assert_bit_set() {
  for(int bit = 0, value = 1; bit < sizeof(int) * 8; bit++, value <<= 1) {
    assert_test_passes(assert_bit_set(bit, value));
    if(bit > 0) {
      assert_test_fails(assert_bit_set(bit - 1, value));
    }
  }
}

void test_assert_bit_not_set() {
  for(int bit = 0, value = 1; bit < sizeof(int) * 8; bit++, value <<= 1) {
    assert_test_fails(assert_bit_not_set(bit, value));
    if(bit > 0) {
      assert_test_passes(assert_bit_not_set(bit - 1, value));
    }
  }
}

void test_assert_bit_mask_matches() {
  // mask in binary => 000100100011010001010110
  int mask = 0x123456;
  for(int i = 0; i < sizeof(int) * 8 - 1; i++) {
    assert_test_passes(assert_bit_mask_matches((i | mask), mask));
    assert_test_fails(assert_bit_mask_matches(i, mask));
  }
}

void test_assert_double_equal() {
  const double delta = 0.001;
  assert_test_passes(assert_double_equal(1.0, 1.0, delta));
  assert_test_fails(assert_double_equal(1.0, 2.0, delta));
  double d1 = 1.5;
  double d2 = 2.5;
  assert_test_passes(assert_double_equal(d2 - 1, d1, delta));
  assert_test_passes(assert_double_equal(d1 + 1, d2, delta));
  assert_test_fails(assert_double_equal(d1, d2, delta));
}

void test_assert_string_contains() {
  const char *str1 = "string one";
  const char *str2 = "string one and more";
  assert_test_passes(assert_string_contains(str1, str2));
  assert_test_fails(assert_string_contains(str2, str1));
}

void test_assert_string_not_contains() {
  const char *str1 = "string one";
  const char *str2 = "string one and more";
  assert_test_fails(assert_string_not_contains(str1, str2));
  assert_test_passes(assert_string_not_contains(str2, str1));
}

void test_assert_string_starts_with() {
  const char *str1 = "string one";
  const char *str2 = "string one and more";
  assert_test_passes(assert_string_starts_with(str1, str2));
  assert_test_fails(assert_string_starts_with(str2, str1));
}

void test_assert_string_ends_with() {
  const char *str1 = "and more";
  const char *str2 = "string one and more";
  assert_test_passes(assert_string_ends_with(str1, str2));
  assert_test_fails(assert_string_ends_with(str2, str1));
}

void test_fixture_stest() {
  test_fixture_start();
  run_test(test_assert_true);
  run_test(test_assert_false);
  run_test(test_assert_int_equal);
  run_test(test_assert_ulong_equal);
  run_test(test_assert_string_equal);
  run_test(test_assert_n_array_equal);
  run_test(test_assert_fail);
  run_test(test_assert_bit_set);
  run_test(test_assert_bit_not_set);
  run_test(test_assert_bit_mask_matches);
  run_test(test_assert_double_equal);
  run_test(test_assert_string_contains);
  run_test(test_assert_string_not_contains);
  run_test(test_assert_string_starts_with);
  run_test(test_assert_string_ends_with);
  test_fixture_end();
}

void all_tests() { test_fixture_stest(); }

int main(int argc, char **argv) {
  return stest_testrunner(argc, argv, all_tests, NULL, NULL);
}
