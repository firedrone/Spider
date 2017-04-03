/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_COMPAT_THREADS_H
#define TOR_COMPAT_THREADS_H

#include "orconfig.h"
#include "spiderint.h"
#include "testsupport.h"

#if defined(HAVE_PTHREAD_H) && !defined(_WIN32)
#include <pthread.h>
#endif

#if defined(_WIN32)
#define USE_WIN32_THREADS
#elif defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_CREATE)
#define USE_PTHREADS
#else
#error "No threading system was found"
#endif

int spawn_func(void (*func)(void *), void *data);
void spawn_exit(void) ATTR_NORETURN;

/* Because we use threads instead of processes on most platforms (Windows,
 * Linux, etc), we need locking for them.  On platforms with poor thread
 * support or broken gethostbyname_r, these functions are no-ops. */

/** A generic lock structure for multithreaded builds. */
typedef struct spider_mutex_t {
#if defined(USE_WIN32_THREADS)
  /** Windows-only: on windows, we implement locks with CRITICAL_SECTIONS. */
  CRITICAL_SECTION mutex;
#elif defined(USE_PTHREADS)
  /** Pthreads-only: with pthreads, we implement locks with
   * pthread_mutex_t. */
  pthread_mutex_t mutex;
#else
  /** No-threads only: Dummy variable so that spider_mutex_t takes up space. */
  int _unused;
#endif
} spider_mutex_t;

spider_mutex_t *spider_mutex_new(void);
spider_mutex_t *spider_mutex_new_nonrecursive(void);
void spider_mutex_init(spider_mutex_t *m);
void spider_mutex_init_nonrecursive(spider_mutex_t *m);
void spider_mutex_acquire(spider_mutex_t *m);
void spider_mutex_release(spider_mutex_t *m);
void spider_mutex_free(spider_mutex_t *m);
void spider_mutex_uninit(spider_mutex_t *m);
unsigned long spider_get_thread_id(void);
void spider_threads_init(void);

/** Conditions need nonrecursive mutexes with pthreads. */
#define spider_mutex_init_for_cond(m) spider_mutex_init_nonrecursive(m)

void set_main_thread(void);
int in_main_thread(void);

typedef struct spider_cond_t {
#ifdef USE_PTHREADS
  pthread_cond_t cond;
#elif defined(USE_WIN32_THREADS)
  HANDLE event;

  CRITICAL_SECTION lock;
  int n_waiting;
  int n_to_wake;
  int generation;
#else
#error no known condition implementation.
#endif
} spider_cond_t;

spider_cond_t *spider_cond_new(void);
void spider_cond_free(spider_cond_t *cond);
int spider_cond_init(spider_cond_t *cond);
void spider_cond_uninit(spider_cond_t *cond);
int spider_cond_wait(spider_cond_t *cond, spider_mutex_t *mutex,
                  const struct timeval *tv);
void spider_cond_signal_one(spider_cond_t *cond);
void spider_cond_signal_all(spider_cond_t *cond);

/** Helper type used to manage waking up the main thread while it's in
 * the libevent main loop.  Used by the work queue code. */
typedef struct alert_sockets_s {
  /* XXXX This structure needs a better name. */
  /** Socket that the main thread should listen for EV_READ events on.
   * Note that this socket may be a regular fd on a non-Windows platform.
   */
  spider_socket_t read_fd;
  /** Socket to use when alerting the main thread. */
  spider_socket_t write_fd;
  /** Function to alert the main thread */
  int (*alert_fn)(spider_socket_t write_fd);
  /** Function to make the main thread no longer alerted. */
  int (*drain_fn)(spider_socket_t read_fd);
} alert_sockets_t;

/* Flags to disable one or more alert_sockets backends. */
#define ASOCKS_NOEVENTFD2   (1u<<0)
#define ASOCKS_NOEVENTFD    (1u<<1)
#define ASOCKS_NOPIPE2      (1u<<2)
#define ASOCKS_NOPIPE       (1u<<3)
#define ASOCKS_NOSOCKETPAIR (1u<<4)

int alert_sockets_create(alert_sockets_t *socks_out, uint32_t flags);
void alert_sockets_close(alert_sockets_t *socks);

typedef struct spider_threadlocal_s {
#ifdef _WIN32
  DWORD index;
#else
  pthread_key_t key;
#endif
} spider_threadlocal_t;

/** Initialize a thread-local variable.
 *
 * After you call this function on a spider_threadlocal_t, you can call
 * spider_threadlocal_set to change the current value of this variable for the
 * current thread, and spider_threadlocal_get to retrieve the current value for
 * the current thread.  Each thread has its own value.
 **/
int spider_threadlocal_init(spider_threadlocal_t *threadlocal);
/**
 * Release all resource associated with a thread-local variable.
 */
void spider_threadlocal_destroy(spider_threadlocal_t *threadlocal);
/**
 * Return the current value of a thread-local variable for this thread.
 *
 * It's undefined behavior to use this function if the threadlocal hasn't
 * been initialized, or has been destroyed.
 */
void *spider_threadlocal_get(spider_threadlocal_t *threadlocal);
/**
 * Change the current value of a thread-local variable for this thread to
 * <b>value</b>.
 *
 * It's undefined behavior to use this function if the threadlocal hasn't
 * been initialized, or has been destroyed.
 */
void spider_threadlocal_set(spider_threadlocal_t *threadlocal, void *value);

#endif

