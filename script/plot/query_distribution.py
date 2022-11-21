import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import pandas as pd

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, title_l: list, data_l: list,
                name_m: dict, result_fname: str,
                is_test: bool, label_pos_l: list, set_xlog_l: list):
    assert len(title_l) == len(data_l)
    n_fig = len(title_l)
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(len(data_l) * 4 + 2, 4))
    fig.text(label_pos_l[0], label_pos_l[1], name_m['fig_y'], va='center', rotation='vertical')
    for fig_i in range(n_fig):
        subplot_str = int('1' + str(n_fig) + str(fig_i + 1))
        ax = fig.add_subplot(subplot_str)
        ax.hist(data_l[fig_i], bins='auto', color='#828487')

        if set_xlog_l[fig_i]:
            ax.set_xscale('log')
            ax.set_yscale('log')
        ax.set_xlabel(name_m['fig_x'])
        ax.set_title(title_l[fig_i])
    if is_test:
        plt.savefig("{}.jpg".format(result_fname), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format(result_fname), bbox_inches='tight')


if __name__ == '__main__':
    fname_l = [
        './data/single_query_performance/yahoomusic_big-RankSample-top50-n_sample_588-single-query-performance.csv',
        './data/single_query_performance/yahoomusic_big-QueryRankSampleSearchKthRank-top50-n_sample_588-single-query-performance.csv']
    title_l = ['Uniform Sample', 'Query Aware Sample']
    data_l = []
    for fname in fname_l:
        data_l.append(pd.read_csv(fname)['total_time'])

    name_m = {'csv_x': 'total_time', 'fig_x': 'Running Time (Second)',
              'fig_y': 'Frequency'}
    result_fname = 'query_response_time_distribution'
    is_test = True
    label_pos_l = [0.03, 0.5]
    set_xlog_l = [True, False]
    plot_figure(title_l=title_l, data_l=data_l,
                name_m=name_m, result_fname=result_fname,
                is_test=is_test, label_pos_l=label_pos_l, set_xlog_l=set_xlog_l)
