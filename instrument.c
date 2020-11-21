// SPDX-FileCopyrightText: © 2020 Georg Sauthoff <mail@gms.tf>
// SPDX-License-Identifier: BSL-1.0


#include "instrument.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Instrument *get_instruments(const char *filename, size_t *k)
{
    int r = 0;
    char *line = 0;
    size_t line_n = 0;

    FILE * f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return 0;
    }


    size_t n = 0;
    for (;;) {
        errno = 0;
        ssize_t l = getline(&line, &line_n, f);
        if (l == -1) {
            if (errno) {
                perror("getline");
                return 0;
            }
            break;
        }
        ++n;
    }
    r = fseek(f, 0, SEEK_SET);
    if (r == -1) {
        perror("fseek");
        return 0;
    }
    Instrument *xs = (Instrument*) calloc(n, sizeof *xs);
    Instrument *p = xs;
    if (!xs)
        return 0;
    for (;;) {
        errno = 0;
        ssize_t l = getline(&line, &line_n, f);
        if (l == -1) {
            if (errno) {
                perror("getline");
                return 0;
            }
            break;
        }
        if (l)
            line[l-1] = 0;
        strcpy(p->isin, line);
        ++p;
    }

    r = fclose(f);
    if (r == -1) {
        free(xs);
        perror("fclose");
        return 0;
    }
    *k = n;
    return xs;
}
