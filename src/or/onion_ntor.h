/* Copyright (c) 2012-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_ONION_NTOR_H
#define TOR_ONION_NTOR_H

#include "spiderint.h"
#include "crypto_curve25519.h"
#include "di_ops.h"

/** State to be maintained by a client between sending an nspider onionskin
 * and receiving a reply. */
typedef struct nspider_handshake_state_t nspider_handshake_state_t;

/** Length of an nspider onionskin, as sent from the client to server. */
#define NTOR_ONIONSKIN_LEN 84
/** Length of an nspider reply, as sent from server to client. */
#define NTOR_REPLY_LEN 64

void nspider_handshake_state_free(nspider_handshake_state_t *state);

int onion_skin_nspider_create(const uint8_t *router_id,
                           const curve25519_public_key_t *router_key,
                           nspider_handshake_state_t **handshake_state_out,
                           uint8_t *onion_skin_out);

int onion_skin_nspider_server_handshake(const uint8_t *onion_skin,
                                 const di_digest256_map_t *private_keys,
                                 const curve25519_keypair_t *junk_keypair,
                                 const uint8_t *my_node_id,
                                 uint8_t *handshake_reply_out,
                                 uint8_t *key_out,
                                 size_t key_out_len);

int onion_skin_nspider_client_handshake(
                             const nspider_handshake_state_t *handshake_state,
                             const uint8_t *handshake_reply,
                             uint8_t *key_out,
                             size_t key_out_len,
                             const char **msg_out);

#ifdef ONION_NTOR_PRIVATE

/** Sspiderage held by a client while waiting for an nspider reply from a server. */
struct nspider_handshake_state_t {
  /** Identity digest of the router we're talking to. */
  uint8_t router_id[DIGEST_LEN];
  /** Onion key of the router we're talking to. */
  curve25519_public_key_t pubkey_B;

  /**
   * Short-lived keypair for use with this handshake.
   * @{ */
  curve25519_secret_key_t seckey_x;
  curve25519_public_key_t pubkey_X;
  /** @} */
};
#endif

#endif

