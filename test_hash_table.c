// SPDX-FileCopyrightText: © 2020 Georg Sauthoff <mail@gms.tf>
// SPDX-License-Identifier: BSL-1.0

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "phash_table.h"

#include "instrument.h"



// NB: using gms_hash_sdbm_64() doesn't make a difference w.r.t
// hash table space usage,
// i.e. bit-distribution doesn't seem to change when truncating the
// 64 bit hash value vs. truncating during the computation of the 32 bit hash
static uint32_t hash_instrument(const void *p, uint32_t i, uint32_t param)
{
    const Instrument *x = p;
    return gms_hash_sdbm_32(x[i].isin, 12, param);
}
static uint32_t hash_ins_str(const void *p, uint32_t i, uint32_t param)
{
    (void)i;
    const char *isin = p;
    return gms_hash_sdbm_32(isin, 12, param);
}


int main(int argc, char **argv)
{
    assert(argc > 1);
    const char *filename = argv[1];

    size_t n = 0;
    Instrument *xs = get_instruments(filename, &n);
    assert(xs);

    Instrument *end = xs + n;
    (void)end;

    //for (Instrument *p = xs; p != end; ++p) {
    //    printf("%s\n", p->isin);
    //}
    printf("%zu instruments\n", n);


    Gms_Phash_Table h;
    int r =  gms_phash_table_build(&h, xs, n, hash_instrument);
    if (r) {
        fprintf(stderr, "Hash table build failed: %d\n", r);
        free(xs);
        return 1;
    }

    printf("Offset table size: %" PRIu32 " slots (%zu bytes), Index table size: %"
            PRIu32 " slots (%zu bytes), Total: %zu bytes\n",
            h.off_table_n, sizeof h.off_table[0] * h.off_table_n,
            h.idx_table_n, sizeof h.idx_table[0] * h.idx_table_n,
            sizeof h.off_table[0] * h.off_table_n + sizeof h.idx_table[0] * h.idx_table_n
            );


    for (Instrument *p = xs; p != end; ++p) {
        uint32_t i = gms_phash_table_lookup(&h, p->isin, hash_ins_str);
        if (memcmp(p->isin, xs[i].isin, 12)) {
            printf("Mismatch: expected %s vs. %s (i: %" PRIu32 ")\n",
                    p->isin, xs[i].isin, i);
        }
    }


    gms_phash_table_free(&h);


    free(xs);

    return 0;
}
