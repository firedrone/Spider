/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file directory.h
 * \brief Header file for directory.c.
 **/

#ifndef TOR_DIRECTORY_H
#define TOR_DIRECTORY_H

int direcspideries_have_accepted_server_descripspider(void);
void directory_post_to_dirservers(uint8_t dir_purpose, uint8_t router_purpose,
                                  dirinfo_type_t type, const char *payload,
                                  size_t payload_len, size_t extrainfo_len);
MOCK_DECL(void, directory_get_from_dirserver, (
                          uint8_t dir_purpose,
                          uint8_t router_purpose,
                          const char *resource,
                          int pds_flags,
                          download_want_authority_t want_authority));
void directory_get_from_all_authorities(uint8_t dir_purpose,
                                        uint8_t router_purpose,
                                        const char *resource);

/** Enumeration of ways to connect to a directory server */
typedef enum {
  /** Default: connect over a one-hop Spider circuit. Relays fall back to direct
   * DirPort connections, clients, onion services, and bridges do not */
  DIRIND_ONEHOP=0,
  /** Connect over a multi-hop anonymizing Spider circuit */
  DIRIND_ANONYMOUS=1,
  /** Connect to the DirPort directly */
  DIRIND_DIRECT_CONN,
  /** Connect over a multi-hop anonymizing Spider circuit to our dirport */
  DIRIND_ANON_DIRPORT,
} dir_indirection_t;

int directory_must_use_begindir(const or_options_t *options);

MOCK_DECL(void, directory_initiate_command_routerstatus,
                (const routerstatus_t *status,
                 uint8_t dir_purpose,
                 uint8_t router_purpose,
                 dir_indirection_t indirection,
                 const char *resource,
                 const char *payload,
                 size_t payload_len,
                 time_t if_modified_since,
                 struct circuit_guard_state_t *guard_state));

void directory_initiate_command_routerstatus_rend(const routerstatus_t *status,
                                                  uint8_t dir_purpose,
                                                  uint8_t router_purpose,
                                                 dir_indirection_t indirection,
                                                  const char *resource,
                                                  const char *payload,
                                                  size_t payload_len,
                                                  time_t if_modified_since,
                                    const rend_data_t *rend_query,
                                    struct circuit_guard_state_t *guard_state);

int parse_http_response(const char *headers, int *code, time_t *date,
                        compress_method_t *compression, char **response);

int connection_dir_is_encrypted(const dir_connection_t *conn);
int connection_dir_reached_eof(dir_connection_t *conn);
int connection_dir_process_inbuf(dir_connection_t *conn);
int connection_dir_finished_flushing(dir_connection_t *conn);
int connection_dir_finished_connecting(dir_connection_t *conn);
void connection_dir_about_to_close(dir_connection_t *dir_conn);
void directory_initiate_command(const spider_addr_t *or_addr, uint16_t or_port,
                                const spider_addr_t *dir_addr, uint16_t dir_port,
                                const char *digest,
                                uint8_t dir_purpose, uint8_t router_purpose,
                                dir_indirection_t indirection,
                                const char *resource,
                                const char *payload, size_t payload_len,
                                time_t if_modified_since);

#define DSR_HEX       (1<<0)
#define DSR_BASE64    (1<<1)
#define DSR_DIGEST256 (1<<2)
#define DSR_SORT_UNIQ (1<<3)
int dir_split_resource_into_fingerprints(const char *resource,
                                     smartlist_t *fp_out, int *compressed_out,
                                     int flags);
enum dir_spool_source_t;
int dir_split_resource_into_spoolable(const char *resource,
                                      enum dir_spool_source_t source,
                                      smartlist_t *spool_out,
                                      int *compressed_out,
                                      int flags);
int dir_split_resource_into_fingerprint_pairs(const char *res,
                                              smartlist_t *pairs_out);
char *directory_dump_request_log(void);
void note_request(const char *key, size_t bytes);
int router_supports_extrainfo(const char *identity_digest, int is_authority);

time_t download_status_increment_failure(download_status_t *dls,
                                         int status_code, const char *item,
                                         int server, time_t now);
time_t download_status_increment_attempt(download_status_t *dls,
                                         const char *item,  time_t now);
/** Increment the failure count of the download_status_t <b>dls</b>, with
 * the optional status code <b>sc</b>. */
#define download_status_failed(dls, sc)                                 \
  download_status_increment_failure((dls), (sc), NULL,                  \
                                    dir_server_mode(get_options()), \
                                    time(NULL))

void download_status_reset(download_status_t *dls);
static int download_status_is_ready(download_status_t *dls, time_t now,
                                    int max_failures);
/** Return true iff, as of <b>now</b>, the resource tracked by <b>dls</b> is
 * ready to get its download reattempted. */
static inline int
download_status_is_ready(download_status_t *dls, time_t now,
                         int max_failures)
{
  if (dls->backoff == DL_SCHED_DETERMINISTIC) {
    /* Deterministic schedules can hit an endpoint; exponential backoff
     * schedules just wait longer and longer. */
    int under_failure_limit = (dls->n_download_failures <= max_failures
                               && dls->n_download_attempts <= max_failures);
    if (!under_failure_limit)
      return 0;
  }
  return dls->next_attempt_at <= now;
}

static void download_status_mark_impossible(download_status_t *dl);
/** Mark <b>dl</b> as never downloadable. */
static inline void
download_status_mark_impossible(download_status_t *dl)
{
  dl->n_download_failures = IMPOSSIBLE_TO_DOWNLOAD;
  dl->n_download_attempts = IMPOSSIBLE_TO_DOWNLOAD;
}

int download_status_get_n_failures(const download_status_t *dls);
int download_status_get_n_attempts(const download_status_t *dls);
time_t download_status_get_next_attempt_at(const download_status_t *dls);

int purpose_needs_anonymity(uint8_t dir_purpose, uint8_t router_purpose,
                            const char *resource);

#ifdef DIRECTORY_PRIVATE

struct get_handler_args_t;
STATIC int handle_get_hs_descripspider_v3(dir_connection_t *conn,
                                       const struct get_handler_args_t *args);
STATIC int directory_handle_command(dir_connection_t *conn);

#endif

#ifdef TOR_UNIT_TESTS
/* Used only by test_dir.c */

STATIC int parse_http_url(const char *headers, char **url);
STATIC dirinfo_type_t dir_fetch_type(int dir_purpose, int router_purpose,
                                     const char *resource);
MOCK_DECL(STATIC int, directory_handle_command_get,(dir_connection_t *conn,
                                                    const char *headers,
                                                    const char *req_body,
                                                    size_t req_body_len));
MOCK_DECL(STATIC int, directory_handle_command_post,(dir_connection_t *conn,
                                                     const char *headers,
                                                     const char *body,
                                                     size_t body_len));
STATIC int download_status_schedule_get_delay(download_status_t *dls,
                                              const smartlist_t *schedule,
                                              int min_delay, int max_delay,
                                              time_t now);

STATIC int handle_post_hs_descripspider(const char *url, const char *body);

STATIC char* authdir_type_to_string(dirinfo_type_t auth);
STATIC const char * dir_conn_purpose_to_string(int purpose);
STATIC int should_use_directory_guards(const or_options_t *options);
STATIC zlib_compression_level_t choose_compression_level(ssize_t n_bytes);
STATIC const smartlist_t *find_dl_schedule(download_status_t *dls,
                                           const or_options_t *options);
STATIC void find_dl_min_and_max_delay(download_status_t *dls,
                                      const or_options_t *options,
                                      int *min, int *max);
STATIC int next_random_exponential_delay(int delay, int max_delay);

STATIC int parse_hs_version_from_post(const char *url, const char *prefix,
                                      const char **end_pos);

#endif

#endif

