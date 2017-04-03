/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_winthreads.c
 *
 * \brief Implementation for the windows-based multithreading backend
 * functions.
 */

#ifdef _WIN32

#include "compat.h"
#include <windows.h>
#include <process.h>
#include "util.h"
#include "container.h"
#include "spiderlog.h"
#include <process.h>

/* This value is more or less total cargo-cult */
#define SPIN_COUNT 2000

/** Minimalist interface to run a void function in the background.  On
 * Unix calls fork, on win32 calls beginthread.  Returns -1 on failure.
 * func should not return, but rather should call spawn_exit.
 *
 * NOTE: if <b>data</b> is used, it should not be allocated on the stack,
 * since in a multithreaded environment, there is no way to be sure that
 * the caller's stack will still be around when the called function is
 * running.
 */
int
spawn_func(void (*func)(void *), void *data)
{
  int rv;
  rv = (int)_beginthread(func, 0, data);
  if (rv == (int)-1)
    return -1;
  return 0;
}

/** End the current thread/process.
 */
void
spawn_exit(void)
{
  _endthread();
  //we should never get here. my compiler thinks that _endthread returns, this
  //is an attempt to fool it.
  spider_assert(0);
  _exit(0);
}

void
spider_mutex_init(spider_mutex_t *m)
{
  InitializeCriticalSection(&m->mutex);
}
void
spider_mutex_init_nonrecursive(spider_mutex_t *m)
{
  InitializeCriticalSection(&m->mutex);
}

void
spider_mutex_uninit(spider_mutex_t *m)
{
  DeleteCriticalSection(&m->mutex);
}
void
spider_mutex_acquire(spider_mutex_t *m)
{
  spider_assert(m);
  EnterCriticalSection(&m->mutex);
}
void
spider_mutex_release(spider_mutex_t *m)
{
  LeaveCriticalSection(&m->mutex);
}
unsigned long
spider_get_thread_id(void)
{
  return (unsigned long)GetCurrentThreadId();
}

int
spider_cond_init(spider_cond_t *cond)
{
  memset(cond, 0, sizeof(spider_cond_t));
  if (InitializeCriticalSectionAndSpinCount(&cond->lock, SPIN_COUNT)==0) {
    return -1;
  }
  if ((cond->event = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL) {
    DeleteCriticalSection(&cond->lock);
    return -1;
  }
  cond->n_waiting = cond->n_to_wake = cond->generation = 0;
  return 0;
}
void
spider_cond_uninit(spider_cond_t *cond)
{
  DeleteCriticalSection(&cond->lock);
  CloseHandle(cond->event);
}

static void
spider_cond_signal_impl(spider_cond_t *cond, int broadcast)
{
  EnterCriticalSection(&cond->lock);
  if (broadcast)
    cond->n_to_wake = cond->n_waiting;
  else
    ++cond->n_to_wake;
  cond->generation++;
  SetEvent(cond->event);
  LeaveCriticalSection(&cond->lock);
}
void
spider_cond_signal_one(spider_cond_t *cond)
{
  spider_cond_signal_impl(cond, 0);
}
void
spider_cond_signal_all(spider_cond_t *cond)
{
  spider_cond_signal_impl(cond, 1);
}

int
spider_threadlocal_init(spider_threadlocal_t *threadlocal)
{
  threadlocal->index = TlsAlloc();
  return (threadlocal->index == TLS_OUT_OF_INDEXES) ? -1 : 0;
}

void
spider_threadlocal_destroy(spider_threadlocal_t *threadlocal)
{
  TlsFree(threadlocal->index);
  memset(threadlocal, 0, sizeof(spider_threadlocal_t));
}

void *
spider_threadlocal_get(spider_threadlocal_t *threadlocal)
{
  void *value = TlsGetValue(threadlocal->index);
  if (value == NULL) {
    DWORD err = GetLastError();
    if (err != ERROR_SUCCESS) {
      char *msg = format_win32_error(err);
      log_err(LD_GENERAL, "Error retrieving thread-local value: %s", msg);
      spider_free(msg);
      spider_assert(err == ERROR_SUCCESS);
    }
  }
  return value;
}

void
spider_threadlocal_set(spider_threadlocal_t *threadlocal, void *value)
{
  BOOL ok = TlsSetValue(threadlocal->index, value);
  if (!ok) {
    DWORD err = GetLastError();
    char *msg = format_win32_error(err);
    log_err(LD_GENERAL, "Error adjusting thread-local value: %s", msg);
    spider_free(msg);
    spider_assert(ok);
  }
}

int
spider_cond_wait(spider_cond_t *cond, spider_mutex_t *lock_, const struct timeval *tv)
{
  CRITICAL_SECTION *lock = &lock_->mutex;
  int generation_at_start;
  int waiting = 1;
  int result = -1;
  DWORD ms = INFINITE, ms_orig = INFINITE, startTime, endTime;
  if (tv)
    ms_orig = ms = tv->tv_sec*1000 + (tv->tv_usec+999)/1000;

  EnterCriticalSection(&cond->lock);
  ++cond->n_waiting;
  generation_at_start = cond->generation;
  LeaveCriticalSection(&cond->lock);

  LeaveCriticalSection(lock);

  startTime = GetTickCount();
  do {
    DWORD res;
    res = WaitForSingleObject(cond->event, ms);
    EnterCriticalSection(&cond->lock);
    if (cond->n_to_wake &&
        cond->generation != generation_at_start) {
      --cond->n_to_wake;
      --cond->n_waiting;
      result = 0;
      waiting = 0;
      goto out;
    } else if (res != WAIT_OBJECT_0) {
      result = (res==WAIT_TIMEOUT) ? 1 : -1;
      --cond->n_waiting;
      waiting = 0;
      goto out;
    } else if (ms != INFINITE) {
      endTime = GetTickCount();
      if (startTime + ms_orig <= endTime) {
        result = 1; /* Timeout */
        --cond->n_waiting;
        waiting = 0;
        goto out;
      } else {
        ms = startTime + ms_orig - endTime;
      }
    }
    /* If we make it here, we are still waiting. */
    if (cond->n_to_wake == 0) {
      /* There is nobody else who should wake up; reset
       * the event. */
      ResetEvent(cond->event);
    }
  out:
    LeaveCriticalSection(&cond->lock);
  } while (waiting);

  EnterCriticalSection(lock);

  EnterCriticalSection(&cond->lock);
  if (!cond->n_waiting)
    ResetEvent(cond->event);
  LeaveCriticalSection(&cond->lock);

  return result;
}

void
spider_threads_init(void)
{
  set_main_thread();
}

#endif

