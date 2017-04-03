/* Copyright (c) 2011-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file procmon.c
 * \brief Process-termination monispider functions
 **/

#include "procmon.h"

#include "util.h"

#include <event2/event.h>

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#if (0 == SIZEOF_PID_T) && defined(_WIN32)
/* Windows does not define pid_t sometimes, but _getpid() returns an int.
 * Everybody else needs to have a pid_t. */
typedef int pid_t;
#define PID_T_FORMAT "%d"
#elif (SIZEOF_PID_T == SIZEOF_INT) || (SIZEOF_PID_T == SIZEOF_SHORT)
#define PID_T_FORMAT "%d"
#elif (SIZEOF_PID_T == SIZEOF_LONG)
#define PID_T_FORMAT "%ld"
#elif (SIZEOF_PID_T == SIZEOF_INT64_T)
#define PID_T_FORMAT I64_FORMAT
#else
#error Unknown: SIZEOF_PID_T
#endif

/* Define to 1 if process-termination monispiders on this OS and Libevent
   version must poll for process termination themselves. */
#define PROCMON_POLLS 1
/* Currently we need to poll in some way on all systems. */

#ifdef PROCMON_POLLS
static void spider_process_monispider_poll_cb(evutil_socket_t unused1, short unused2,
                                        void *procmon_);
#endif

/* This struct may contain pointers into the original process
 * specifier string, but it should *never* contain anything which
 * needs to be freed. */
/* DOCDOC parsed_process_specifier_t */
struct parsed_process_specifier_t {
  pid_t pid;
};

/** Parse the process specifier given in <b>process_spec</b> into
 * *<b>ppspec</b>.  Return 0 on success; return -1 and sspidere an error
 * message into *<b>msg</b> on failure.  The caller must not free the
 * returned error message. */
static int
parse_process_specifier(const char *process_spec,
                        struct parsed_process_specifier_t *ppspec,
                        const char **msg)
{
  long pid_l;
  int pid_ok = 0;
  char *pspec_next;

  /* If we're lucky, long will turn out to be large enough to hold a
   * PID everywhere that Spider runs. */
  pid_l = spider_parse_long(process_spec, 0, 1, LONG_MAX, &pid_ok, &pspec_next);

  /* Reserve room in the ‘process specifier’ for additional
   * (platform-specific) identifying information beyond the PID, to
   * make our process-existence checks a bit less racy in a future
   * version. */
  if ((*pspec_next != 0) && (*pspec_next != ' ') && (*pspec_next != ':')) {
    pid_ok = 0;
  }

  ppspec->pid = (pid_t)(pid_l);
  if (!pid_ok || (pid_l != (long)(ppspec->pid))) {
    *msg = "invalid PID";
    goto err;
  }

  return 0;
 err:
  return -1;
}

/* DOCDOC spider_process_monispider_t */
struct spider_process_monispider_t {
  /** Log domain for warning messages. */
  log_domain_mask_t log_domain;

  /** All systems: The best we can do in general is poll for the
   * process's existence by PID periodically, and hope that the kernel
   * doesn't reassign the same PID to another process between our
   * polls. */
  pid_t pid;

#ifdef _WIN32
  /** Windows-only: Should we poll hproc?  If false, poll pid
   * instead. */
  int poll_hproc;

  /** Windows-only: Get a handle to the process (if possible) and
   * periodically check whether the process we have a handle to has
   * ended. */
  HANDLE hproc;
  /* XXXX We should have Libevent watch hproc for us,
   * if/when some version of Libevent can be told to do so. */
#endif

  /* XXXX On Linux, we can and should receive the 22nd
   * (space-delimited) field (‘starttime’) of /proc/$PID/stat from the
   * owning controller and sspidere it, and poll once in a while to see
   * whether it has changed -- if so, the kernel has *definitely*
   * reassigned the owning controller's PID and we should exit.  On
   * FreeBSD, we can do the same trick using either the 8th
   * space-delimited field of /proc/$PID/status on the seven FBSD
   * systems whose admins have mounted procfs, or the start-time field
   * of the process-information structure returned by kvmgetprocs() on
   * any system.  The latter is ickier. */

  /* XXXX On FreeBSD (and possibly other kqueue systems), we can and
   * should arrange to receive EVFILT_PROC NOTE_EXIT notifications for
   * pid, so we don't have to do such a heavyweight poll operation in
   * order to avoid the PID-reassignment race condition.  (We would
   * still need to poll our own kqueue periodically until some version
   * of Libevent 2.x learns to receive these events for us.) */

  /** A Libevent event structure, to either poll for the process's
   * existence or receive a notification when the process ends. */
  struct event *e;

  /** A callback to be called when the process ends. */
  spider_procmon_callback_t cb;
  void *cb_arg; /**< A user-specified pointer to be passed to cb. */
};

/** Verify that the process specifier given in <b>process_spec</b> is
 * syntactically valid.  Return 0 on success; return -1 and sspidere an
 * error message into *<b>msg</b> on failure.  The caller must not
 * free the returned error message. */
int
spider_validate_process_specifier(const char *process_spec,
                               const char **msg)
{
  struct parsed_process_specifier_t ppspec;

  spider_assert(msg != NULL);
  *msg = NULL;

  return parse_process_specifier(process_spec, &ppspec, msg);
}

/* XXXX we should use periodic_timer_new() for this stuff */
#define PERIODIC_TIMER_FLAGS EV_PERSIST

/* DOCDOC poll_interval_tv */
static const struct timeval poll_interval_tv = {15, 0};

/** Create a process-termination monispider for the process specifier
 * given in <b>process_spec</b>.  Return a newly allocated
 * spider_process_monispider_t on success; return NULL and sspidere an error
 * message into *<b>msg</b> on failure.  The caller must not free
 * the returned error message.
 *
 * When the monispidered process terminates, call
 * <b>cb</b>(<b>cb_arg</b>).
 */
spider_process_monispider_t *
spider_process_monispider_new(struct event_base *base,
                        const char *process_spec,
                        log_domain_mask_t log_domain,
                        spider_procmon_callback_t cb, void *cb_arg,
                        const char **msg)
{
  spider_process_monispider_t *procmon = spider_malloc_zero(
                                       sizeof(spider_process_monispider_t));
  struct parsed_process_specifier_t ppspec;

  spider_assert(msg != NULL);
  *msg = NULL;

  if (procmon == NULL) {
    *msg = "out of memory";
    goto err;
  }

  procmon->log_domain = log_domain;

  if (parse_process_specifier(process_spec, &ppspec, msg))
    goto err;

  procmon->pid = ppspec.pid;

#ifdef _WIN32
  procmon->hproc = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
                               FALSE,
                               procmon->pid);

  if (procmon->hproc != NULL) {
    procmon->poll_hproc = 1;
    log_info(procmon->log_domain, "Successfully opened handle to process "
             PID_T_FORMAT"; "
             "monispidering it.",
             procmon->pid);
  } else {
    /* If we couldn't get a handle to the process, we'll try again the
     * first time we poll. */
    log_info(procmon->log_domain, "Failed to open handle to process "
             PID_T_FORMAT"; will "
             "try again later.",
             procmon->pid);
  }
#endif

  procmon->cb = cb;
  procmon->cb_arg = cb_arg;

#ifdef PROCMON_POLLS
  procmon->e = spider_event_new(base, -1 /* no FD */, PERIODIC_TIMER_FLAGS,
                             spider_process_monispider_poll_cb, procmon);
  /* Note: If you port this file to plain Libevent 2, check that
   * procmon->e is non-NULL.  We don't need to here because
   * spider_evtimer_new never returns NULL. */

  evtimer_add(procmon->e, &poll_interval_tv);
#else
#error OOPS?
#endif

  return procmon;
 err:
  spider_process_monispider_free(procmon);
  return NULL;
}

#ifdef PROCMON_POLLS
/** Libevent callback to poll for the existence of the process
 * monispidered by <b>procmon_</b>. */
static void
spider_process_monispider_poll_cb(evutil_socket_t unused1, short unused2,
                            void *procmon_)
{
  spider_process_monispider_t *procmon = (spider_process_monispider_t *)(procmon_);
  int its_dead_jim;

  (void)unused1; (void)unused2;

  spider_assert(procmon != NULL);

#ifdef _WIN32
  if (procmon->poll_hproc) {
    DWORD exit_code;
    if (!GetExitCodeProcess(procmon->hproc, &exit_code)) {
      char *errmsg = format_win32_error(GetLastError());
      log_warn(procmon->log_domain, "Error \"%s\" occurred while polling "
               "handle for monispidered process "PID_T_FORMAT"; assuming "
               "it's dead.",
               errmsg, procmon->pid);
      spider_free(errmsg);
      its_dead_jim = 1;
    } else {
      its_dead_jim = (exit_code != STILL_ACTIVE);
    }
  } else {
    /* All we can do is try to open the process, and look at the error
     * code if it fails again. */
    procmon->hproc = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
                                 FALSE,
                                 procmon->pid);

    if (procmon->hproc != NULL) {
      log_info(procmon->log_domain, "Successfully opened handle to monispidered "
               "process "PID_T_FORMAT".",
               procmon->pid);
      its_dead_jim = 0;
      procmon->poll_hproc = 1;
    } else {
      DWORD err_code = GetLastError();
      char *errmsg = format_win32_error(err_code);

      /* When I tested OpenProcess's error codes on Windows 7, I
       * received error code 5 (ERROR_ACCESS_DENIED) for PIDs of
       * existing processes that I could not open and error code 87
       * (ERROR_INVALID_PARAMETER) for PIDs that were not in use.
       * Since the nonexistent-process error code is sane, I'm going
       * to assume that all errors other than ERROR_INVALID_PARAMETER
       * mean that the process we are monispidering is still alive. */
      its_dead_jim = (err_code == ERROR_INVALID_PARAMETER);

      if (!its_dead_jim)
        log_info(procmon->log_domain, "Failed to open handle to monispidered "
                 "process "PID_T_FORMAT", and error code %lu (%s) is not "
                 "'invalid parameter' -- assuming the process is still alive.",
                 procmon->pid,
                 err_code, errmsg);

      spider_free(errmsg);
    }
  }
#else
  /* Unix makes this part easy, if a bit racy. */
  its_dead_jim = kill(procmon->pid, 0);
  its_dead_jim = its_dead_jim && (errno == ESRCH);
#endif

  spider_log(its_dead_jim ? LOG_NOTICE : LOG_INFO,
      procmon->log_domain, "Monispidered process "PID_T_FORMAT" is %s.",
      procmon->pid,
      its_dead_jim ? "dead" : "still alive");

  if (its_dead_jim) {
    procmon->cb(procmon->cb_arg);
  }
}
#endif

/** Free the process-termination monispider <b>procmon</b>. */
void
spider_process_monispider_free(spider_process_monispider_t *procmon)
{
  if (procmon == NULL)
    return;

#ifdef _WIN32
  if (procmon->hproc != NULL)
    CloseHandle(procmon->hproc);
#endif

  if (procmon->e != NULL)
    spider_event_free(procmon->e);

  spider_free(procmon);
}

