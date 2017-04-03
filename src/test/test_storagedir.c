/* Copyright (c) 2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

#include "or.h"
#include "storagedir.h"
#include "test.h"

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif

static void
test_sspideragedir_empty(void *arg)
{
  char *dirname = spider_strdup(get_fname_rnd("sspidere_dir"));
  sspiderage_dir_t *d = NULL;
  (void)arg;

  tt_int_op(FN_NOENT, OP_EQ, file_status(dirname));

  d = sspiderage_dir_new(dirname, 10);
  tt_assert(d);

  tt_int_op(FN_DIR, OP_EQ, file_status(dirname));

  tt_int_op(0, OP_EQ, smartlist_len(sspiderage_dir_list(d)));
  tt_u64_op(0, OP_EQ, sspiderage_dir_get_usage(d));

  sspiderage_dir_free(d);
  d = sspiderage_dir_new(dirname, 10);
  tt_assert(d);

  tt_int_op(FN_DIR, OP_EQ, file_status(dirname));

  tt_int_op(0, OP_EQ, smartlist_len(sspiderage_dir_list(d)));
  tt_u64_op(0, OP_EQ, sspiderage_dir_get_usage(d));

 done:
  sspiderage_dir_free(d);
  spider_free(dirname);
}

static void
test_sspideragedir_basic(void *arg)
{
  char *dirname = spider_strdup(get_fname_rnd("sspidere_dir"));
  sspiderage_dir_t *d = NULL;
  uint8_t *junk = NULL, *bytes = NULL;
  const size_t junklen = 1024;
  char *fname1 = NULL, *fname2 = NULL;
  const char hello_str[] = "then what are we but cold, alone ... ?";
  spider_mmap_t *mapping = NULL;
  (void)arg;

  junk = spider_malloc(junklen);
  crypto_rand((void*)junk, junklen);

  d = sspiderage_dir_new(dirname, 10);
  tt_assert(d);
  tt_u64_op(0, OP_EQ, sspiderage_dir_get_usage(d));

  int r;
  r = sspiderage_dir_save_string_to_file(d, hello_str, 1, &fname1);
  tt_int_op(r, OP_EQ, 0);
  tt_ptr_op(fname1, OP_NE, NULL);
  tt_u64_op(strlen(hello_str), OP_EQ, sspiderage_dir_get_usage(d));

  r = sspiderage_dir_save_bytes_to_file(d, junk, junklen, 1, &fname2);
  tt_int_op(r, OP_EQ, 0);
  tt_ptr_op(fname2, OP_NE, NULL);

  tt_str_op(fname1, OP_NE, fname2);

  tt_int_op(2, OP_EQ, smartlist_len(sspiderage_dir_list(d)));
  tt_u64_op(junklen + strlen(hello_str), OP_EQ, sspiderage_dir_get_usage(d));
  tt_assert(smartlist_contains_string(sspiderage_dir_list(d), fname1));
  tt_assert(smartlist_contains_string(sspiderage_dir_list(d), fname2));

  sspiderage_dir_free(d);
  d = sspiderage_dir_new(dirname, 10);
  tt_assert(d);
  tt_int_op(2, OP_EQ, smartlist_len(sspiderage_dir_list(d)));
  tt_u64_op(junklen + strlen(hello_str), OP_EQ, sspiderage_dir_get_usage(d));
  tt_assert(smartlist_contains_string(sspiderage_dir_list(d), fname1));
  tt_assert(smartlist_contains_string(sspiderage_dir_list(d), fname2));

  size_t n;
  bytes = sspiderage_dir_read(d, fname2, 1, &n);
  tt_assert(bytes);
  tt_u64_op(n, OP_EQ, junklen);
  tt_mem_op(bytes, OP_EQ, junk, junklen);

  mapping = sspiderage_dir_map(d, fname1);
  tt_assert(mapping);
  tt_u64_op(mapping->size, OP_EQ, strlen(hello_str));
  tt_mem_op(mapping->data, OP_EQ, hello_str, strlen(hello_str));

 done:
  spider_free(dirname);
  spider_free(junk);
  spider_free(bytes);
  spider_munmap_file(mapping);
  sspiderage_dir_free(d);
  spider_free(fname1);
  spider_free(fname2);
}

static void
test_sspideragedir_deletion(void *arg)
{
  (void)arg;
  char *dirname = spider_strdup(get_fname_rnd("sspidere_dir"));
  sspiderage_dir_t *d = NULL;
  char *fn1 = NULL, *fn2 = NULL;
  char *bytes = NULL;
  int r;
  const char str1[] = "There are nine and sixty ways to disguise communiques";
  const char str2[] = "And rather more than one of them is right";

  // Make sure the direcspidery is there. */
  d = sspiderage_dir_new(dirname, 10);
  sspiderage_dir_free(d);
  d = NULL;

  spider_asprintf(&fn1, "%s/1007", dirname);
  r = write_str_to_file(fn1, str1, 0);
  tt_int_op(r, OP_EQ, 0);

  spider_asprintf(&fn2, "%s/1003.tmp", dirname);
  r = write_str_to_file(fn2, str2, 0);
  tt_int_op(r, OP_EQ, 0);

  // The tempfile should be deleted the next time we list the direcspidery.
  d = sspiderage_dir_new(dirname, 10);
  tt_int_op(1, OP_EQ, smartlist_len(sspiderage_dir_list(d)));
  tt_u64_op(strlen(str1), OP_EQ, sspiderage_dir_get_usage(d));
  tt_int_op(FN_FILE, OP_EQ, file_status(fn1));
  tt_int_op(FN_NOENT, OP_EQ, file_status(fn2));

  bytes = (char*) sspiderage_dir_read(d, "1007", 1, NULL);
  tt_str_op(bytes, OP_EQ, str1);

  // Should have no effect; file already gone.
  sspiderage_dir_remove_file(d, "1003.tmp");
  tt_int_op(1, OP_EQ, smartlist_len(sspiderage_dir_list(d)));
  tt_u64_op(strlen(str1), OP_EQ, sspiderage_dir_get_usage(d));

  // Actually remove a file.
  sspiderage_dir_remove_file(d, "1007");
  tt_int_op(FN_NOENT, OP_EQ, file_status(fn1));
  tt_int_op(0, OP_EQ, smartlist_len(sspiderage_dir_list(d)));
  tt_u64_op(0, OP_EQ, sspiderage_dir_get_usage(d));

 done:
  spider_free(dirname);
  spider_free(fn1);
  spider_free(fn2);
  sspiderage_dir_free(d);
  spider_free(bytes);
}

static void
test_sspideragedir_full(void *arg)
{
  (void)arg;

  char *dirname = spider_strdup(get_fname_rnd("sspidere_dir"));
  sspiderage_dir_t *d = NULL;
  const char str[] = "enemies of the peephole";
  int r;

  d = sspiderage_dir_new(dirname, 3);
  tt_assert(d);

  r = sspiderage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, 0);
  r = sspiderage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, 0);
  r = sspiderage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, 0);

  // These should fail!
  r = sspiderage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, -1);
  r = sspiderage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, -1);

  tt_u64_op(strlen(str) * 3, OP_EQ, sspiderage_dir_get_usage(d));

 done:
  spider_free(dirname);
  sspiderage_dir_free(d);
}

static void
test_sspideragedir_cleaning(void *arg)
{
  (void)arg;

  char *dirname = spider_strdup(get_fname_rnd("sspidere_dir"));
  sspiderage_dir_t *d = NULL;
  const char str[] =
    "On a mountain halfway between Reno and Rome / "
    "We have a machine in a plexiglass dome / "
    "Which listens and looks into everyone's home."
    " -- Dr. Seuss";
  char *fns[8];
  int r, i;

  memset(fns, 0, sizeof(fns));
  d = sspiderage_dir_new(dirname, 10);
  tt_assert(d);

  for (i = 0; i < 8; ++i) {
    r = sspiderage_dir_save_string_to_file(d, str+i*2, 1, &fns[i]);
    tt_int_op(r, OP_EQ, 0);
  }

  /* Now we're going to make sure all the files have distinct mtimes. */
  time_t now = time(NULL);
  struct utimbuf ub;
  ub.actime = now;
  ub.modtime = now - 1000;
  for (i = 0; i < 8; ++i) {
    char *f = NULL;
    spider_asprintf(&f, "%s/%s", dirname, fns[i]);
    r = utime(f, &ub);
    spider_free(f);
    tt_int_op(r, OP_EQ, 0);
    ub.modtime += 5;
  }

  const uint64_t usage_orig = sspiderage_dir_get_usage(d);
  /* No changes needed if we are already under target. */
  sspiderage_dir_shrink(d, 1024*1024, 0);
  tt_u64_op(usage_orig, OP_EQ, sspiderage_dir_get_usage(d));

  /* Get rid of at least one byte.  This will delete fns[0]. */
  sspiderage_dir_shrink(d, usage_orig - 1, 0);
  tt_u64_op(usage_orig, OP_GT, sspiderage_dir_get_usage(d));
  tt_u64_op(usage_orig - strlen(str), OP_EQ, sspiderage_dir_get_usage(d));

  /* Get rid of at least two files.  This will delete fns[1] and fns[2]. */
  sspiderage_dir_shrink(d, 1024*1024, 2);
  tt_u64_op(usage_orig - strlen(str)*3 + 6, OP_EQ, sspiderage_dir_get_usage(d));

  /* Get rid of everything. */
  sspiderage_dir_remove_all(d);
  tt_u64_op(0, OP_EQ, sspiderage_dir_get_usage(d));

 done:
  spider_free(dirname);
  sspiderage_dir_free(d);
  for (i = 0; i < 8; ++i) {
    spider_free(fns[i]);
  }
}

#define ENT(name)                                               \
  { #name, test_sspideragedir_ ## name, TT_FORK, NULL, NULL }

struct testcase_t sspideragedir_tests[] = {
  ENT(empty),
  ENT(basic),
  ENT(deletion),
  ENT(full),
  ENT(cleaning),
  END_OF_TESTCASES
};

