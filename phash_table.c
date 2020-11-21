// SPDX-FileCopyrightText: © 2020 Georg Sauthoff <mail@gms.tf>
// SPDX-License-Identifier: BSL-1.0

#include "phash_table.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>


void gms_phash_table_free(Gms_Phash_Table *h)
{
    free(h->bkt_table);
    free(h->idx_table);
    *h = (const Gms_Phash_Table){0};
}


static void gms_phash_table_free_helper(uint8_t *ns, uint32_t **vs, uint32_t *ws)
{
    free(ns);
    free(vs);
    free(ws);
}


int gms_phash_table_build(Gms_Phash_Table *h, const void *p, uint32_t n,
        Gms_Phash_Func hfn)
{
    h->bkt_table_n = n/2;

    h->bkt_table = (Gms_Phash_Bucket*) calloc(h->bkt_table_n, sizeof h->bkt_table[0]);
    if (!h->bkt_table)
        return -1;
    h->idx_table = 0;

    uint8_t *ns = (uint8_t*) calloc(h->bkt_table_n, sizeof ns[0]);
    if (!ns)
        return -1;
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t x = hfn(p, i, 0);
        uint32_t k = (uint64_t)x * h->bkt_table_n >> 32;
        ++ns[k];
        if (!ns[k]) {
            gms_phash_table_free_helper(ns, 0, 0);
            gms_phash_table_free(h);
            return -2;
        }
    }

    uint32_t **vs = (uint32_t**) calloc(h->bkt_table_n, sizeof vs[0]);
    if (!vs) {
        gms_phash_table_free_helper(ns, 0, 0);
        gms_phash_table_free(h);
        return -1;
    }
    uint32_t *ws = (uint32_t*) calloc(2 * n, sizeof ws[0]);
    if (!ws) {
        gms_phash_table_free_helper(ns, vs, 0);
        gms_phash_table_free(h);
        return -1;
    }
    {
        uint32_t *t = ws;
        for (uint32_t i = 0; i < h->bkt_table_n; ++i) {
            if (ns[i]) {
                vs[i] = t;
                t += 2 * ns[i];
            }
        }
    }
    memset(ns, 0, h->bkt_table_n * sizeof ns[0]);

    for (uint32_t i = 0; i < n; ++i) {
        uint32_t x = hfn(p, i, 0);
        uint32_t k = (uint64_t)x * h->bkt_table_n >> 32;
        vs[k][ns[k] * 2] = i;
        vs[k][ns[k] * 2 + 1] = x;
        ++ns[k];
    }

    bool col[256] = {0};
    uint32_t l = 0;
    for (uint32_t i = 0; i < h->bkt_table_n; ++i) {
        if (!ns[i])
            continue;
        if (ns[i] == 1) {
            h->bkt_table[i].off     = l;
            // already initialized to 0
            // NB: n = 0 or n = 1 is both fine
            // h->bkt_table[i].n     = 0;
            // h->bkt_table[i].param = 0;
            ++l;
        } else if (ns[i] > 1) {
            // printf("Collisions: %d\n", (int)ns[i]);
            // for (uint8_t k = 0; k < ns[i]; ++k) {
            //     printf("    %u -> %u\n", vs[i][k * 2], vs[i][k * 2 + 1]);
            // }

            uint32_t j = ns[i];
            bool done = true;
            uint8_t e = 0;
            for ( ; j < 256; ++j) {
                for (e = 0; e < 24; ++e) {
                    memset(col, 0, sizeof col);
                    done = true;
                    // printf("  Trying size %d (with param %d)\n", (int)j, (int)e);
                    for (uint8_t k = 0; k < ns[i]; ++k) {
                        uint8_t x = vs[i][k * 2 + 1];
                        if (e)
                            x = hfn(p, vs[i][k * 2], e);
                        uint8_t  a = (uint16_t)x * j >> 8;
                        if (col[a]) {
                            done = false;
                            break;
                        }
                        col[a] = true;
                    }
                    if (done)
                        break;
                }
                if (done)
                    break;
            }
            if (!done) {
                gms_phash_table_free_helper(ns, vs, ws);
                gms_phash_table_free(h);
                return -3;
            }
            h->bkt_table[i].off    = l;
            h->bkt_table[i].n      = j;
            h->bkt_table[i].param  = e;
            l += j;
        }
    }
    h->idx_table = (uint32_t*) calloc(l, sizeof h->idx_table[0]);
    if (!h->idx_table) {
        gms_phash_table_free_helper(ns, vs, ws);
        gms_phash_table_free(h);
        return -1;
    }
    h->idx_table_n = l;
    for (uint32_t i = 0; i < h->bkt_table_n; ++i) {
        if (!ns[i])
            continue;
        for (uint8_t k = 0; k < ns[i]; ++k) {
            uint32_t a = vs[i][k * 2];
            uint32_t x = vs[i][k * 2 + 1];

            uint32_t j = (uint64_t)x * h->bkt_table_n >> 32;
            const Gms_Phash_Bucket *o = h->bkt_table + j;

            uint8_t y = x;
            if (o->param)
                y = hfn(p, vs[i][k * 2], o->param);
            
            uint8_t c = (uint16_t)y * o->n >> 8;
            h->idx_table[o->off + c] = a;
        }
    }
    gms_phash_table_free_helper(ns, vs, ws);
    return 0;
}

