// Use calloc or realloc as appropriate instead of multiply-and-alloc

@malloc_to_calloc@
identifier f =~ "(spider_malloc|spider_malloc_zero)";
expression a;
constant b;
@@
- f(a * b)
+ spider_calloc(a, b)

@calloc_arg_order@
expression a;
type t;
@@
- spider_calloc(sizeof(t), a)
+ spider_calloc(a, sizeof(t))

@realloc_to_reallocarray@
expression a, b;
expression p;
@@
- spider_realloc(p, a * b)
+ spider_reallocarray(p, a, b)
