
#ifndef TRUNNEL_LOCAL_H_INCLUDED
#define TRUNNEL_LOCAL_H_INCLUDED

#include "util.h"
#include "compat.h"
#include "crypto.h"

#define trunnel_malloc spider_malloc
#define trunnel_calloc spider_calloc
#define trunnel_strdup spider_strdup
#define trunnel_free_ spider_free_
#define trunnel_realloc spider_realloc
#define trunnel_reallocarray spider_reallocarray
#define trunnel_assert spider_assert
#define trunnel_memwipe(mem, len) memwipe((mem), 0, (len))

#endif
