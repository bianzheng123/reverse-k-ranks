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

matplotlib.rcParams.update({'font.size': 25})


def plot_figure(*, reuslt_size_l: list,
                xlim: list, ylim: list, x_ticks: list, n_bin: int,
                fname_sufix: str,
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
            color='#828487')
    ax.ticklabel_format(style='sci', scilimits=(0, 0), axis='x')

    # ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel(name_m['fig_x'])
    ax.set_ylabel(name_m['fig_y'])
    if xlim:
        ax.set_xlim(xlim)
    if ylim:
        ax.set_ylim(ylim)
    if x_ticks:
        ax.set_xticks(x_ticks)
    ax.set_yticks([1e0, 1e1, 1e2, 1e3])

    if is_test:
        plt.savefig(f"ResultSizeBias_{fname_sufix}.jpg", bbox_inches='tight')
    else:
        plt.savefig(f"ResultSizeBias_{fname_sufix}.pdf", bbox_inches='tight')


def transform_data(ds: str, topk: int, simpfer_k_max: int):
    result_size_l = np.loadtxt(
        f'data/result_size_bias/{ds}-top{topk}-simpfer_k_max_{simpfer_k_max}-n_cand.txt', dtype=np.int32)
    return result_size_l


if __name__ == "__main__":
    dataset_l = ['Movielens', 'Yahoomusic', 'Yelp']
    origin_dataset_l = ['movielens-27m', 'yahoomusic_big', 'yelp']
    xlim_l = [[0, 160000], [0, 240000], [0, 350000]]
    # xlim_l = [[0, 500], [0, 500], [0, 500]]
    ylim_l = [[0.9, 3000], [0.9, 3000], [0.9, 3000]]
    x_ticks_l = [None, [0, 100000, 200000], None]
    # x_ticks_l = [None, None, None]
    n_bin_l = [50, 50, 40]
    # n_bin_l = [2000, 2000, 2000]
    simpfer_k_max_l = [1000, 300, 300]
    name_m = {'fig_x': 'Result Size',
              'fig_y': 'Frequency'}
    is_test = False
    for dataset_name_fig, dataset_name_file, xlim, ylim, x_ticks, n_bin, simpfer_k_max in zip(dataset_l,
                                                                                              origin_dataset_l, xlim_l,
                                                                                              ylim_l, x_ticks_l,
                                                                                              n_bin_l, simpfer_k_max_l):
        reuslt_size_l = transform_data(dataset_name_file, 200, simpfer_k_max)
        # reuslt_size_l = np.where(reuslt_size_l < 1000)
        print(
            f"min result size {np.min(reuslt_size_l)}, max result size {np.max(reuslt_size_l)}, "
            f"0 result size count {len(np.argwhere(reuslt_size_l == 0))}")
        plot_figure(reuslt_size_l=reuslt_size_l,
                    xlim=xlim, ylim=ylim, x_ticks=x_ticks, n_bin=n_bin,
                    fname_sufix=dataset_name_fig,
                    name_m=name_m, is_test=is_test)
