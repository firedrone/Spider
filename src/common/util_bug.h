/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file util_bug.h
 **/

#ifndef TOR_UTIL_BUG_H
#define TOR_UTIL_BUG_H

#include "orconfig.h"
#include "compat.h"
#include "testsupport.h"

/* Replace assert() with a variant that sends failures to the log before
 * calling assert() normally.
 */
#ifdef NDEBUG
/* Nobody should ever want to build with NDEBUG set.  99% of our asserts will
 * be outside the critical path anyway, so it's silly to disable bug-checking
 * throughout the entire program just because a few asserts are slowing you
 * down.  Profile, optimize the critical path, and keep debugging on.
 *
 * And I'm not just saying that because some of our asserts check
 * security-critical properties.
 */
#error "Sorry; we don't support building with NDEBUG."
#endif

/* Sometimes we don't want to use assertions during branch coverage tests; it
 * leads to tons of unreached branches which in reality are only assertions we
 * didn't hit. */
#if defined(TOR_UNIT_TESTS) && defined(DISABLE_ASSERTS_IN_UNIT_TESTS)
#define spider_assert(a) STMT_BEGIN                                        \
  (void)(a);                                                            \
  STMT_END
#else
/** Like assert(3), but send assertion failures to the log as well as to
 * stderr. */
#define spider_assert(expr) STMT_BEGIN                                     \
  if (PREDICT_UNLIKELY(!(expr))) {                                      \
    spider_assertion_failed_(SHORT_FILE__, __LINE__, __func__, #expr);     \
    abort();                                                            \
  } STMT_END
#endif

#define spider_assert_unreached() spider_assert(0)

/* Non-fatal bug assertions. The "unreached" variants mean "this line should
 * never be reached." The "once" variants mean "Don't log a warning more than
 * once".
 *
 * The 'BUG' macro checks a boolean condition and logs an error message if it
 * is true.  Example usage:
 *   if (BUG(x == NULL))
 *     return -1;
 */

#ifdef ALL_BUGS_ARE_FATAL
#define spider_assert_nonfatal_unreached() spider_assert(0)
#define spider_assert_nonfatal(cond) spider_assert((cond))
#define spider_assert_nonfatal_unreached_once() spider_assert(0)
#define spider_assert_nonfatal_once(cond) spider_assert((cond))
#define BUG(cond)                                                       \
  (PREDICT_UNLIKELY(cond) ?                                             \
   (spider_assertion_failed_(SHORT_FILE__,__LINE__,__func__,"!("#cond")"), \
    abort(), 1)                                                         \
   : 0)
#elif defined(TOR_UNIT_TESTS) && defined(DISABLE_ASSERTS_IN_UNIT_TESTS)
#define spider_assert_nonfatal_unreached() STMT_NIL
#define spider_assert_nonfatal(cond) ((void)(cond))
#define spider_assert_nonfatal_unreached_once() STMT_NIL
#define spider_assert_nonfatal_once(cond) ((void)(cond))
#define BUG(cond) (PREDICT_UNLIKELY(cond) ? 1 : 0)
#else /* Normal case, !ALL_BUGS_ARE_FATAL, !DISABLE_ASSERTS_IN_UNIT_TESTS */
#define spider_assert_nonfatal_unreached() STMT_BEGIN                      \
  spider_bug_occurred_(SHORT_FILE__, __LINE__, __func__, NULL, 0);         \
  STMT_END
#define spider_assert_nonfatal(cond) STMT_BEGIN                            \
  if (PREDICT_UNLIKELY(!(cond))) {                                      \
    spider_bug_occurred_(SHORT_FILE__, __LINE__, __func__, #cond, 0);  \
  }                                                                     \
  STMT_END
#define spider_assert_nonfatal_unreached_once() STMT_BEGIN                 \
  static int warning_logged__ = 0;                                      \
  if (!warning_logged__) {                                              \
    warning_logged__ = 1;                                               \
    spider_bug_occurred_(SHORT_FILE__, __LINE__, __func__, NULL, 1);       \
  }                                                                     \
  STMT_END
#define spider_assert_nonfatal_once(cond) STMT_BEGIN                       \
  static int warning_logged__ = 0;                                      \
  if (!warning_logged__ && PREDICT_UNLIKELY(!(cond))) {                 \
    warning_logged__ = 1;                                               \
    spider_bug_occurred_(SHORT_FILE__, __LINE__, __func__, #cond, 1);      \
  }                                                                     \
  STMT_END
#define BUG(cond)                                                       \
  (PREDICT_UNLIKELY(cond) ?                                             \
   (spider_bug_occurred_(SHORT_FILE__,__LINE__,__func__,"!("#cond")",0), 1) \
   : 0)
#endif

#ifdef __GNUC__
#define IF_BUG_ONCE__(cond,var)                                         \
  if (( {                                                               \
      static int var = 0;                                               \
      int bool_result = (cond);                                         \
      if (PREDICT_UNLIKELY(bool_result) && !var) {                      \
        var = 1;                                                        \
        spider_bug_occurred_(SHORT_FILE__, __LINE__, __func__,             \
                          "!("#cond")", 1);                             \
      }                                                                 \
      PREDICT_UNLIKELY(bool_result); } ))
#else
#define IF_BUG_ONCE__(cond,var)                                         \
  static int var = 0;                                                   \
  if (PREDICT_UNLIKELY(cond) ?                                          \
      (var ? 1 :                                                        \
       (var=1,                                                          \
        spider_bug_occurred_(SHORT_FILE__, __LINE__, __func__,             \
                           "!("#cond")", 1),                            \
        1))                                                             \
      : 0)
#endif
#define IF_BUG_ONCE_VARNAME_(a)               \
  warning_logged_on_ ## a ## __
#define IF_BUG_ONCE_VARNAME__(a)              \
  IF_BUG_ONCE_VARNAME_(a)

/** This macro behaves as 'if (bug(x))', except that it only logs its
 * warning once, no matter how many times it triggers.
 */

#define IF_BUG_ONCE(cond)                                       \
  IF_BUG_ONCE__((cond),                                         \
                IF_BUG_ONCE_VARNAME__(__LINE__))

/** Define this if you want Spider to crash when any problem comes up,
 * so you can get a coredump and track things down. */
// #define spider_fragile_assert() spider_assert_unreached(0)
#define spider_fragile_assert() spider_assert_nonfatal_unreached_once()

void spider_assertion_failed_(const char *fname, unsigned int line,
                           const char *func, const char *expr);
void spider_bug_occurred_(const char *fname, unsigned int line,
                       const char *func, const char *expr,
                       int once);

#ifdef TOR_UNIT_TESTS
void spider_capture_bugs_(int n);
void spider_end_capture_bugs_(void);
const struct smartlist_t *spider_get_captured_bug_log_(void);
void spider_set_failed_assertion_callback(void (*fn)(void));
#endif

#endif

