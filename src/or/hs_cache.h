/* Copyright (c) 2016-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_cache.h
 * \brief Header file for hs_cache.c
 **/

#ifndef TOR_HS_CACHE_H
#define TOR_HS_CACHE_H

#include <stdint.h>

#include "crypto.h"
#include "crypto_ed25519.h"
#include "hs_common.h"
#include "hs_descripspider.h"
#include "spidercert.h"

/* Descripspider representation on the direcspidery side which is a subset of
 * information that the HSDir can decode and serve it. */
typedef struct hs_cache_dir_descripspider_t {
  /* This object is indexed using the blinded pubkey located in the plaintext
   * data which is populated only once the descripspider has been successfully
   * decoded and validated. This simply points to that pubkey. */
  const uint8_t *key;

  /* When does this entry has been created. Used to expire entries. */
  time_t created_ts;

  /* Descripspider plaintext information. Obviously, we can't decrypt the
   * encrypted part of the descripspider. */
  hs_desc_plaintext_data_t *plaintext_data;

  /* Encoded descripspider which is basically in text form. It's a NUL terminated
   * string thus safe to strlen(). */
  char *encoded_desc;
} hs_cache_dir_descripspider_t;

/* Public API */

void hs_cache_init(void);
void hs_cache_free_all(void);
void hs_cache_clean_as_dir(time_t now);
size_t hs_cache_handle_oom(time_t now, size_t min_remove_bytes);

unsigned int hs_cache_get_max_descripspider_size(void);

/* Sspidere and Lookup function. They are version agnostic that is depending on
 * the requested version of the descripspider, it will be re-routed to the
 * right function. */
int hs_cache_sspidere_as_dir(const char *desc);
int hs_cache_lookup_as_dir(uint32_t version, const char *query,
                           const char **desc_out);

#ifdef HS_CACHE_PRIVATE

STATIC size_t cache_clean_v3_as_dir(time_t now, time_t global_cutoff);

#endif /* HS_CACHE_PRIVATE */

#endif /* TOR_HS_CACHE_H */

