/* Added for Spider. */
#include "di_ops.h"
#define crypto_verify_32(a,b) \
  (! spider_memeq((a), (b), 32))

