#ifndef INSTRUMENT_H
#define INSTRUMENT_H

// SPDX-FileCopyrightText: © 2020 Georg Sauthoff <mail@gms.tf>
// SPDX-License-Identifier: BSL-1.0


struct Instrument {
    char isin[12 + 1];
    unsigned id;
};
typedef struct Instrument Instrument;


Instrument *get_instruments(const char *filename, size_t *k);


#endif


