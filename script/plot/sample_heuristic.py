import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 15})


def plot_curve(*, x_l_l: list, y_l_l: list, label_l: list,
               xlabel: str, ylabel: str,
               ylim: list, log: bool,
               fname_suffix: str,
               is_test: bool):
    assert len(x_l_l) == len(y_l_l) == len(label_l)
    fig = plt.figure(figsize=(6, 4))
    subplot_str = int('111')
    ax1 = fig.add_subplot(subplot_str)
    ax1.plot(x_l_l[0], y_l_l[0] / 1000,
             color='#000000', linewidth=2.5, linestyle='-',
             label=label_l[0],
             marker=marker_l[0], fillstyle='none', markersize=markersize)
    ax1.plot(x_l_l[1], y_l_l[1] / 1000,
             color='#000000', linewidth=2.5, linestyle='-',
             label=label_l[1],
             marker=marker_l[1], fillstyle='none', markersize=markersize)

    ax1.set_xlabel(xlabel)
    ax1.set_ylabel(ylabel)
    ax1.set_ylim(ylim)
    ax1.set_xlim([0, 210])
    if log:
        ax1.set_yscale('log')
    ax1.legend(frameon=False, loc='best')
    if is_test:
        plt.savefig("sample_heuristic_{}.jpg".format(fname_suffix), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("sample_heuristic_{}.pdf".format(fname_suffix), bbox_inches='tight')


if __name__ == "__main__":
    is_test = True
    label_l = ['Kth Candidate Rank', 'Uniform Candidate Rank']

    yahoomusic_l = pd.read_csv('./data/sample_heuristic/Yahoomusic.csv')
    topk_l = yahoomusic_l['topk']
    yahoomsuic_kth_l = yahoomusic_l['QRSKthIntLR']
    yahoomusic_uniform = yahoomusic_l['QRSUniformIntLR']

    x_l_l = [topk_l, topk_l]
    y_l_l = [yahoomsuic_kth_l, yahoomusic_uniform]

    plot_curve(x_l_l=x_l_l, y_l_l=y_l_l, label_l=label_l,
               xlabel='Top-k', ylabel='Query Time (Second)',
               ylim=[0, 1.4], log=False, is_test=is_test, fname_suffix='1_yahoomusic_big')

    yelp_l = pd.read_csv('./data/sample_heuristic/Yelp.csv')
    yelp_kth_l = yelp_l['QRSKthIntLR']
    yelp_uniform = yelp_l['QRSUniformIntLR']

    x_l_l = [topk_l, topk_l]
    y_l_l = [yelp_kth_l, yelp_uniform]

    plot_curve(x_l_l=x_l_l, y_l_l=y_l_l, label_l=label_l,
               xlabel='Top-k', ylabel='Query Time (Second)',
               ylim=[0, 1.2], log=False, is_test=is_test, fname_suffix='2_yelp')
