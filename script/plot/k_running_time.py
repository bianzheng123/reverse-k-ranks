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

fig = plt.figure(figsize=(10, 4))
fig.text(0.05, 0.5, 'Running Time (ms)', va='center', rotation='vertical')
ax = fig.add_subplot(121)

basic_dir = os.path.join('..', '..', 'result', 'plot_performance')
ml_score_sample = pd.read_csv(os.path.join(basic_dir, 'movielens-27m-IntervalBound-n_sample_105-config.csv'))
ml_rank_sample = pd.read_csv(os.path.join(basic_dir, 'movielens-27m-RankBound-cache_bound_every_512-config.csv'))

ax.plot(ml_score_sample['topk'], ml_score_sample['retrieval_time'],
        color=color_l[0], linewidth=2.5, linestyle='-',
        label='Score Sample',
        marker=marker_l[0], markersize=markersize)
ax.plot(ml_rank_sample['topk'], ml_rank_sample['retrieval_time'],
        color=color_l[1], linewidth=2.5, linestyle='-',
        label='Rank Sample',
        marker=marker_l[1], markersize=markersize)
ax.set_xlabel(r'$k$')
# ax.set_ylabel('Running Time (ms)')
ax.set_ylim(0)
ax.set_title('MovieLens')

ax2 = fig.add_subplot(122)
netflix_score_sample = pd.read_csv(os.path.join(basic_dir, 'netflix-IntervalBound-n_sample_34-config.csv'))
netflix_rank_sample = pd.read_csv(os.path.join(basic_dir, 'netflix-RankBound-cache_bound_every_512-config.csv'))

ax2.plot(netflix_score_sample['topk'], netflix_score_sample['retrieval_time'],
         color=color_l[0], linewidth=2.5, linestyle='-',
         label='Score Sample',
         marker=marker_l[0], markersize=markersize)
ax2.plot(netflix_rank_sample['topk'], netflix_rank_sample['retrieval_time'],
         color=color_l[1], linewidth=2.5, linestyle='-',
         label='Rank Sample',
         marker=marker_l[1], markersize=markersize)
ax2.set_xlabel(r'$k$')
# ax2.set_ylabel('Running Time (ms)')
ax2.set_ylim(0)
ax2.set_title('Netflix')

ax2.legend(frameon=False, loc='lower right')

plt.savefig("k_running_time.pdf")
