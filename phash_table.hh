#ifndef GMS_PHASH_TABLE_HH
#define GMS_PHASH_TABLE_HH

// SPDX-FileCopyrightText: © 2020 Georg Sauthoff <mail@gms.tf>
// SPDX-License-Identifier: BSL-1.0

#include "phash_table.h"

#include <exception>

namespace gms {

    struct Phash_Table_Error : public std::exception
    {
        int code {0};

        Phash_Table_Error(int code)
            : code(code)
        {
        }
        const char* what() const noexcept override {
            return "failed building perfect hash table";
        }
    };

    using Phash_Func = Gms_Phash_Func;

    struct Phash_Table : Gms_Phash_Table {
        Phash_Table() =default;
        Phash_Table(const void *p, uint32_t n, Phash_Func hfn)
        {
            int r = gms_phash_table_build(this, p, n, hfn);
            if (r)
                throw Phash_Table_Error(r);
        }
        Phash_Table(const Phash_Table &) =delete;
        Phash_Table &operator=(const Phash_Table &) =delete;
        Phash_Table(Phash_Table &&o)
        {
            bkt_table = o.bkt_table;
            idx_table = o.idx_table;
            bkt_table_n = o.bkt_table_n;
            idx_table_n = o.idx_table_n;

            o.bkt_table = nullptr;
            o.idx_table = nullptr;
            o.bkt_table_n = 0;
            o.idx_table_n = 0;
        }
        Phash_Table &operator=(Phash_Table &&o)
        {
            bkt_table = o.bkt_table;
            idx_table = o.idx_table;
            bkt_table_n = o.bkt_table_n;
            idx_table_n = o.idx_table_n;

            o.bkt_table = nullptr;
            o.idx_table = nullptr;
            o.bkt_table_n = 0;
            o.idx_table_n = 0;
            return *this;
        }
        ~Phash_Table() {
            gms_phash_table_free(this);
        }
        inline uint32_t lookup(const void *p, Phash_Func hfn) const
        {
            return gms_phash_table_lookup(this, p, hfn);
        }
    };





    inline uint32_t hash_sdbm_32(const void *s, size_t n, uint32_t param)
    {
        return gms_hash_sdbm_32(s, n, param);
    }

}

#endif
