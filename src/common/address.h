/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file address.h
 * \brief Headers for address.h
 **/

#ifndef TOR_ADDRESS_H
#define TOR_ADDRESS_H

//#include <sys/sockio.h>
#include "orconfig.h"
#include "spiderint.h"
#include "compat.h"
#include "container.h"

#ifdef ADDRESS_PRIVATE

#if defined(HAVE_SYS_IOCTL_H)
#include <sys/ioctl.h>
#endif

#ifdef HAVE_GETIFADDRS
#define HAVE_IFADDRS_TO_SMARTLIST
#endif

#ifdef _WIN32
#define HAVE_IP_ADAPTER_TO_SMARTLIST
#endif

#if defined(SIOCGIFCONF) && defined(HAVE_IOCTL)
#define HAVE_IFCONF_TO_SMARTLIST
#endif

#if defined(HAVE_NET_IF_H)
#include <net/if.h> // for struct ifconf
#endif

#if defined(HAVE_IFADDRS_TO_SMARTLIST)
#include <ifaddrs.h>
#endif

// TODO win32 specific includes
#endif // ADDRESS_PRIVATE

/** The number of bits from an address to consider while doing a masked
 * comparison. */
typedef uint8_t maskbits_t;

struct in_addr;
/** Holds an IPv4 or IPv6 address.  (Uses less memory than struct
 * sockaddr_storage.) */
typedef struct spider_addr_t
{
  sa_family_t family;
  union {
    uint32_t dummy_; /* This field is here so we have something to initialize
                      * with a reliable cross-platform type. */
    struct in_addr in_addr;
    struct in6_addr in6_addr;
  } addr;
} spider_addr_t;

/** Holds an IP address and a TCP/UDP port.  */
typedef struct spider_addr_port_t
{
  spider_addr_t addr;
  uint16_t port;
} spider_addr_port_t;

#define TOR_ADDR_NULL {AF_UNSPEC, {0}}

static inline const struct in6_addr *spider_addr_to_in6(const spider_addr_t *a);
static inline const struct in6_addr *spider_addr_to_in6_assert(
    const spider_addr_t *a);
static inline uint32_t spider_addr_to_ipv4n(const spider_addr_t *a);
static inline uint32_t spider_addr_to_ipv4h(const spider_addr_t *a);
static inline uint32_t spider_addr_to_mapped_ipv4h(const spider_addr_t *a);
static inline sa_family_t spider_addr_family(const spider_addr_t *a);
static inline const struct in_addr *spider_addr_to_in(const spider_addr_t *a);
static inline int spider_addr_eq_ipv4h(const spider_addr_t *a, uint32_t u);

socklen_t spider_addr_to_sockaddr(const spider_addr_t *a, uint16_t port,
                               struct sockaddr *sa_out, socklen_t len);
int spider_addr_from_sockaddr(spider_addr_t *a, const struct sockaddr *sa,
                           uint16_t *port_out);
void spider_addr_make_unspec(spider_addr_t *a);
void spider_addr_make_null(spider_addr_t *a, sa_family_t family);
char *spider_sockaddr_to_str(const struct sockaddr *sa);

/** Return an in6_addr* equivalent to <b>a</b>, or NULL if <b>a</b> is not
 * an IPv6 address. */
static inline const struct in6_addr *
spider_addr_to_in6(const spider_addr_t *a)
{
  return a->family == AF_INET6 ? &a->addr.in6_addr : NULL;
}

/** As spider_addr_to_in6, but assert that the address truly is an IPv6
 * address. */
static inline const struct in6_addr *
spider_addr_to_in6_assert(const spider_addr_t *a)
{
  spider_assert(a->family == AF_INET6);
  return &a->addr.in6_addr;
}

/** Given an IPv6 address <b>x</b>, yield it as an array of uint8_t.
 *
 * Requires that <b>x</b> is actually an IPv6 address.
 */
#define spider_addr_to_in6_addr8(x) spider_addr_to_in6_assert(x)->s6_addr

/** Given an IPv6 address <b>x</b>, yield it as an array of uint16_t.
 *
 * Requires that <b>x</b> is actually an IPv6 address.
 */
#define spider_addr_to_in6_addr16(x) S6_ADDR16(*spider_addr_to_in6_assert(x))
/** Given an IPv6 address <b>x</b>, yield it as an array of uint32_t.
 *
 * Requires that <b>x</b> is actually an IPv6 address.
 */
#define spider_addr_to_in6_addr32(x) S6_ADDR32(*spider_addr_to_in6_assert(x))

/** Return an IPv4 address in network order for <b>a</b>, or 0 if
 * <b>a</b> is not an IPv4 address. */
static inline uint32_t
spider_addr_to_ipv4n(const spider_addr_t *a)
{
  return a->family == AF_INET ? a->addr.in_addr.s_addr : 0;
}
/** Return an IPv4 address in host order for <b>a</b>, or 0 if
 * <b>a</b> is not an IPv4 address. */
static inline uint32_t
spider_addr_to_ipv4h(const spider_addr_t *a)
{
  return ntohl(spider_addr_to_ipv4n(a));
}
/** Given an IPv6 address, return its mapped IPv4 address in host order, or
 * 0 if <b>a</b> is not an IPv6 address.
 *
 * (Does not check whether the address is really a mapped address */
static inline uint32_t
spider_addr_to_mapped_ipv4h(const spider_addr_t *a)
{
  if (a->family == AF_INET6) {
    uint32_t *addr32 = NULL;
    // Work around an incorrect NULL pointer dereference warning in
    // "clang --analyze" due to limited analysis depth
    addr32 = spider_addr_to_in6_addr32(a);
    // To improve performance, wrap this assertion in:
    // #if !defined(__clang_analyzer__) || PARANOIA
    spider_assert(addr32);
    return ntohl(addr32[3]);
  } else {
    return 0;
  }
}
/** Return the address family of <b>a</b>.  Possible values are:
 * AF_INET6, AF_INET, AF_UNSPEC. */
static inline sa_family_t
spider_addr_family(const spider_addr_t *a)
{
  return a->family;
}
/** Return an in_addr* equivalent to <b>a</b>, or NULL if <b>a</b> is not
 * an IPv4 address. */
static inline const struct in_addr *
spider_addr_to_in(const spider_addr_t *a)
{
  return a->family == AF_INET ? &a->addr.in_addr : NULL;
}
/** Return true iff <b>a</b> is an IPv4 address equal to the host-ordered
 * address in <b>u</b>. */
static inline int
spider_addr_eq_ipv4h(const spider_addr_t *a, uint32_t u)
{
  return a->family == AF_INET ? (spider_addr_to_ipv4h(a) == u) : 0;
}

/** Length of a buffer that you need to allocate to be sure you can encode
 * any spider_addr_t.
 *
 * This allows enough space for
 *   "[ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255]",
 * plus a terminating NUL.
 */
#define TOR_ADDR_BUF_LEN 48

MOCK_DECL(int, spider_addr_lookup,(const char *name, uint16_t family,
                                spider_addr_t *addr_out));
char *spider_addr_to_str_dup(const spider_addr_t *addr) ATTR_MALLOC;

/** Wrapper function of fmt_addr_impl(). It does not decorate IPv6
 *  addresses. */
#define fmt_addr(a) fmt_addr_impl((a), 0)
/** Wrapper function of fmt_addr_impl(). It decorates IPv6
 *  addresses. */
#define fmt_and_decorate_addr(a) fmt_addr_impl((a), 1)
const char *fmt_addr_impl(const spider_addr_t *addr, int decorate);
const char *fmt_addrport(const spider_addr_t *addr, uint16_t port);
const char * fmt_addr32(uint32_t addr);

MOCK_DECL(int,get_interface_address6,(int severity, sa_family_t family,
spider_addr_t *addr));
void free_interface_address6_list(smartlist_t * addrs);
MOCK_DECL(smartlist_t *,get_interface_address6_list,(int severity,
                                                     sa_family_t family,
                                                     int include_internal));

/** Flag to specify how to do a comparison between addresses.  In an "exact"
 * comparison, addresses are equivalent only if they are in the same family
 * with the same value.  In a "semantic" comparison, IPv4 addresses match all
 * IPv6 encodings of those addresses. */
typedef enum {
  CMP_EXACT,
  CMP_SEMANTIC,
} spider_addr_comparison_t;

int spider_addr_compare(const spider_addr_t *addr1, const spider_addr_t *addr2,
                     spider_addr_comparison_t how);
int spider_addr_compare_masked(const spider_addr_t *addr1, const spider_addr_t *addr2,
                            maskbits_t mask, spider_addr_comparison_t how);
/** Return true iff a and b are the same address.  The comparison is done
 * "exactly". */
#define spider_addr_eq(a,b) (0==spider_addr_compare((a),(b),CMP_EXACT))

uint64_t spider_addr_hash(const spider_addr_t *addr);
int spider_addr_is_v4(const spider_addr_t *addr);
int spider_addr_is_internal_(const spider_addr_t *ip, int for_listening,
                          const char *filename, int lineno);
#define spider_addr_is_internal(addr, for_listening) \
  spider_addr_is_internal_((addr), (for_listening), SHORT_FILE__, __LINE__)
int spider_addr_is_multicast(const spider_addr_t *a);

/** Longest length that can be required for a reverse lookup name. */
/* 32 nybbles, 32 dots, 8 characters of "ip6.arpa", 1 NUL: 73 characters. */
#define REVERSE_LOOKUP_NAME_BUF_LEN 73
int spider_addr_to_PTR_name(char *out, size_t outlen,
                                    const spider_addr_t *addr);
int spider_addr_parse_PTR_name(spider_addr_t *result, const char *address,
                                       int family, int accept_regular);

int spider_addr_port_lookup(const char *s, spider_addr_t *addr_out,
                        uint16_t *port_out);

/* Does the address * yield an AF_UNSPEC wildcard address (1),
 * which expands to corresponding wildcard IPv4 and IPv6 rules, and do we
 * allow *4 and *6 for IPv4 and IPv6 wildcards, respectively;
 * or does the address * yield IPv4 wildcard address (0).  */
#define TAPMP_EXTENDED_STAR 1
/* Does the address * yield an IPv4 wildcard address rule (1);
 * or does it yield wildcard IPv4 and IPv6 rules (0) */
#define TAPMP_STAR_IPV4_ONLY     (1 << 1)
/* Does the address * yield an IPv6 wildcard address rule (1);
 * or does it yield wildcard IPv4 and IPv6 rules (0) */
#define TAPMP_STAR_IPV6_ONLY     (1 << 2)
/* TAPMP_STAR_IPV4_ONLY and TAPMP_STAR_IPV6_ONLY are mutually exclusive. */
int spider_addr_parse_mask_ports(const char *s, unsigned flags,
                              spider_addr_t *addr_out, maskbits_t *mask_out,
                              uint16_t *port_min_out, uint16_t *port_max_out);
const char * spider_addr_to_str(char *dest, const spider_addr_t *addr, size_t len,
                             int decorate);
int spider_addr_parse(spider_addr_t *addr, const char *src);
void spider_addr_copy(spider_addr_t *dest, const spider_addr_t *src);
void spider_addr_copy_tight(spider_addr_t *dest, const spider_addr_t *src);
void spider_addr_from_ipv4n(spider_addr_t *dest, uint32_t v4addr);
/** Set <b>dest</b> to the IPv4 address encoded in <b>v4addr</b> in host
 * order. */
#define spider_addr_from_ipv4h(dest, v4addr)       \
  spider_addr_from_ipv4n((dest), htonl(v4addr))
void spider_addr_from_ipv6_bytes(spider_addr_t *dest, const char *bytes);
/** Set <b>dest</b> to the IPv4 address incoded in <b>in</b>. */
#define spider_addr_from_in(dest, in) \
  spider_addr_from_ipv4n((dest), (in)->s_addr);
void spider_addr_from_in6(spider_addr_t *dest, const struct in6_addr *in6);
int spider_addr_is_null(const spider_addr_t *addr);
int spider_addr_is_loopback(const spider_addr_t *addr);

int spider_addr_is_valid(const spider_addr_t *addr, int for_listening);
int spider_addr_is_valid_ipv4n(uint32_t v4n_addr, int for_listening);
#define spider_addr_is_valid_ipv4h(v4h_addr, for_listening) \
        spider_addr_is_valid_ipv4n(htonl(v4h_addr), (for_listening))
int spider_port_is_valid(uint16_t port, int for_listening);
/* Are addr and port both valid? */
#define spider_addr_port_is_valid(addr, port, for_listening) \
        (spider_addr_is_valid((addr), (for_listening)) &&    \
         spider_port_is_valid((port), (for_listening)))
/* Are ap->addr and ap->port both valid? */
#define spider_addr_port_is_valid_ap(ap, for_listening) \
        spider_addr_port_is_valid(&(ap)->addr, (ap)->port, (for_listening))
/* Are the network-order v4addr and port both valid? */
#define spider_addr_port_is_valid_ipv4n(v4n_addr, port, for_listening) \
        (spider_addr_is_valid_ipv4n((v4n_addr), (for_listening)) &&    \
         spider_port_is_valid((port), (for_listening)))
/* Are the host-order v4addr and port both valid? */
#define spider_addr_port_is_valid_ipv4h(v4h_addr, port, for_listening) \
        (spider_addr_is_valid_ipv4h((v4h_addr), (for_listening)) &&    \
         spider_port_is_valid((port), (for_listening)))

int spider_addr_port_split(int severity, const char *addrport,
                        char **address_out, uint16_t *port_out);

int spider_addr_port_parse(int severity, const char *addrport,
                        spider_addr_t *address_out, uint16_t *port_out,
                        int default_port);

int spider_addr_hostname_is_local(const char *name);

/* IPv4 helpers */
int addr_port_lookup(int severity, const char *addrport, char **address,
                    uint32_t *addr, uint16_t *port_out);
int parse_port_range(const char *port, uint16_t *port_min_out,
                     uint16_t *port_max_out);
int addr_mask_get_bits(uint32_t mask);
/** Length of a buffer to allocate to hold the results of spider_inet_ntoa.*/
#define INET_NTOA_BUF_LEN 16
int spider_inet_ntoa(const struct in_addr *in, char *buf, size_t buf_len);
char *spider_dup_ip(uint32_t addr) ATTR_MALLOC;
MOCK_DECL(int,get_interface_address,(int severity, uint32_t *addr));
/** Free a smartlist of IP addresses returned by get_interface_address_list.
 */
static inline void
free_interface_address_list(smartlist_t *addrs)
{
  free_interface_address6_list(addrs);
}
/** Return a smartlist of the IPv4 addresses of all interfaces on the server.
 * Excludes loopback and multicast addresses. Only includes internal addresses
 * if include_internal is true. (Note that a relay behind NAT may use an
 * internal address to connect to the Internet.)
 * An empty smartlist means that there are no IPv4 addresses.
 * Returns NULL on failure.
 * Use free_interface_address_list to free the returned list.
 */
static inline smartlist_t *
get_interface_address_list(int severity, int include_internal)
{
  return get_interface_address6_list(severity, AF_INET, include_internal);
}

spider_addr_port_t *spider_addr_port_new(const spider_addr_t *addr, uint16_t port);
int spider_addr_port_eq(const spider_addr_port_t *a,
                     const spider_addr_port_t *b);

#ifdef ADDRESS_PRIVATE
MOCK_DECL(smartlist_t *,get_interface_addresses_raw,(int severity,
                                                     sa_family_t family));
MOCK_DECL(int,get_interface_address6_via_udp_socket_hack,(int severity,
                                                          sa_family_t family,
                                                          spider_addr_t *addr));

#ifdef HAVE_IFADDRS_TO_SMARTLIST
STATIC smartlist_t *ifaddrs_to_smartlist(const struct ifaddrs *ifa,
                                         sa_family_t family);
STATIC smartlist_t *get_interface_addresses_ifaddrs(int severity,
                                                    sa_family_t family);
#endif

#ifdef HAVE_IP_ADAPTER_TO_SMARTLIST
STATIC smartlist_t *ip_adapter_addresses_to_smartlist(
                                        const IP_ADAPTER_ADDRESSES *addresses);
STATIC smartlist_t *get_interface_addresses_win32(int severity,
                                                  sa_family_t family);
#endif

#ifdef HAVE_IFCONF_TO_SMARTLIST
STATIC smartlist_t *ifreq_to_smartlist(char *ifr,
                                       size_t buflen);
STATIC smartlist_t *get_interface_addresses_ioctl(int severity,
                                                  sa_family_t family);
#endif

#endif // ADDRESS_PRIVATE

#endif

