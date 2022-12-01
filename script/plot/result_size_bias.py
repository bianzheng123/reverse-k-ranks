import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
from matplotlib import ticker
import numpy as np

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, topk_l: list, non0_l: list,
                name_m: dict, result_fname: str, test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))

    ax = fig.add_subplot(111)
    ax.plot(topk_l, non0_l,
            color='#000000', linewidth=2.5, linestyle='-',
            marker=marker_l[2], fillstyle='none', markersize=markersize)

    ax.set_xlabel(name_m['fig_x'])
    ax.set_ylabel(name_m['fig_y'])
    ax.set_xlim([0, 210])
    ax.set_ylim([0, 0.27])
    ax.yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1, decimals=0))
    if test:
        plt.savefig("{}.jpg".format(result_fname), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format(result_fname), bbox_inches='tight')


def transform_data():
    topk_l = [10, 20, 50, 100, 200]
    non0_l = []
    for topk in topk_l:
        result_size_l = np.loadtxt(
            'data/result_size_bias/movielens-27m-top{}-simpfer_k_max_1000-n_cand.txt'.format(topk), dtype=np.int32)
        non0 = np.sum(result_size_l != 0)
        non0_l.append(non0)
    non0_l = np.array(non0_l) / 1000
    return topk_l, non0_l


if __name__ == "__main__":
    dataset_l = ['Movielens', 'Yahoomusic', 'Yelp']
    name_m = {'fig_x': r'k',
              'fig_y': 'Percentage of \nrecommendation'}
    result_fname = 'ResultSizeBias'
    is_test = False
    topk_l, non0_l = transform_data()
    plot_figure(topk_l=topk_l, non0_l=non0_l,
                name_m=name_m, result_fname=result_fname, test=is_test)
