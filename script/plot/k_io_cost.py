import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import os

params = {
    'axes.labelsize': 8,
    'font.size': 8,
    'legend.fontsize': 10,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'text.usetex': False,
    'figure.figsize': [4.5, 4.5]
}
linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['H', 'D', 'P', '>', '*', 'X', 's', '<', '^', 'p', 'v']
markersize = 10
matplotlib.RcParams.update(params)

fig = plt.figure(figsize=(25, 4))
fig.text(0.1, 0.5, 'IO Cost', va='center', rotation='vertical')
basic_dir = os.path.join('data', 'sensitive_with_k')


def plot(i, fname, ds, is_finish):
    subplot_str = 150 + i
    ax = fig.add_subplot(subplot_str)
    df = pd.read_csv(os.path.join(basic_dir, '{}.csv'.format(fname)))
    ax.plot(df['topk'], df['QueryRankSampleIOCost'],
            color=color_l[0], linewidth=2.5, linestyle='-',
            label='Query Rank Sample',
            marker=marker_l[0], markersize=markersize)
    ax.plot(df['topk'], df['RankSampleIOCost'],
            color=color_l[1], linewidth=2.5, linestyle='-',
            label='Rank Sample',
            marker=marker_l[1], markersize=markersize)
    ax.set_xlabel(r'$k$')
    # ax.set_ylabel('Running Time (ms)')
    # ax.set_ylim(0)
    ax.set_yscale('log')
    ax.set_title(ds)
    if is_finish:
        ax.legend(frameon=False, loc='lower right')


ds_m = {
    'Movielens-27m': 'Movielens',
    'Netflix': 'Netflix',
    'Yahoomusic': 'Yahoomusic',
    'Yelp': 'Yelp',
    "Amazon": 'Amazon'
}

for i, ds_name in enumerate(ds_m.keys(), 0):
    is_finish = (i == len(ds_m.keys()) - 1)
    plot(i + 1, ds_name, ds_m[ds_name], is_finish)

plt.savefig("k_io_cost.pdf", bbox_inches='tight')
