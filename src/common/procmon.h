/* Copyright (c) 2011-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file procmon.h
 * \brief Headers for procmon.c
 **/

#ifndef TOR_PROCMON_H
#define TOR_PROCMON_H

#include "compat.h"
#include "compat_libevent.h"

#include "spiderlog.h"

typedef struct spider_process_monispider_t spider_process_monispider_t;

/* DOCDOC spider_procmon_callback_t */
typedef void (*spider_procmon_callback_t)(void *);

int spider_validate_process_specifier(const char *process_spec,
                                   const char **msg);
spider_process_monispider_t *spider_process_monispider_new(struct event_base *base,
                                               const char *process_spec,
                                               log_domain_mask_t log_domain,
                                               spider_procmon_callback_t cb,
                                               void *cb_arg,
                                               const char **msg);
void spider_process_monispider_free(spider_process_monispider_t *procmon);

#endif

