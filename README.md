# STest

## About
**STest** is a unit testing framework for C/C++ based on Keith Nicholas's Seatest. **STest** is an xUnit style of unit testing framework, and is made to be portable. Installing **STest** is as easy as dropping the **stest.c** and **stest.h** files into your project.

## Features
- xUnit style asserts
- Easily extensible for custom asserts
- Optional color-coded output
- Cross platform in any C/C++ project
- Supports global set-up and tear-down functions
- Supports per-test set-up and tear-down functions
- Ability to selectively run tests and fixtures

## Asserts
| Assert | Arguments | Meaning |
|--------|-----------| ----------- |
|assert_true| int test | Asserts test is non-zero|
|assert_false| int test | Asserts test is zero|
|assert_int_equal| int expected, int actual| Asserts expected == actual|
|assert_ulong_equal| unsigned long expected, unsigned long actual| Asserts expected == actual|
|assert_string_equal| char* expected, char* actual| Asserts all characters of expected equal all characters of actual|
|assert_n_array_equal| void* expected, void* actual, int n| Asserts first n elements from expected to actual|
|assert_bit_set| int bit_number, int value| Asserts the bit_number in value is set to a 1|
|assert_bit_not_set| int bit_number, int value| Asserts the bit_number in value is set to a 0
|assert_bit_mask_matches| \<size> value, \<size> mask|Asserts all 1 bits in mask are set to 1 in value|
|assert_fail| char* message | Automatic failing test with a custom message|
|assert_float_equal| float expected, float actual, float delta| Asserts expected is within delta above or below value|
|assert_double_equal|double expected, double actual, double delta| Asserts expected is within delta above or below value|
|assert_string_contains| char* contained, char* container| Asserts contained is a substring of container|
|assert_string_not_contains|char* contained, char* container| Asserts contained is not a substring of container|
|assert_string_starts_with| char* contained, char* container| Asserts container begins with contained|
|assert_string_ends_with| char* contained, char* container| Asserts container ends with contained|

## Command Line Arguments
The test runner can be run with a few simple command line arguments.

| Option           | Meaning                                          |
| -----------------| -------------------------------------------------|
| -d               | Display tests, do not run tests                  |
| -v               | Run tests in verbose mode                        |
| -vs              | Alternative display mode                         |
| -t \<testname>   | Only run tests that match \<testname>            |
| -f \<fixturename>| Only run fixtures that match \<fixturename>      |
| -m               | Output machine readable                          |
| -s               | Skip the rest of the test when an assert fails   |
| -k \<marker>     | prepend \<marker> before machine readable output |
| -c               | Color code output (green success, red failure)   |
| help             | Output help message                              |

## Example Usage

```C
// Sample test
STEST(my_test)
  int actual = 10;
  assert_int_equal(10, actual);

  char *actual_str = "Hello World";
  assert_string_contains("Hello", actual_str);
}

STEST_HELPER(int, helper, int arg1, int arg2)
  assert_true(arg2 > arg1);
  return arg2 - arg1;
}

// Another sample Test calling a helper function
STEST(another_test)
  assert_true(1);
  assert_false(0);
  assert_int_equal(helper(1, 2));
}

// Sample fixture
// Fixtures are a collection of related tests
void test_fixture() {
  test_fixture_start();
  run_test(my_test());
  another_test();
  test_fixture_end();
}

// Sample test suite
// Should combine all fixtures that belong in the suite
void run_all_tests() { test_fixture(); }

// Keep main simple to run all of your test suites
int main(int argc, char **argv) {
  return stest_testrunner(argc, argv, run_all_tests, NULL, NULL);
}
```

## Contributing

I am happy to accept pull requests for bug fixes and new features. Here are the suggested steps:
1. Fork the repository
2. Create a new branch
3. Implement your feature
4. Reformat your code with the provided .clang-format file
5. Add your commits
6. Create a pull request to master

I will try to keep the issues tab updated with improvements I am envisioning. 