import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
from matplotlib import ticker
import numpy as np
import math

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, reuslt_size_l: list,
                xlim: list, ylim: list, n_bin: int,
                name_m: dict, is_test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))
    subplot_str = 111
    ax = fig.add_subplot(subplot_str)
    # counts, bins = np.histogram(total_time_l, bins=n_bin, weights=np.ones(len(total_time_l)) * weight)
    # print(counts)
    # assert 1000 - 0.1 <= np.sum(counts) <= 1000 + 0.1
    # ax.stairs(counts, bins, color='#828487', fill=True)
    # print(np.logspace(2, 5, num=100))
    ax.hist(reuslt_size_l, bins=n_bin,
            color='#828487', density=False)

    # ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel(name_m['fig_x'])
    ax.set_ylabel(name_m['fig_y'])
    if xlim:
        ax.set_xlim(xlim)
    if ylim:
        ax.set_ylim(ylim)

    if is_test:
        plt.savefig("result_size_bias.jpg", bbox_inches='tight')
    else:
        plt.savefig("result_size_bias.pdf", bbox_inches='tight')


def transform_data(topk: int):
    result_size_l = np.loadtxt(
        'data/result_size_bias/movielens-27m-top{}-simpfer_k_max_1000-n_cand.txt'.format(topk), dtype=np.int32)
    return result_size_l


if __name__ == "__main__":
    dataset_l = ['Movielens', 'Yahoomusic', 'Yelp']
    name_m = {'fig_x': 'Result Size',
              'fig_y': 'Frequency'}
    result_fname = 'ResultSizeBias'
    is_test = False
    reuslt_size_l = transform_data(200)
    print(f"min result size {np.min(reuslt_size_l)}, max result size {np.max(reuslt_size_l)}")
    plot_figure(reuslt_size_l=reuslt_size_l,
                xlim=[0, 160000], ylim=[0.9, 3000], n_bin=50,
                name_m=name_m, is_test=is_test)
