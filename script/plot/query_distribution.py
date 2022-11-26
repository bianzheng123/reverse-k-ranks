import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import pandas as pd

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, method_name: str, total_time_l: list,
                name_m: dict, is_test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))
    subplot_str = 111
    ax = fig.add_subplot(subplot_str)
    ax.hist(total_time_l, bins='auto', color='#828487')

    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel(name_m['fig_x'])
    ax.set_ylabel(name_m['fig_y'])
    ax.set_xlim([0.4, 1e2])
    ax.set_ylim([1, 5e2])

    if is_test:
        plt.savefig("query_distribution_response_time_{}.jpg".format(method_name), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("query_distribution_response_time_{}.pdf".format(method_name), bbox_inches='tight')


if __name__ == '__main__':
    fname_l = [
        './data/single_query_performance/yahoomusic_big-RankSample-top50-n_sample_588-single-query-performance.csv',
        './data/single_query_performance/yahoomusic_big-QueryRankSampleSearchKthRank-top50-n_sample_588-single-query-performance.csv']
    data_l = []
    for fname in fname_l:
        data_l.append(pd.read_csv(fname)['total_time'])
    method_name_l = ['1_uniform_sample', '2_query_aware_sample']

    name_m = {'csv_x': 'total_time', 'fig_x': 'Running Time (Second)',
              'fig_y': 'Frequency'}
    is_test = False
    for total_time_l, method_name in zip(data_l, method_name_l):
        plot_figure(method_name=method_name, total_time_l=total_time_l,
                    name_m=name_m, is_test=is_test)
