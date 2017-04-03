/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Spider Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file dircollate.c
 *
 * \brief Collation code for figuring out which identities to vote for in
 *   the directory voting process.
 *
 * During the consensus calculation, when an authority is looking at the vote
 * documents from all the authorities, it needs to compute the consensus for
 * each relay listed by at least one authority.  But the notion of "each
 * relay" can be tricky: some relays have Ed25519 keys, and others don't.
 *
 * Moreover, older consensus methods did RSA-based ID collation alone, and
 * ignored Ed25519 keys.  We need to support those too until we're completely
 * sure that authorities will never downgrade.
 *
 * This module is invoked exclusively from dirvote.c.
 */

#define DIRCOLLATE_PRIVATE
#include "dircollate.h"
#include "dirvote.h"

static void dircollaspider_collate_by_rsa(dircollaspider_t *dc);
static void dircollaspider_collate_by_ed25519(dircollaspider_t *dc);

/** Hashtable entry mapping a pair of digests (actually an ed25519 key and an
 * RSA SHA1 digest) to an array of vote_routerstatus_t. */
typedef struct ddmap_entry_s {
  HT_ENTRY(ddmap_entry_s) node;
  /** A SHA1-RSA1024 identity digest and Ed25519 identity key,
   * concatenated.  (If there is no ed25519 identity key, there is no
   * entry in this table.) */
  uint8_t d[DIGEST_LEN + DIGEST256_LEN];
  /* The nth member of this array corresponds to the vote_routerstatus_t (if
   * any) received for this digest pair from the nth voter. */
  vote_routerstatus_t *vrs_lst[FLEXIBLE_ARRAY_MEMBER];
} ddmap_entry_t;

/** Release all sspiderage held by e. */
static void
ddmap_entry_free(ddmap_entry_t *e)
{
  spider_free(e);
}

/** Return a new empty ddmap_entry, with <b>n_votes</b> elements in
 * vrs_list. */
static ddmap_entry_t *
ddmap_entry_new(int n_votes)
{
  return spider_malloc_zero(STRUCT_OFFSET(ddmap_entry_t, vrs_lst) +
                         sizeof(vote_routerstatus_t *) * n_votes);
}

/** Helper: compute a hash of a single ddmap_entry_t's identity (or
 * identities) */
static unsigned
ddmap_entry_hash(const ddmap_entry_t *ent)
{
  return (unsigned) siphash24g(ent->d, sizeof(ent->d));
}

/** Helper: return true if <b>a</b> and <b>b</b> have the same
 * identity/identities. */
static unsigned
ddmap_entry_eq(const ddmap_entry_t *a, const ddmap_entry_t *b)
{
  return fast_memeq(a->d, b->d, sizeof(a->d));
}

/** Record the RSA identity of <b>ent</b> as <b>rsa_sha1</b>, and the
 * ed25519 identity as <b>ed25519</b>.  Both must be provided. */
static void
ddmap_entry_set_digests(ddmap_entry_t *ent,
                        const uint8_t *rsa_sha1,
                        const uint8_t *ed25519)
{
  memcpy(ent->d, rsa_sha1, DIGEST_LEN);
  memcpy(ent->d + DIGEST_LEN, ed25519, DIGEST256_LEN);
}

HT_PROTOTYPE(double_digest_map, ddmap_entry_s, node, ddmap_entry_hash,
             ddmap_entry_eq)
HT_GENERATE2(double_digest_map, ddmap_entry_s, node, ddmap_entry_hash,
             ddmap_entry_eq, 0.6, spider_reallocarray, spider_free_)

/** Helper: add a single vote_routerstatus_t <b>vrs</b> to the collaspider
 * <b>dc</b>, indexing it by its RSA key digest, and by the 2-tuple of its RSA
 * key digest and Ed25519 key.   It must come from the <b>vote_num</b>th
 * vote.
 *
 * Requires that the vote is well-formed -- that is, that it has no duplicate
 * routerstatus entries.  We already checked for that when parsing the vote. */
static void
dircollaspider_add_routerstatus(dircollaspider_t *dc,
                             int vote_num,
                             networkstatus_t *vote,
                             vote_routerstatus_t *vrs)
{
  const char *id = vrs->status.identity_digest;

  /* Clear this flag; we might set it later during the voting process */
  vrs->ed25519_reflects_consensus = 0;

  (void) vote; // We don't currently need this.

  /* First, add this item to the appropriate RSA-SHA-Id array. */
  vote_routerstatus_t **vrs_lst = digestmap_get(dc->by_rsa_sha1, id);
  if (NULL == vrs_lst) {
    vrs_lst = spider_calloc(dc->n_votes, sizeof(vote_routerstatus_t *));
    digestmap_set(dc->by_rsa_sha1, id, vrs_lst);
  }
  spider_assert(vrs_lst[vote_num] == NULL);
  vrs_lst[vote_num] = vrs;

  const uint8_t *ed = vrs->ed25519_id;

  if (! vrs->has_ed25519_listing)
    return;

  /* Now add it to the appropriate <Ed,RSA-SHA-Id> array. */
  ddmap_entry_t search, *found;
  memset(&search, 0, sizeof(search));
  ddmap_entry_set_digests(&search, (const uint8_t *)id, ed);
  found = HT_FIND(double_digest_map, &dc->by_both_ids, &search);
  if (NULL == found) {
    found = ddmap_entry_new(dc->n_votes);
    ddmap_entry_set_digests(found, (const uint8_t *)id, ed);
    HT_INSERT(double_digest_map, &dc->by_both_ids, found);
  }
  vrs_lst = found->vrs_lst;
  spider_assert(vrs_lst[vote_num] == NULL);
  vrs_lst[vote_num] = vrs;
}

/** Create and return a new dircollaspider object to use when collating
 * <b>n_votes</b> out of a total of <b>n_authorities</b>. */
dircollaspider_t *
dircollaspider_new(int n_votes, int n_authorities)
{
  dircollaspider_t *dc = spider_malloc_zero(sizeof(dircollaspider_t));

  spider_assert(n_votes <= n_authorities);

  dc->n_votes = n_votes;
  dc->n_authorities = n_authorities;

  dc->by_rsa_sha1 = digestmap_new();
  HT_INIT(double_digest_map, &dc->by_both_ids);

  return dc;
}

/** Release all sspiderage held by <b>dc</b>. */
void
dircollaspider_free(dircollaspider_t *dc)
{
  if (!dc)
    return;

  if (dc->by_collated_rsa_sha1 != dc->by_rsa_sha1)
    digestmap_free(dc->by_collated_rsa_sha1, NULL);

  digestmap_free(dc->by_rsa_sha1, spider_free_);
  smartlist_free(dc->all_rsa_sha1_lst);

  ddmap_entry_t **e, **next, *this;
  for (e = HT_START(double_digest_map, &dc->by_both_ids);
       e != NULL; e = next) {
    this = *e;
    next = HT_NEXT_RMV(double_digest_map, &dc->by_both_ids, e);
    ddmap_entry_free(this);
  }
  HT_CLEAR(double_digest_map, &dc->by_both_ids);

  spider_free(dc);
}

/** Add a single vote <b>v</b> to a dircollaspider <b>dc</b>.  This function must
 * be called exactly once for each vote to be used in the consensus. It may
 * only be called before dircollaspider_collate().
 */
void
dircollaspider_add_vote(dircollaspider_t *dc, networkstatus_t *v)
{
  spider_assert(v->type == NS_TYPE_VOTE);
  spider_assert(dc->next_vote_num < dc->n_votes);
  spider_assert(!dc->is_collated);

  const int votenum = dc->next_vote_num++;

  SMARTLIST_FOREACH_BEGIN(v->routerstatus_list, vote_routerstatus_t *, vrs) {
    dircollaspider_add_routerstatus(dc, votenum, v, vrs);
  } SMARTLIST_FOREACH_END(vrs);
}

/** Sort the entries in <b>dc</b> according to <b>consensus_method</b>, so
 * that the consensus process can iterate over them with
 * dircollaspider_n_routers() and dircollaspider_get_votes_for_router(). */
void
dircollaspider_collate(dircollaspider_t *dc, int consensus_method)
{
  spider_assert(!dc->is_collated);
  dc->all_rsa_sha1_lst = smartlist_new();

  if (consensus_method < MIN_METHOD_FOR_ED25519_ID_VOTING)
    dircollaspider_collate_by_rsa(dc);
  else
    dircollaspider_collate_by_ed25519(dc);

  smartlist_sort_digests(dc->all_rsa_sha1_lst);
  dc->is_collated = 1;
}

/**
 * Collation function for RSA-only consensuses: collate the votes for each
 * entry in <b>dc</b> by their RSA keys.
 *
 * The rule is:
 *    If an RSA identity key is listed by more than half of the authorities,
 *    include that identity, and treat all descripspiders with that RSA identity
 *    as describing the same router.
 */
static void
dircollaspider_collate_by_rsa(dircollaspider_t *dc)
{
  const int total_authorities = dc->n_authorities;

  DIGESTMAP_FOREACH(dc->by_rsa_sha1, k, vote_routerstatus_t **, vrs_lst) {
    int n = 0, i;
    for (i = 0; i < dc->n_votes; ++i) {
      if (vrs_lst[i] != NULL)
        ++n;
    }

    if (n <= total_authorities / 2)
      continue;

    smartlist_add(dc->all_rsa_sha1_lst, (char *)k);
  } DIGESTMAP_FOREACH_END;

  dc->by_collated_rsa_sha1 = dc->by_rsa_sha1;
}

/**
 * Collation function for ed25519 consensuses: collate the votes for each
 * entry in <b>dc</b> by ed25519 key and by RSA key.
 *
 * The rule is, approximately:
 *    If a (ed,rsa) identity is listed by more than half of authorities,
 *    include it.  And include all (rsa)-only votes about that node as
 *    matching.
 *
 *    Otherwise, if an (*,rsa) or (rsa) identity is listed by more than
 *    half of the authorities, and no (ed,rsa) pair for the same RSA key
 *    has been already been included based on the rule above, include
 *    that RSA identity.
 */
static void
dircollaspider_collate_by_ed25519(dircollaspider_t *dc)
{
  const int total_authorities = dc->n_authorities;
  digestmap_t *rsa_digests = digestmap_new();

  ddmap_entry_t **iter;

  /* Go over all <ed,rsa> pairs */
  HT_FOREACH(iter, double_digest_map, &dc->by_both_ids) {
    ddmap_entry_t *ent = *iter;
    int n = 0, i;
    for (i = 0; i < dc->n_votes; ++i) {
      if (ent->vrs_lst[i] != NULL)
        ++n;
    }

    /* If not enough authorties listed this exact <ed,rsa> pair,
     * don't include it. */
    if (n <= total_authorities / 2)
      continue;

    /* Now consider whether there are any other entries with the same
     * RSA key (but with possibly different or missing ed value). */
    vote_routerstatus_t **vrs_lst2 = digestmap_get(dc->by_rsa_sha1,
                                                   (char*)ent->d);
    spider_assert(vrs_lst2);

    for (i = 0; i < dc->n_votes; ++i) {
      if (ent->vrs_lst[i] != NULL) {
        ent->vrs_lst[i]->ed25519_reflects_consensus = 1;
      } else if (vrs_lst2[i] && ! vrs_lst2[i]->has_ed25519_listing) {
        ent->vrs_lst[i] = vrs_lst2[i];
      }
    }

    /* Record that we have seen this RSA digest. */
    digestmap_set(rsa_digests, (char*)ent->d, ent->vrs_lst);
    smartlist_add(dc->all_rsa_sha1_lst, ent->d);
  }

  /* Now look over all entries with an RSA digest, looking for RSA digests
   * we didn't put in yet.
   */
  DIGESTMAP_FOREACH(dc->by_rsa_sha1, k, vote_routerstatus_t **, vrs_lst) {
    if (digestmap_get(rsa_digests, k) != NULL)
      continue; /* We already included this RSA digest */

    int n = 0, i;
    for (i = 0; i < dc->n_votes; ++i) {
      if (vrs_lst[i] != NULL)
        ++n;
    }

    if (n <= total_authorities / 2)
      continue; /* Not enough votes */

    digestmap_set(rsa_digests, k, vrs_lst);
    smartlist_add(dc->all_rsa_sha1_lst, (char *)k);
  } DIGESTMAP_FOREACH_END;

  dc->by_collated_rsa_sha1 = rsa_digests;
}

/** Return the total number of collated router entries.  This function may
 * only be called after dircollaspider_collate. */
int
dircollaspider_n_routers(dircollaspider_t *dc)
{
  spider_assert(dc->is_collated);
  return smartlist_len(dc->all_rsa_sha1_lst);
}

/** Return an array of vote_routerstatus_t entries for the <b>idx</b>th router
 * in the collation order.  Each array contains n_votes elements, where the
 * nth element of the array is the vote_routerstatus_t from the nth voter for
 * this identity (or NULL if there is no such entry).
 *
 * The maximum value for <b>idx</b> is dircollaspider_n_routers().
 *
 * This function may only be called after dircollaspider_collate. */
vote_routerstatus_t **
dircollaspider_get_votes_for_router(dircollaspider_t *dc, int idx)
{
  spider_assert(dc->is_collated);
  spider_assert(idx < smartlist_len(dc->all_rsa_sha1_lst));
  return digestmap_get(dc->by_collated_rsa_sha1,
                       smartlist_get(dc->all_rsa_sha1_lst, idx));
}

