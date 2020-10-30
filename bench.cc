// SPDX-FileCopyrightText: © 2020 Georg Sauthoff <mail@gms.tf>
// SPDX-License-Identifier: BSL-1.0


// cf. https://github.com/google/benchmark
#include <benchmark/benchmark.h>


#include <unordered_map>
#include <string_view>

#include <stdlib.h>
#include <assert.h>


#include "phash_table.hh"


#include "instrument.c"


// XXX
// cf. https://github.com/veorq/SipHash.git
#include "SipHash/siphash.c"


#ifndef SLOTS_TO_TEST
#define SLOTS_TO_TEST 10
#endif

#define LAST_SLOT_TO_TEST (SLOTS_TO_TEST - 1)



static Instrument *single_get_instruments(size_t &n)
{
    static Instrument *xs = nullptr;
    static size_t k = 0;
    if (!xs) {
        xs = get_instruments("isin-small-sample.lst", &k);
        assert(xs);
    }
    n = k;
    return xs;
}




static uint32_t hash_instr(const void *p, uint32_t i, uint32_t param)
{
    const Instrument *x = (const Instrument *) p;
    return gms_hash_sdbm_32(x[i].isin, 12, param);

    // cf. /usr/include/c++/9/bits/hash_bytes.h
    //return std::_Hash_bytes(x[i].isin, 12, param);
}
static uint32_t hash_instr_str(const void *p, uint32_t i, uint32_t param)
{
    (void)i;
    const char *isin = (const char*) p;
    return gms_hash_sdbm_32(isin, 12, param);

    //return std::_Hash_bytes(isin, 12, param);
}
static uint32_t lookup_instr(const gms::Phash_Table &h, const Instrument *xs, const char *s)
{
    uint32_t i = h.lookup(s, hash_instr_str);
    if (memcmp(s, xs[i].isin, 12))
        return -1;
    else
        return i;
}


static uint32_t hash_instr_stl(const void *p, uint32_t i, uint32_t param)
{
    const Instrument *x = (const Instrument *) p;
    // cf. /usr/include/c++/9/bits/hash_bytes.h
    return std::_Hash_bytes(x[i].isin, 12, param);
}
static uint32_t hash_instr_str_stl(const void *p, uint32_t i, uint32_t param)
{
    (void)i;
    const char *isin = (const char*) p;
    // cf. /usr/include/c++/9/bits/hash_bytes.h
    return std::_Hash_bytes(isin, 12, param);
}
static uint32_t lookup_instr_stl(const gms::Phash_Table &h, const Instrument *xs, const char *s)
{
    uint32_t i = h.lookup(s, hash_instr_str_stl);
    if (memcmp(s, xs[i].isin, 12))
        return -1;
    else
        return i;
}


static uint32_t hash_instr_sip(const void *p, uint32_t i, uint32_t param)
{
    const Instrument *x = (const Instrument *) p;

    uint8_t k[16] = {0};
    k[0] = param;
    size_t out = 0;
    siphash((const uint8_t*)x[i].isin, 12, (const uint8_t*)&k[0], (uint8_t*)&out, sizeof out);
    return out;
}
static uint32_t hash_instr_str_sip(const void *p, uint32_t i, uint32_t param)
{
    (void)i;
    const char *isin = (const char*) p;

    uint8_t k[16] = {0};
    k[0] = param;
    size_t out = 0;
    siphash((const uint8_t*)isin, 12, (const uint8_t*)&k[0], (uint8_t*)&out, sizeof out);
    return out;
}
static uint32_t lookup_instr_sip(const gms::Phash_Table &h, const Instrument *xs, const char *s)
{
    uint32_t i = h.lookup(s, hash_instr_str_sip);
    if (memcmp(s, xs[i].isin, 12))
        return -1;
    else
        return i;
}


struct Isin_Hash {
    size_t operator()(const char *s) const
    {
        return gms::hash_sdbm_32(s, 12, 0);
    }
};
struct Isin_Hash_Stl {
    size_t operator()(const char *s) const
    {
        // return std::hash<std::string_view>()(std::string_view(s, 12));
        // since we want to use the same seed as above

        return std::_Hash_bytes(s, 12, 0);
    }
};
struct Isin_Hash_Sip {
    size_t operator()(const char *s) const
    {
        static const uint8_t k[16] = {0};
        size_t out = 0;
        siphash((const uint8_t*)s, 12, (const uint8_t*)&k[0], (uint8_t*)&out, sizeof out);
        return out;
    }
};
struct Isin_Eq {
    bool operator()(const char *s, const char *t) const
    {
        return !memcmp(s, t, 12);
    }
};


static const std::unordered_map<const char *, uint32_t, Isin_Hash, Isin_Eq> &
    single_get_umap()
{
    static size_t n = 0;
    static std::unordered_map<const char *, uint32_t, Isin_Hash, Isin_Eq> h;

    if (!n) {
        Instrument *xs = single_get_instruments(n);
        for (uint32_t i= 0; i < n; ++i) {
            h.emplace(xs[i].isin, i);
        }
    }
    return h;
}
static const std::unordered_map<const char *, uint32_t, Isin_Hash_Stl, Isin_Eq> &
    single_get_umap_stl()
{
    static size_t n = 0;
    static std::unordered_map<const char *, uint32_t, Isin_Hash_Stl, Isin_Eq> h;

    if (!n) {
        Instrument *xs = single_get_instruments(n);
        for (uint32_t i= 0; i < n; ++i) {
            h.emplace(xs[i].isin, i);
        }
    }
    return h;
}
static const std::unordered_map<const char *, uint32_t, Isin_Hash_Sip, Isin_Eq> &
    single_get_umap_sip()
{
    static size_t n = 0;
    static std::unordered_map<const char *, uint32_t, Isin_Hash_Sip, Isin_Eq> h;

    if (!n) {
        Instrument *xs = single_get_instruments(n);
        for (uint32_t i= 0; i < n; ++i) {
            h.emplace(xs[i].isin, i);
        }
    }
    return h;
}

static const gms::Phash_Table &single_get_ptable()
{
    static size_t n = 0;
    static gms::Phash_Table h;

    if (!n) {
        Instrument *xs = single_get_instruments(n);
        h = gms::Phash_Table(xs, n, hash_instr);
    }
    return h;
}
static const gms::Phash_Table &single_get_ptable_stl()
{
    static size_t n = 0;
    static gms::Phash_Table h;

    if (!n) {
        Instrument *xs = single_get_instruments(n);
        h = gms::Phash_Table(xs, n, hash_instr_stl);
    }
    return h;
}
static const gms::Phash_Table &single_get_ptable_sip()
{
    static size_t n = 0;
    static gms::Phash_Table h;

    if (!n) {
        Instrument *xs = single_get_instruments(n);
        h = gms::Phash_Table(xs, n, hash_instr_sip);
    }
    return h;
}




static void umap_sdbm(benchmark::State& state) {
    const std::unordered_map<const char *, uint32_t, Isin_Hash, Isin_Eq>
        &h = single_get_umap();
    size_t n = 0;
    const Instrument *xs = single_get_instruments(n);

    const char *q =  xs[state.range(0)].isin;

    for (auto _ : state) {
        uint32_t r = 0;

        r = h.at(q);

        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(umap_sdbm)->DenseRange(0, LAST_SLOT_TO_TEST, 1);

static void ptable_sdbm(benchmark::State& state) {
    const gms::Phash_Table &h = single_get_ptable();
    size_t n = 0;
    const Instrument *xs = single_get_instruments(n);

    const char *q =  xs[state.range(0)].isin;

    for (auto _ : state) {
        uint32_t r = 0;
        
        r = lookup_instr(h, xs, q);

        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(ptable_sdbm)->DenseRange(0, LAST_SLOT_TO_TEST, 1);


static void umap_stl(benchmark::State& state) {
    const std::unordered_map<const char *, uint32_t, Isin_Hash_Stl, Isin_Eq>
        &h = single_get_umap_stl();
    size_t n = 0;
    const Instrument *xs = single_get_instruments(n);

    const char *q =  xs[state.range(0)].isin;

    for (auto _ : state) {
        uint32_t r = 0;

        r = h.at(q);

        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(umap_stl)->DenseRange(0, LAST_SLOT_TO_TEST, 1);

static void ptable_stl(benchmark::State& state) {
    const gms::Phash_Table &h = single_get_ptable_stl();
    size_t n = 0;
    const Instrument *xs = single_get_instruments(n);

    const char *q =  xs[state.range(0)].isin;

    for (auto _ : state) {
        uint32_t r = 0;
        
        r = lookup_instr_stl(h, xs, q);

        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(ptable_stl)->DenseRange(0, LAST_SLOT_TO_TEST, 1);

static void umap_sip(benchmark::State& state) {
    const std::unordered_map<const char *, uint32_t, Isin_Hash_Sip, Isin_Eq>
        &h = single_get_umap_sip();
    size_t n = 0;
    const Instrument *xs = single_get_instruments(n);

    const char *q =  xs[state.range(0)].isin;

    for (auto _ : state) {
        uint32_t r = 0;

        r = h.at(q);

        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(umap_sip)->DenseRange(0, LAST_SLOT_TO_TEST, 1);

static void ptable_sip(benchmark::State& state) {
    const gms::Phash_Table &h = single_get_ptable_sip();
    size_t n = 0;
    const Instrument *xs = single_get_instruments(n);

    const char *q =  xs[state.range(0)].isin;

    for (auto _ : state) {
        uint32_t r = 0;
        
        r = lookup_instr_sip(h, xs, q);

        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(ptable_sip)->DenseRange(0, LAST_SLOT_TO_TEST, 1);


BENCHMARK_MAIN();

