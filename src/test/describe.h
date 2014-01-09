
//
// describe.h
//
// Copyright (c) 2013 Stephen Mathieson
// MIT licensed
//


#ifndef DESCRIBE_H
#define DESCRIBE_H 1

#include "assertion-macros.h"

#define DESCRIBE_VERSION "0.0.1"
#define DESCRIBE_OK      "✓"
#define DESCRIBE_FAIL    "✖"

/*
 * Describe `fn` with `title`
 */

#define describe(title, fn) int main(void) { \
  printf("\n  %s\n", title); \
  fn; \
  printf("\n"); \
  return assert_failures(); \
}

/*
 * Describe `fn` with `specification`
 */

#define it(specification, fn) ({ \
  int before = assert_failures(); \
  fn; \
  if (assert_failures() == before) { \
    printf("    \e[92m%s \e[90m%s\e[0m\n", DESCRIBE_OK, specification); \
  } else { \
    printf("    \e[90m%s \e[90m%s\e[0m\n", DESCRIBE_FAIL, specification); \
  } \
});

#endif
