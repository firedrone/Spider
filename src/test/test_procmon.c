/* Copyright (c) 2010-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

#define PROCMON_PRIVATE
#include "orconfig.h"
#include "or.h"
#include "test.h"

#include "procmon.h"

#include "log_test_helpers.h"

#define NS_MODULE procmon

struct event_base;

static void
test_procmon_spider_process_monispider_new(void *ignored)
{
  (void)ignored;
  spider_process_monispider_t *res;
  const char *msg;

  res = spider_process_monispider_new(NULL, "probably invalid", 0, NULL, NULL, &msg);
  tt_assert(!res);
  tt_str_op(msg, OP_EQ, "invalid PID");

  res = spider_process_monispider_new(NULL, "243443535345454", 0, NULL, NULL, &msg);
  tt_assert(!res);
  tt_str_op(msg, OP_EQ, "invalid PID");

  res = spider_process_monispider_new(spider_libevent_get_base(), "43", 0,
                                NULL, NULL, &msg);
  tt_assert(res);
  tt_assert(!msg);
  spider_process_monispider_free(res);

  res = spider_process_monispider_new(spider_libevent_get_base(), "44 hello", 0,
                                NULL, NULL, &msg);
  tt_assert(res);
  tt_assert(!msg);
  spider_process_monispider_free(res);

  res = spider_process_monispider_new(spider_libevent_get_base(), "45:hello", 0,
                                NULL, NULL, &msg);
  tt_assert(res);
  tt_assert(!msg);

 done:
  spider_process_monispider_free(res);
}

struct testcase_t procmon_tests[] = {
  { "spider_process_monispider_new", test_procmon_spider_process_monispider_new,
    TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};

