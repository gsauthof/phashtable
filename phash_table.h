#ifndef GMS_PHASH_TABLE_H
#define GMS_PHASH_TABLE_H

// SPDX-FileCopyrightText: © 2020 Georg Sauthoff <mail@gms.tf>
// SPDX-License-Identifier: BSL-1.0

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Gms_Phash_Elem {
    uint32_t off;      // offset into Gms_Phash_Table::idx_table
    uint8_t  n;        // secondary hash table size
    uint8_t  param;    // hash function parameter/seed to resolve collisions
};
typedef struct Gms_Phash_Elem Gms_Phash_Elem;

struct Gms_Phash_Table {
    Gms_Phash_Elem  *off_table;     // offsets into idx_table
    uint32_t        *idx_table;
    uint32_t         off_table_n;
    uint32_t         idx_table_n;
};
typedef struct Gms_Phash_Table Gms_Phash_Table;

typedef uint32_t (*Gms_Phash_Func)(const void *p, uint32_t i, uint32_t param);

int gms_phash_table_build(Gms_Phash_Table *h, const void *p, uint32_t n,
        Gms_Phash_Func hfn);
void gms_phash_table_free(Gms_Phash_Table *h);



static inline uint32_t gms_phash_table_lookup(const Gms_Phash_Table *h, const void *p,
        Gms_Phash_Func hfn)
{
    uint32_t x = hfn(p, 0, 0);

    uint32_t i = (uint64_t)x * h->off_table_n >> 32;

    const Gms_Phash_Elem *o = h->off_table + i;

    // the expected case is that o->param is 0
    // if it's zero we could just assign x to y
    // thus, we could make this a conditional call
    // however, we don't do that in favor of equalizing the
    // runtime of the lookup for all elements (i.e. to minimize latency jitter)
    uint8_t y = hfn(p, 0, o->param);

    uint8_t j = (uint16_t)y * o->n >> 8;

    return h->idx_table[o->off + j];
}

// popular general hash function
// originates from the sdbm package
// also used in GNU awk
// distributes better than djb2
// distributes worse than e.g. SipHash
// but is faster and good enough when the hash table size is a prime
static inline uint64_t gms_hash_sdbm_64(const void *sP, size_t n, uint64_t param)
{
    uint64_t hash = 0;
    const unsigned char *s   = (const unsigned char*) sP;
    const unsigned char *end = s + n;

    // NB: 65599 is a prime that works well in practice
    // the parametrization isn't part of the original design,
    // i.e. this hash function wasn't designed as a parametized/seedable
    // hash function
    // however, this works good-enough to resolve collisions
    uint64_t k = 65599 + param;

    // NB: other versions of this function use some shifts
    //     however, modern optimizing compilers generate the same code,
    //     i.e. they replace the shifts with a multiplcation
    //     cf. https://godbolt.org/z/3Txh1n
    for (; s != end; ++s)
        hash = hash * k + *s;

    return hash;
}

// same as above but creates 32 bit hash values
// NB: on x86_64, saves some bytes on code size (e.g. 41 vs. 60 bytes with gcc 10.2)
//
// popular general hash function
// originates from the sdbm package
// also used in GNU awk
// distributes better than djb2
// distributes worse than e.g. SipHash
// but is faster and good enough when the hash table size is a prime
static inline uint32_t gms_hash_sdbm_32(const void *sP, size_t n, uint32_t param)
{
    uint32_t hash = 0;
    const unsigned char *s   = (const unsigned char*) sP;
    const unsigned char *end = s + n;

    // NB: 65599 is a prime that works well in practice
    // the parametrization isn't part of the original design,
    // i.e. this hash function wasn't designed as a parametized/seedable
    // hash function
    // however, this works good-enough to resolve collisions
    uint32_t k = 65599 + param;

    // NB: other versions of this function use some shifts
    //     however, modern optimizing compilers generate the same code,
    //     i.e. they replace the shifts with a multiplcation
    //     cf. https://godbolt.org/z/3Txh1n
    for (; s != end; ++s)
        hash = hash * k + *s;

    return hash;
}

#ifdef __cplusplus
}
#endif


#endif
