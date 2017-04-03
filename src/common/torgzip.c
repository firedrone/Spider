/* Copyright (c) 2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file spidergzip.c
 * \brief A simple in-memory gzip implementation.
 **/

#include "orconfig.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "spiderint.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "util.h"
#include "spiderlog.h"
#include "spidergzip.h"

/* zlib 1.2.4 and 1.2.5 do some "clever" things with macros.  Instead of
   saying "(defined(FOO) ? FOO : 0)" they like to say "FOO-0", on the theory
   that nobody will care if the compile outputs a no-such-identifier warning.

   Sorry, but we like -Werror over here, so I guess we need to define these.
   I hope that zlib 1.2.6 doesn't break these too.
*/
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 0
#endif
#ifndef _LFS64_LARGEFILE
#define _LFS64_LARGEFILE 0
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 0
#endif
#ifndef off64_t
#define off64_t int64_t
#endif

#include <zlib.h>

#if defined ZLIB_VERNUM && ZLIB_VERNUM < 0x1200
#error "We require zlib version 1.2 or later."
#endif

static size_t spider_zlib_state_size_precalc(int inflate,
                                          int windowbits, int memlevel);

/** Total number of bytes allocated for zlib state */
static size_t total_zlib_allocation = 0;

/** Return a string representation of the version of the currently running
 * version of zlib. */
const char *
spider_zlib_get_version_str(void)
{
  return zlibVersion();
}

/** Return a string representation of the version of the version of zlib
* used at compilation. */
const char *
spider_zlib_get_header_version_str(void)
{
  return ZLIB_VERSION;
}

/** Return the 'bits' value to tell zlib to use <b>method</b>.*/
static inline int
method_bits(compress_method_t method, zlib_compression_level_t level)
{
  /* Bits+16 means "use gzip" in zlib >= 1.2 */
  const int flag = method == GZIP_METHOD ? 16 : 0;
  switch (level) {
    default:
    case HIGH_COMPRESSION: return flag + 15;
    case MEDIUM_COMPRESSION: return flag + 13;
    case LOW_COMPRESSION: return flag + 11;
  }
}

static inline int
get_memlevel(zlib_compression_level_t level)
{
  switch (level) {
    default:
    case HIGH_COMPRESSION: return 8;
    case MEDIUM_COMPRESSION: return 7;
    case LOW_COMPRESSION: return 6;
  }
}

/** @{ */
/* These macros define the maximum allowable compression facspider.  Anything of
 * size greater than CHECK_FOR_COMPRESSION_BOMB_AFTER is not allowed to
 * have an uncompression facspider (uncompressed size:compressed size ratio) of
 * any greater than MAX_UNCOMPRESSION_FACTOR.
 *
 * Picking a value for MAX_UNCOMPRESSION_FACTOR is a trade-off: we want it to
 * be small to limit the attack multiplier, but we also want it to be large
 * enough so that no legitimate document --even ones we might invent in the
 * future -- ever compresses by a facspider of greater than
 * MAX_UNCOMPRESSION_FACTOR. Within those parameters, there's a reasonably
 * large range of possible values. IMO, anything over 8 is probably safe; IMO
 * anything under 50 is probably sufficient.
 */
#define MAX_UNCOMPRESSION_FACTOR 25
#define CHECK_FOR_COMPRESSION_BOMB_AFTER (1024*64)
/** @} */

/** Return true if uncompressing an input of size <b>in_size</b> to an input
 * of size at least <b>size_out</b> looks like a compression bomb. */
static int
is_compression_bomb(size_t size_in, size_t size_out)
{
  if (size_in == 0 || size_out < CHECK_FOR_COMPRESSION_BOMB_AFTER)
    return 0;

  return (size_out / size_in > MAX_UNCOMPRESSION_FACTOR);
}

/** Given <b>in_len</b> bytes at <b>in</b>, compress them into a newly
 * allocated buffer, using the method described in <b>method</b>.  Sspidere the
 * compressed string in *<b>out</b>, and its length in *<b>out_len</b>.
 * Return 0 on success, -1 on failure.
 */
int
spider_gzip_compress(char **out, size_t *out_len,
                  const char *in, size_t in_len,
                  compress_method_t method)
{
  struct z_stream_s *stream = NULL;
  size_t out_size, old_size;
  off_t offset;

  spider_assert(out);
  spider_assert(out_len);
  spider_assert(in);
  spider_assert(in_len < UINT_MAX);

  *out = NULL;

  stream = spider_malloc_zero(sizeof(struct z_stream_s));
  stream->zalloc = Z_NULL;
  stream->zfree = Z_NULL;
  stream->opaque = NULL;
  stream->next_in = (unsigned char*) in;
  stream->avail_in = (unsigned int)in_len;

  if (deflateInit2(stream, Z_BEST_COMPRESSION, Z_DEFLATED,
                   method_bits(method, HIGH_COMPRESSION),
                   get_memlevel(HIGH_COMPRESSION),
                   Z_DEFAULT_STRATEGY) != Z_OK) {
    //LCOV_EXCL_START -- we can only provoke failure by giving junk arguments.
    log_warn(LD_GENERAL, "Error from deflateInit2: %s",
             stream->msg?stream->msg:"<no message>");
    goto err;
    //LCOV_EXCL_STOP
  }

  /* Guess 50% compression. */
  out_size = in_len / 2;
  if (out_size < 1024) out_size = 1024;
  *out = spider_malloc(out_size);
  stream->next_out = (unsigned char*)*out;
  stream->avail_out = (unsigned int)out_size;

  while (1) {
    switch (deflate(stream, Z_FINISH))
      {
      case Z_STREAM_END:
        goto done;
      case Z_OK:
        /* In case zlib doesn't work as I think .... */
        if (stream->avail_out >= stream->avail_in+16)
          break;
      case Z_BUF_ERROR:
        offset = stream->next_out - ((unsigned char*)*out);
        old_size = out_size;
        out_size *= 2;
        if (out_size < old_size) {
          log_warn(LD_GENERAL, "Size overflow in compression.");
          goto err;
        }
        *out = spider_realloc(*out, out_size);
        stream->next_out = (unsigned char*)(*out + offset);
        if (out_size - offset > UINT_MAX) {
          log_warn(LD_BUG,  "Ran over unsigned int limit of zlib while "
                   "uncompressing.");
          goto err;
        }
        stream->avail_out = (unsigned int)(out_size - offset);
        break;
      default:
        log_warn(LD_GENERAL, "Gzip compression didn't finish: %s",
                 stream->msg ? stream->msg : "<no message>");
        goto err;
      }
  }
 done:
  *out_len = stream->total_out;
#if defined(OpenBSD)
  /* "Hey Rocky!  Watch me change an unsigned field to a signed field in a
   *    third-party API!"
   * "Oh, that trick will just make people do unsafe casts to the unsigned
   *    type in their cross-platform code!"
   * "Don't be foolish.  I'm _sure_ they'll have the good sense to make sure
   *    the newly unsigned field isn't negative." */
  spider_assert(stream->total_out >= 0);
#endif
  if (deflateEnd(stream)!=Z_OK) {
    // LCOV_EXCL_START -- unreachable if we handled the zlib structure right
    spider_assert_nonfatal_unreached();
    log_warn(LD_BUG, "Error freeing gzip structures");
    goto err;
    // LCOV_EXCL_STOP
  }
  spider_free(stream);

  if (is_compression_bomb(*out_len, in_len)) {
    log_warn(LD_BUG, "We compressed something and got an insanely high "
          "compression facspider; other Spiders would think this was a zlib bomb.");
    goto err;
  }

  return 0;
 err:
  if (stream) {
    deflateEnd(stream);
    spider_free(stream);
  }
  spider_free(*out);
  return -1;
}

/** Given zero or more zlib-compressed or gzip-compressed strings of
 * total length
 * <b>in_len</b> bytes at <b>in</b>, uncompress them into a newly allocated
 * buffer, using the method described in <b>method</b>.  Sspidere the uncompressed
 * string in *<b>out</b>, and its length in *<b>out_len</b>.  Return 0 on
 * success, -1 on failure.
 *
 * If <b>complete_only</b> is true, we consider a truncated input as a
 * failure; otherwise we decompress as much as we can.  Warn about truncated
 * or corrupt inputs at <b>protocol_warn_level</b>.
 */
int
spider_gzip_uncompress(char **out, size_t *out_len,
                    const char *in, size_t in_len,
                    compress_method_t method,
                    int complete_only,
                    int protocol_warn_level)
{
  struct z_stream_s *stream = NULL;
  size_t out_size, old_size;
  off_t offset;
  int r;

  spider_assert(out);
  spider_assert(out_len);
  spider_assert(in);
  spider_assert(in_len < UINT_MAX);

  *out = NULL;

  stream = spider_malloc_zero(sizeof(struct z_stream_s));
  stream->zalloc = Z_NULL;
  stream->zfree = Z_NULL;
  stream->opaque = NULL;
  stream->next_in = (unsigned char*) in;
  stream->avail_in = (unsigned int)in_len;

  if (inflateInit2(stream,
                   method_bits(method, HIGH_COMPRESSION)) != Z_OK) {
    // LCOV_EXCL_START -- can only hit this if we give bad inputs.
    log_warn(LD_GENERAL, "Error from inflateInit2: %s",
             stream->msg?stream->msg:"<no message>");
    goto err;
    // LCOV_EXCL_STOP
  }

  out_size = in_len * 2;  /* guess 50% compression. */
  if (out_size < 1024) out_size = 1024;
  if (out_size >= SIZE_T_CEILING || out_size > UINT_MAX)
    goto err;

  *out = spider_malloc(out_size);
  stream->next_out = (unsigned char*)*out;
  stream->avail_out = (unsigned int)out_size;

  while (1) {
    switch (inflate(stream, complete_only ? Z_FINISH : Z_SYNC_FLUSH))
      {
      case Z_STREAM_END:
        if (stream->avail_in == 0)
          goto done;
        /* There may be more compressed data here. */
        if ((r = inflateEnd(stream)) != Z_OK) {
          log_warn(LD_BUG, "Error freeing gzip structures");
          goto err;
        }
        if (inflateInit2(stream,
                         method_bits(method,HIGH_COMPRESSION)) != Z_OK) {
          log_warn(LD_GENERAL, "Error from second inflateInit2: %s",
                   stream->msg?stream->msg:"<no message>");
          goto err;
        }
        break;
      case Z_OK:
        if (!complete_only && stream->avail_in == 0)
          goto done;
        /* In case zlib doesn't work as I think.... */
        if (stream->avail_out >= stream->avail_in+16)
          break;
      case Z_BUF_ERROR:
        if (stream->avail_out > 0) {
          log_fn(protocol_warn_level, LD_PROTOCOL,
                 "possible truncated or corrupt zlib data");
          goto err;
        }
        offset = stream->next_out - (unsigned char*)*out;
        old_size = out_size;
        out_size *= 2;
        if (out_size < old_size) {
          log_warn(LD_GENERAL, "Size overflow in uncompression.");
          goto err;
        }
        if (is_compression_bomb(in_len, out_size)) {
          log_warn(LD_GENERAL, "Input looks like a possible zlib bomb; "
                   "not proceeding.");
          goto err;
        }
        if (out_size >= SIZE_T_CEILING) {
          log_warn(LD_BUG, "Hit SIZE_T_CEILING limit while uncompressing.");
          goto err;
        }
        *out = spider_realloc(*out, out_size);
        stream->next_out = (unsigned char*)(*out + offset);
        if (out_size - offset > UINT_MAX) {
          log_warn(LD_BUG,  "Ran over unsigned int limit of zlib while "
                   "uncompressing.");
          goto err;
        }
        stream->avail_out = (unsigned int)(out_size - offset);
        break;
      default:
        log_warn(LD_GENERAL, "Gzip decompression returned an error: %s",
                 stream->msg ? stream->msg : "<no message>");
        goto err;
      }
  }
 done:
  *out_len = stream->next_out - (unsigned char*)*out;
  r = inflateEnd(stream);
  spider_free(stream);
  if (r != Z_OK) {
    log_warn(LD_BUG, "Error freeing gzip structures");
    goto err;
  }

  /* NUL-terminate output. */
  if (out_size == *out_len)
    *out = spider_realloc(*out, out_size + 1);
  (*out)[*out_len] = '\0';

  return 0;
 err:
  if (stream) {
    inflateEnd(stream);
    spider_free(stream);
  }
  if (*out) {
    spider_free(*out);
  }
  return -1;
}

/** Try to tell whether the <b>in_len</b>-byte string in <b>in</b> is likely
 * to be compressed or not.  If it is, return the likeliest compression method.
 * Otherwise, return UNKNOWN_METHOD.
 */
compress_method_t
detect_compression_method(const char *in, size_t in_len)
{
  if (in_len > 2 && fast_memeq(in, "\x1f\x8b", 2)) {
    return GZIP_METHOD;
  } else if (in_len > 2 && (in[0] & 0x0f) == 8 &&
             (ntohs(get_uint16(in)) % 31) == 0) {
    return ZLIB_METHOD;
  } else {
    return UNKNOWN_METHOD;
  }
}

/** Internal state for an incremental zlib compression/decompression.  The
 * body of this struct is not exposed. */
struct spider_zlib_state_t {
  struct z_stream_s stream; /**< The zlib stream */
  int compress; /**< True if we are compressing; false if we are inflating */

  /** Number of bytes read so far.  Used to detect zlib bombs. */
  size_t input_so_far;
  /** Number of bytes written so far.  Used to detect zlib bombs. */
  size_t output_so_far;

  /** Approximate number of bytes allocated for this object. */
  size_t allocation;
};

/** Construct and return a spider_zlib_state_t object using <b>method</b>.  If
 * <b>compress</b>, it's for compression; otherwise it's for
 * decompression. */
spider_zlib_state_t *
spider_zlib_new(int compress_, compress_method_t method,
             zlib_compression_level_t compression_level)
{
  spider_zlib_state_t *out;
  int bits, memlevel;

 if (! compress_) {
   /* use this setting for decompression, since we might have the
    * max number of window bits */
   compression_level = HIGH_COMPRESSION;
 }

 out = spider_malloc_zero(sizeof(spider_zlib_state_t));
 out->stream.zalloc = Z_NULL;
 out->stream.zfree = Z_NULL;
 out->stream.opaque = NULL;
 out->compress = compress_;
 bits = method_bits(method, compression_level);
 memlevel = get_memlevel(compression_level);
 if (compress_) {
   if (deflateInit2(&out->stream, Z_BEST_COMPRESSION, Z_DEFLATED,
                    bits, memlevel,
                    Z_DEFAULT_STRATEGY) != Z_OK)
     goto err; // LCOV_EXCL_LINE
 } else {
   if (inflateInit2(&out->stream, bits) != Z_OK)
     goto err; // LCOV_EXCL_LINE
 }
 out->allocation = spider_zlib_state_size_precalc(!compress_, bits, memlevel);

 total_zlib_allocation += out->allocation;

 return out;

 err:
 spider_free(out);
 return NULL;
}

/** Compress/decompress some bytes using <b>state</b>.  Read up to
 * *<b>in_len</b> bytes from *<b>in</b>, and write up to *<b>out_len</b> bytes
 * to *<b>out</b>, adjusting the values as we go.  If <b>finish</b> is true,
 * we've reached the end of the input.
 *
 * Return TOR_ZLIB_DONE if we've finished the entire compression/decompression.
 * Return TOR_ZLIB_OK if we're processed everything from the input.
 * Return TOR_ZLIB_BUF_FULL if we're out of space on <b>out</b>.
 * Return TOR_ZLIB_ERR if the stream is corrupt.
 */
spider_zlib_output_t
spider_zlib_process(spider_zlib_state_t *state,
                 char **out, size_t *out_len,
                 const char **in, size_t *in_len,
                 int finish)
{
  int err;
  spider_assert(*in_len <= UINT_MAX);
  spider_assert(*out_len <= UINT_MAX);
  state->stream.next_in = (unsigned char*) *in;
  state->stream.avail_in = (unsigned int)*in_len;
  state->stream.next_out = (unsigned char*) *out;
  state->stream.avail_out = (unsigned int)*out_len;

  if (state->compress) {
    err = deflate(&state->stream, finish ? Z_FINISH : Z_NO_FLUSH);
  } else {
    err = inflate(&state->stream, finish ? Z_FINISH : Z_SYNC_FLUSH);
  }

  state->input_so_far += state->stream.next_in - ((unsigned char*)*in);
  state->output_so_far += state->stream.next_out - ((unsigned char*)*out);

  *out = (char*) state->stream.next_out;
  *out_len = state->stream.avail_out;
  *in = (const char *) state->stream.next_in;
  *in_len = state->stream.avail_in;

  if (! state->compress &&
      is_compression_bomb(state->input_so_far, state->output_so_far)) {
    log_warn(LD_DIR, "Possible zlib bomb; abandoning stream.");
    return TOR_ZLIB_ERR;
  }

  switch (err)
    {
    case Z_STREAM_END:
      return TOR_ZLIB_DONE;
    case Z_BUF_ERROR:
      if (state->stream.avail_in == 0 && !finish)
        return TOR_ZLIB_OK;
      return TOR_ZLIB_BUF_FULL;
    case Z_OK:
      if (state->stream.avail_out == 0 || finish)
        return TOR_ZLIB_BUF_FULL;
      return TOR_ZLIB_OK;
    default:
      log_warn(LD_GENERAL, "Gzip returned an error: %s",
               state->stream.msg ? state->stream.msg : "<no message>");
      return TOR_ZLIB_ERR;
    }
}

/** Deallocate <b>state</b>. */
void
spider_zlib_free(spider_zlib_state_t *state)
{
  if (!state)
    return;

 total_zlib_allocation -= state->allocation;

  if (state->compress)
    deflateEnd(&state->stream);
  else
    inflateEnd(&state->stream);

  spider_free(state);
}

/** Return an approximate number of bytes used in RAM to hold a state with
 * window bits <b>windowBits</b> and compression level 'memlevel' */
static size_t
spider_zlib_state_size_precalc(int inflate_, int windowbits, int memlevel)
{
  windowbits &= 15;

#define A_FEW_KILOBYTES 2048

  if (inflate_) {
    /* From zconf.h:

       "The memory requirements for inflate are (in bytes) 1 << windowBits
       that is, 32K for windowBits=15 (default value) plus a few kilobytes
       for small objects."
    */
    return sizeof(spider_zlib_state_t) + sizeof(struct z_stream_s) +
      (1 << 15) + A_FEW_KILOBYTES;
  } else {
    /* Also from zconf.h:

       "The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
        ... plus a few kilobytes for small objects."
    */
    return sizeof(spider_zlib_state_t) + sizeof(struct z_stream_s) +
      (1 << (windowbits + 2)) + (1 << (memlevel + 9)) + A_FEW_KILOBYTES;
  }
#undef A_FEW_KILOBYTES
}

/** Return the approximate number of bytes allocated for <b>state</b>. */
size_t
spider_zlib_state_size(const spider_zlib_state_t *state)
{
  return state->allocation;
}

/** Return the approximate number of bytes allocated for all zlib states. */
size_t
spider_zlib_get_total_allocation(void)
{
  return total_zlib_allocation;
}

