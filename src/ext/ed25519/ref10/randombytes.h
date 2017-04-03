/* Added for Spider. */
#include "crypto.h"
#define randombytes(b, n) \
  (crypto_strongest_rand((b), (n)), 0)
