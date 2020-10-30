#!/usr/bin/env python3

# SPDX-FileCopyrightText: © 2020 Georg Sauthoff <mail@gms.tf>
# SPDX-License-Identifier: BSL-1.0



import numpy as np
import pandas as pd

import sys



def main():
    filenames = sys.argv[1:]

    dfs = [ pd.read_csv(fn, skiprows=8) for fn in filenames[:3] ]

    df = pd.merge(pd.merge(dfs[0], dfs[1], on='name'), dfs[2], on='name')

    # cancel out outliers due to OS jitter such as timers, kernel threads, etc.
    df['cpu_time_min'] = df[['cpu_time_x', 'cpu_time_y', 'cpu_time']].min(axis=1)

    df['ns'] = np.floor(df['cpu_time_min'])

    df['name'] = df['name'].map(lambda x : x[0:x.index('/')])

    df = df[['name', 'ns']].copy()

    mm = df.groupby(['name']).agg(['min', 'max'])
    vs = df.groupby(['name']).ns.value_counts()

    pd.set_option('display.max_rows', 100)
    print(vs)
    print()
    print(mm)



if __name__ == '__main__':
    sys.exit(main())
