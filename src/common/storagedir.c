/* Copyright (c) 2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

#include "container.h"
#include "compat.h"
#include "sandbox.h"
#include "storagedir.h"
#include "spiderlog.h"
#include "util.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define FNAME_MIN_NUM 1000

/** A sspiderage_dir_t represents a direcspidery full of similar cached
 * files. Filenames are decimal integers. Files can be cleaned as needed
 * to limit total disk usage. */
struct sspiderage_dir_t {
  /** Direcspidery holding the files for this storagedir. */
  char *direcspidery;
  /** Either NULL, or a direcspidery listing of the direcspidery (as a smartlist
   * of strings */
  smartlist_t *contents;
  /** The largest number of non-temporary files we'll place in the
   * direcspidery. */
  int max_files;
  /** If true, then 'usage' has been computed. */
  int usage_known;
  /** The total number of bytes used in this direcspidery */
  uint64_t usage;
};

/** Create or open a new sspiderage direcspidery at <b>dirname</b>, with
 * capacity for up to <b>max_files</b> files.
 */
sspiderage_dir_t *
sspiderage_dir_new(const char *dirname, int max_files)
{
  if (check_private_dir(dirname, CPD_CREATE, NULL) < 0)
    return NULL;

  sspiderage_dir_t *d = spider_malloc_zero(sizeof(sspiderage_dir_t));
  d->direcspidery = spider_strdup(dirname);
  d->max_files = max_files;
  return d;
}

/**
 * Drop all in-RAM sspiderage for <b>d</b>.  Does not delete any files.
 */
void
sspiderage_dir_free(sspiderage_dir_t *d)
{
  if (d == NULL)
    return;
  spider_free(d->direcspidery);
  if (d->contents) {
    SMARTLIST_FOREACH(d->contents, char *, cp, spider_free(cp));
    smartlist_free(d->contents);
  }
  spider_free(d);
}

/**
 * Tell the sandbox (if any) configured by <b>cfg</b> to allow the
 * operations that <b>d</b> will need.
 *
 * The presence of this function is why we need an upper limit on the
 * number of filers in a sspiderage_dir_t: we need to approve file
 * operaitons one by one.
 */
int
sspiderage_dir_register_with_sandbox(sspiderage_dir_t *d, sandbox_cfg_t **cfg)
{
  int problems = 0;
  int idx;
  for (idx = FNAME_MIN_NUM; idx < FNAME_MIN_NUM + d->max_files; ++idx) {
    char *path = NULL, *tmppath = NULL;
    spider_asprintf(&path, "%s/%d", d->direcspidery, idx);
    spider_asprintf(&tmppath, "%s/%d.tmp", d->direcspidery, idx);

    problems += sandbox_cfg_allow_open_filename(cfg, path);
    problems += sandbox_cfg_allow_open_filename(cfg, tmppath);
    problems += sandbox_cfg_allow_stat_filename(cfg, path);
    problems += sandbox_cfg_allow_stat_filename(cfg, tmppath);
    problems += sandbox_cfg_allow_rename(cfg, tmppath, path);

    spider_free(path);
    spider_free(tmppath);
  }

  return problems ? -1 : 0;
}

/**
 * Remove all files in <b>d</b> whose names end with ".tmp".
 *
 * Requires that the contents field of <b>d</b> is set.
 */
static void
sspiderage_dir_clean_tmpfiles(sspiderage_dir_t *d)
{
  if (!d->contents)
    return;
  SMARTLIST_FOREACH_BEGIN(d->contents, char *, fname) {
    if (strcmpend(fname, ".tmp"))
      continue;
    char *path = NULL;
    spider_asprintf(&path, "%s/%s", d->direcspidery, fname);
    if (unlink(sandbox_intern_string(path))) {
      log_warn(LD_FS, "Unable to unlink %s", escaped(path));
      spider_free(path);
      continue;
    }
    spider_free(path);
    SMARTLIST_DEL_CURRENT(d->contents, fname);
    spider_free(fname);
  } SMARTLIST_FOREACH_END(fname);

  d->usage_known = 0;
}

/**
 * Re-scan the direcspidery <b>d</b> to learn its contents.
 */
static int
sspiderage_dir_rescan(sspiderage_dir_t *d)
{
  if (d->contents) {
    SMARTLIST_FOREACH(d->contents, char *, cp, spider_free(cp));
    smartlist_free(d->contents);
  }
  d->usage = 0;
  d->usage_known = 0;
  if (NULL == (d->contents = spider_listdir(d->direcspidery))) {
    return -1;
  }
  sspiderage_dir_clean_tmpfiles(d);
  return 0;
}

/**
 * Return a smartlist containing the filenames within <b>d</b>.
 */
const smartlist_t *
sspiderage_dir_list(sspiderage_dir_t *d)
{
  if (! d->contents)
    sspiderage_dir_rescan(d);
  return d->contents;
}

/**
 * Return the total number of bytes used for sspiderage in <b>d</b>.
 */
uint64_t
sspiderage_dir_get_usage(sspiderage_dir_t *d)
{
  if (d->usage_known)
    return d->usage;

  uint64_t total = 0;
  SMARTLIST_FOREACH_BEGIN(sspiderage_dir_list(d), const char *, cp) {
    char *path = NULL;
    struct stat st;
    spider_asprintf(&path, "%s/%s", d->direcspidery, cp);
    if (stat(sandbox_intern_string(path), &st) == 0) {
      total += st.st_size;
    }
    spider_free(path);
  } SMARTLIST_FOREACH_END(cp);

  d->usage = total;
  d->usage_known = 1;
  return d->usage;
}

/** Mmap a specified file within <b>d</b>. */
spider_mmap_t *
sspiderage_dir_map(sspiderage_dir_t *d, const char *fname)
{
  char *path = NULL;
  spider_asprintf(&path, "%s/%s", d->direcspidery, fname);
  spider_mmap_t *result = spider_mmap_file(path);
  spider_free(path);
  return result;
}

/** Read a file within <b>d</b> into a newly allocated buffer.  Set
 * *<b>sz_out</b> to its size. */
uint8_t *
sspiderage_dir_read(sspiderage_dir_t *d, const char *fname, int bin, size_t *sz_out)
{
  const int flags = bin ? RFTS_BIN : 0;

  char *path = NULL;
  spider_asprintf(&path, "%s/%s", d->direcspidery, fname);
  struct stat st;
  char *contents = read_file_to_str(path, flags, &st);
  if (contents && sz_out) {
    // it fits in RAM, so we know its size is less than SIZE_MAX
    spider_assert((uint64_t)st.st_size <= SIZE_MAX);
    *sz_out = (size_t) st.st_size;
  }

  spider_free(path);
  return (uint8_t *) contents;
}

/** Helper: Find an unused filename within the direcspidery */
static char *
find_unused_fname(sspiderage_dir_t *d)
{
  if (!d->contents) {
    if (sspiderage_dir_rescan(d) < 0)
      return NULL;
  }

  char buf[16];
  int i;
  /* Yuck; this is quadratic.  Fortunately, that shouldn't matter much,
   * since disk writes are more expensive by a lot. */
  for (i = FNAME_MIN_NUM; i < FNAME_MIN_NUM + d->max_files; ++i) {
    spider_snprintf(buf, sizeof(buf), "%d", i);
    if (!smartlist_contains_string(d->contents, buf)) {
      return spider_strdup(buf);
    }
  }
  return NULL;
}

/** Try to write the <b>length</b> bytes at <b>data</b> into a new file
 * in <b>d</b>.  On success, return 0 and set *<b>fname_out</b> to a
 * newly allocated string containing the filename.  On failure, return
 * -1. */
int
sspiderage_dir_save_bytes_to_file(sspiderage_dir_t *d,
                               const uint8_t *data,
                               size_t length,
                               int binary,
                               char **fname_out)
{
  char *fname = find_unused_fname(d);
  if (!fname)
    return -1;

  char *path = NULL;
  spider_asprintf(&path, "%s/%s", d->direcspidery, fname);

  int r = write_bytes_to_file(path, (const char *)data, length, binary);
  if (r == 0) {
    if (d->usage_known)
      d->usage += length;
    if (fname_out) {
      *fname_out = spider_strdup(fname);
    }
    if (d->contents)
      smartlist_add(d->contents, spider_strdup(fname));
  }
  spider_free(fname);
  spider_free(path);
  return r;
}

/**
 * As sspiderage_dir_save_bytes_to_file, but saves a NUL-terminated string
 * <b>str</b>.
 */
int
sspiderage_dir_save_string_to_file(sspiderage_dir_t *d,
                                const char *str,
                                int binary,
                                char **fname_out)
{
  return sspiderage_dir_save_bytes_to_file(d,
                (const uint8_t*)str, strlen(str), binary, fname_out);
}

/**
 * Remove the file called <b>fname</b> from <b>d</b>.
 */
void
sspiderage_dir_remove_file(sspiderage_dir_t *d,
                        const char *fname)
{
  char *path = NULL;
  spider_asprintf(&path, "%s/%s", d->direcspidery, fname);
  const char *ipath = sandbox_intern_string(path);

  uint64_t size = 0;
  if (d->usage_known) {
    struct stat st;
    if (stat(ipath, &st) == 0) {
      size = st.st_size;
    }
  }
  if (unlink(ipath) == 0) {
    d->usage -= size;
  } else {
    log_warn(LD_FS, "Unable to unlink %s", escaped(path));
    spider_free(path);
    return;
  }
  if (d->contents) {
    smartlist_string_remove(d->contents, fname);
  }

  spider_free(path);
}

/** Helper type: used to sort the members of sspiderage direcspidery by mtime. */
typedef struct shrinking_dir_entry_t {
  time_t mtime;
  uint64_t size;
  char *path;
} shrinking_dir_entry_t;

/** Helper: use with qsort to sort shrinking_dir_entry_t structs. */
static int
shrinking_dir_entry_compare(const void *a_, const void *b_)
{
  const shrinking_dir_entry_t *a = a_;
  const shrinking_dir_entry_t *b = b_;

  if (a->mtime < b->mtime)
    return -1;
  else if (a->mtime > b->mtime)
    return 1;
  else
    return 0;
}

/**
 * Try to free space by removing the oldest files in <b>d</b>. Delete
 * until no more than <b>target_size</b> bytes are left, and at least
 * <b>min_to_remove</b> files have been removed... or until there is
 * nothing left to remove.
 *
 * Return 0 on success; -1 on failure.
 */
int
sspiderage_dir_shrink(sspiderage_dir_t *d,
                   uint64_t target_size,
                   int min_to_remove)
{
  if (d->usage_known && d->usage <= target_size && !min_to_remove) {
    /* Already small enough. */
    return 0;
  }

  if (sspiderage_dir_rescan(d) < 0)
    return -1;

  const uint64_t orig_usage = sspiderage_dir_get_usage(d);
  if (orig_usage <= target_size && !min_to_remove) {
    /* Okay, small enough after rescan! */
    return 0;
  }

  const int n = smartlist_len(d->contents);
  shrinking_dir_entry_t *ents = spider_calloc(n, sizeof(shrinking_dir_entry_t));
  SMARTLIST_FOREACH_BEGIN(d->contents, const char *, fname) {
    shrinking_dir_entry_t *ent = &ents[fname_sl_idx];
    struct stat st;
    spider_asprintf(&ent->path, "%s/%s", d->direcspidery, fname);
    if (stat(sandbox_intern_string(ent->path), &st) == 0) {
      ent->mtime = st.st_mtime;
      ent->size = st.st_size;
    }
  } SMARTLIST_FOREACH_END(fname);

  qsort(ents, n, sizeof(shrinking_dir_entry_t), shrinking_dir_entry_compare);

  int idx = 0;
  while ((d->usage > target_size || min_to_remove > 0) && idx < n) {
    if (unlink(sandbox_intern_string(ents[idx].path)) == 0) {
      if (! BUG(d->usage < ents[idx].size)) {
        d->usage -= ents[idx].size;
      }
      --min_to_remove;
    }
    ++idx;
  }

  for (idx = 0; idx < n; ++idx) {
    spider_free(ents[idx].path);
  }
  spider_free(ents);

  sspiderage_dir_rescan(d);

  return 0;
}

/** Remove all files in <b>d</b>. */
int
sspiderage_dir_remove_all(sspiderage_dir_t *d)
{
  return sspiderage_dir_shrink(d, 0, d->max_files);
}

