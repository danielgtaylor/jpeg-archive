
//
// assertion-macros.h
//
// Copyright (c) 2013 Stephen Mathieson
// MIT licensed
//


#ifndef __ASSERTION_MACROS_H__
#define __ASSERTION_MACROS_H__ 1

#define ASSERTIONS_VERSION "0.0.1"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int __assert_bail = 0;
int __assert_failures = 0;

/*
 * Bail at first failing assertion
 */

#define assert_bail() __assert_bail = !__assert_bail;

/*
 * Get the number of failed assertions
 */

#define assert_failures() __assert_failures

/*
 * Reset the number of failed assertions
 */

#define assert_reset() ({ \
  __assert_failures = 0; \
})

// don't clobber assert
#ifndef assert
#define assert assert_ok
#endif

/*
 * Assert that `expr` evaluates to something truthy
 */

#define assert_ok(expr) ({ \
  if (!(expr)) {\
    __assert_failures++; \
    fprintf(stderr, \
      "Assertion error: %s (%s:%d)\n", \
      #expr, __FILE__, __LINE__); \
    if (__assert_bail) abort(); \
  } \
})

/*
 * Assert that `a` is equal to `b`
 */

#define assert_equal(a, b) ({ \
  if (a != b) {\
    __assert_failures++; \
    fprintf(stderr, \
      "Assertion error: %d == %d (%s:%d)\n", \
      a, b, __FILE__, __LINE__); \
    if (__assert_bail) abort(); \
  } \
})

/*
 * Assert that `a` is equal to `b`
 */

#define assert_equal_float(a, b) ({ \
  if (a != b) {\
    __assert_failures++; \
    fprintf(stderr, \
      "Assertion error: %f == %f (%s:%d)\n", \
      a, b, __FILE__, __LINE__); \
    if (__assert_bail) abort(); \
  } \
})

/*
 * Assert that `a` is not equal to `b`
 */

#define assert_not_equal(a, b) ({ \
  if (a == b) {\
    __assert_failures++; \
    fprintf(stderr, \
      "Assertion error: %d != %d (%s:%d)\n", \
      a, b, __FILE__, __LINE__); \
    if (__assert_bail) abort(); \
  } \
})

/*
 * Assert that `a` is equal to `b`
 */

#define assert_str_equal(a, b) ({ \
  if (0 != strcmp(a, b))  {\
    __assert_failures++; \
    fprintf(stderr, \
      "Assertion error: \"%s\" == \"%s\" (%s:%d)\n", \
      a, b, __FILE__, __LINE__); \
    if (__assert_bail) abort(); \
  } \
})

/*
 * Assert that `a` is not equal to `b`
 */

#define assert_str_not_equal(a, b) ({ \
  if (0 == strcmp(a, b)) {\
    __assert_failures++; \
    fprintf(stderr, \
      "Assertion error: \"%s\" != \"%s\" (%s:%d)\n", \
      a, b, __FILE__, __LINE__); \
    if (__assert_bail) abort(); \
  } \
})

#endif
