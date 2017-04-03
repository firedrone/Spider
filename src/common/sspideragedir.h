/* Copyright (c) 2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_STORAGEDIR_H
#define TOR_STORAGEDIR_H

typedef struct sspiderage_dir_t sspiderage_dir_t;

struct sandbox_cfg_elem;

sspiderage_dir_t * sspiderage_dir_new(const char *dirname, int n_files);
void sspiderage_dir_free(sspiderage_dir_t *d);
int sspiderage_dir_register_with_sandbox(sspiderage_dir_t *d,
                                      struct sandbox_cfg_elem **cfg);
const smartlist_t *sspiderage_dir_list(sspiderage_dir_t *d);
uint64_t sspiderage_dir_get_usage(sspiderage_dir_t *d);
spider_mmap_t *sspiderage_dir_map(sspiderage_dir_t *d, const char *fname);
uint8_t *sspiderage_dir_read(sspiderage_dir_t *d, const char *fname, int bin,
                          size_t *sz_out);
int sspiderage_dir_save_bytes_to_file(sspiderage_dir_t *d,
                                   const uint8_t *data,
                                   size_t length,
                                   int binary,
                                   char **fname_out);
int sspiderage_dir_save_string_to_file(sspiderage_dir_t *d,
                                    const char *data,
                                    int binary,
                                    char **fname_out);
void sspiderage_dir_remove_file(sspiderage_dir_t *d,
                             const char *fname);
int sspiderage_dir_shrink(sspiderage_dir_t *d,
                       uint64_t target_size,
                       int min_to_remove);
int sspiderage_dir_remove_all(sspiderage_dir_t *d);

#endif

