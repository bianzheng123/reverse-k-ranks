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
fig.text(0.05, 0.5, 'Prune Ratio', va='center', rotation='vertical')
ax = fig.add_subplot(121)

topk_l = [10, 20, 30, 40, 50]

basic_dir = os.path.join('..', '..', 'result', 'plot_performance')
ml_score_sample = [0.9995, 0.9993, 0.9991, 0.9989, 0.9987]
ml_rank_sample = [0.9771, 0.9768, 0.9767, 0.9764, 0.9763]

ax.plot(topk_l, ml_score_sample,
        color=color_l[0], linewidth=2.5, linestyle='-',
        label='Score Sample',
        marker=marker_l[0], markersize=markersize)
ax.plot(topk_l, ml_rank_sample,
        color=color_l[1], linewidth=2.5, linestyle='-',
        label='Rank Sample',
        marker=marker_l[1], markersize=markersize)
ax.set_xlabel(r'$k$')
# ax.set_ylabel('Running Time (ms)')
ax.set_ylim(0.9)
ax.set_title('MovieLens')

ax2 = fig.add_subplot(122)
netflix_score_sample = [0.9989, 0.9986, 0.9983, 0.9981, 0.9979]
netflix_rank_sample = [0.9359, 0.9358, 0.9354, 0.9353, 0.9350]

ax2.plot(topk_l, netflix_score_sample,
         color=color_l[0], linewidth=2.5, linestyle='-',
         label='Score Sample',
         marker=marker_l[0], markersize=markersize)
ax2.plot(topk_l, netflix_rank_sample,
         color=color_l[1], linewidth=2.5, linestyle='-',
         label='Rank Sample',
         marker=marker_l[1], markersize=markersize)
ax2.set_xlabel(r'$k$')
# ax2.set_ylabel('Running Time (ms)')
ax2.set_ylim(0.9)
ax2.set_title('Netflix')

ax2.legend(frameon=False, loc='lower right')

plt.savefig("k_prune_ratio.pdf")
