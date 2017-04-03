@cast_malloc@
expression e;
type T;
@@
- (T *)spider_malloc(e)
+ spider_malloc(e)

@cast_malloc_zero@
expression e;
type T;
identifier func;
@@
- (T *)spider_malloc_zero(e)
+ spider_malloc_zero(e)

@cast_calloc@
expression a, b;
type T;
identifier func;
@@
- (T *)spider_calloc(a, b)
+ spider_calloc(a, b)

@cast_realloc@
expression e;
expression p;
type T;
@@
- (T *)spider_realloc(p, e)
+ spider_realloc(p, e)

@cast_reallocarray@
expression a,b;
expression p;
type T;
@@
- (T *)spider_reallocarray(p, a, b)
+ spider_reallocarray(p, a, b)
