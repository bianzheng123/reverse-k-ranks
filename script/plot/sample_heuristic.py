import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 15})


def sample_heuristic_curve(*, no_heuristic_build_time_l: list, no_heuristic_retrieval_time_l: list,
                           heuristic_build_time_l: list, heuristic_retrieval_time_l: list,
                           is_test: bool):
    fig = plt.figure(figsize=(6, 4))
    subplot_str = int('111')
    ax1 = fig.add_subplot(subplot_str)
    ax1.plot(np.array(no_heuristic_retrieval_time_l) / 1000, no_heuristic_build_time_l,
             color='#000000', linewidth=2.5, linestyle='-',
             label='No heuristic',
             marker=marker_l[1], fillstyle='none', markersize=markersize)
    ax1.plot(np.array(heuristic_retrieval_time_l) / 1000, heuristic_build_time_l,
             color='#000000', linewidth=2.5, linestyle='-',
             label='Heuristic',
             marker=marker_l[2], fillstyle='none', markersize=markersize)

    ax1.set_xlabel('Query Time (Second)')
    ax1.set_ylabel('Build index time (Second)')
    ax1.set_ylim([1e1, 1e5])
    ax1.set_yscale('log')
    ax1.legend(frameon=False, loc='best')
    if is_test:
        plt.savefig("{}.jpg".format('sample_heuristic'), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format('sample_heuristic'), bbox_inches='tight')


def plot_curve(*, x_l_l: list, y_l_l: list, label_l: list,
               xlabel: str, ylabel: str,
               ylim: list, log: bool,
               fname_suffix: str,
               is_test: bool):
    assert len(x_l_l) == len(y_l_l) == len(label_l)
    fig = plt.figure(figsize=(6, 4))
    subplot_str = int('111')
    ax1 = fig.add_subplot(subplot_str)
    ax1.plot(x_l_l[0], y_l_l[0],
             color='#000000', linewidth=2.5, linestyle='-',
             label=label_l[0],
             marker=marker_l[0], fillstyle='none', markersize=markersize)
    ax1.plot(x_l_l[1], y_l_l[1],
             color='#000000', linewidth=2.5, linestyle='-',
             label=label_l[1],
             marker=marker_l[1], fillstyle='none', markersize=markersize)

    ax1.set_xlabel(xlabel)
    ax1.set_ylabel(ylabel)
    ax1.set_ylim(ylim)
    if log:
        ax1.set_yscale('log')
    ax1.legend(frameon=False, loc='best')
    if is_test:
        plt.savefig("sample_heuristic_{}.jpg".format(fname_suffix), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("sample_heuristic_{}.pdf".format(fname_suffix), bbox_inches='tight')


if __name__ == "__main__":
    is_test = False
    label_l = ['Kth Candidate Rank', 'Uniform Candidate Rank']
    n_sample_query_l = [1000, 2000, 3000, 4000, 5000]
    yahoomsuic_kth_l = [0.575093, 0.55952, 0.558958, 0.561975, 0.563848]
    yahoomusic_uniform = [5.585205, 2.794196, 1.920483, 1.471933, 1.249008]

    x_l_l = [n_sample_query_l, n_sample_query_l]
    y_l_l = [yahoomsuic_kth_l, yahoomusic_uniform]

    plot_curve(x_l_l=x_l_l, y_l_l=y_l_l, label_l=label_l,
               xlabel='# Sample Query', ylabel='Query Time (Second)',
               ylim=[0, 6], log=False, is_test=is_test, fname_suffix='1_yahoomusic_big')

    yelp_kth_l = [1.604534, 0.649852, 0.649828, 0.647989, 0.647463]
    yelp_uniform = [3.601341, 1.699676, 1.227467, 1.017598, 0.917125]

    x_l_l = [n_sample_query_l, n_sample_query_l]
    y_l_l = [yelp_kth_l, yelp_uniform]

    plot_curve(x_l_l=x_l_l, y_l_l=y_l_l, label_l=label_l,
               xlabel='# Sample Query', ylabel='Query Time (Second)',
               ylim=[0.5, 3.7], log=False, is_test=is_test, fname_suffix='2_yelp')
