import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 15})


def sample_query_curve(*, n_sample_query_l: list,
                       build_index_l: list,
                       query_time_top50_l: list, y1lim:list,ylim: list, fname_sufix: str, is_test: bool):
    fig = plt.figure(figsize=(4 + 2, 4))
    subplot_str = int('111')
    ax1 = fig.add_subplot(subplot_str)
    ax2 = ax1.twinx()
    ax1.bar(x=n_sample_query_l, height=build_index_l, label='Index construction time', width=400,
            hatch='///', color='#ffffff', edgecolor='#000000')
    ax2.plot(n_sample_query_l, query_time_top50_l,
             color='#000000', linewidth=2.5, linestyle='-',
             label='Query Time',
             marker='s', fillstyle='none', markersize=markersize)

    ax1.set_xlabel('# Training Tuery')
    ax1.set_ylabel('Build Index Time (Hour)')
    ax1.set_ylim(y1lim)
    # ax1.set_yscale('log')
    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, frameon=False, loc='best')

    ax2.set_ylabel('Query Time (Second)')
    # ax2.set_yscale('log')
    ax2.set_ylim(ylim)
    if is_test:
        plt.savefig("{}_{}.jpg".format('n_sample_query', fname_sufix), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}_{}.pdf".format('n_sample_query', fname_sufix), bbox_inches='tight')


if __name__ == "__main__":
    is_test = False
    n_sample_query_l = [1000, 2000, 3000, 4000, 5000]

    # yahoomusic
    build_index_l = [10.993915, 11.28061306, 12.02303167, 13.25171583, 15.20815917]

    # query_time_top20_l = [0.541015, 0.529482, 0.527024, 0.526398, 0.527561]
    query_time_top50_l = [0.595738, 0.563271, 0.558838, 0.556725, 0.55693]
    # query_time_top100_l = [0.67283, 0.607686, 0.598065, 0.594694, 0.593406]
    # query_time_l_l = [query_time_top100_l, query_time_top50_l, query_time_top20_l]
    fname_sufix = '1_yahoomusic'
    y1lim = [10, 17.5]
    ylim = [0.4, 0.8]

    sample_query_curve(n_sample_query_l=n_sample_query_l,
                       build_index_l=build_index_l,
                       query_time_top50_l=query_time_top50_l, y1lim=y1lim, ylim=ylim,
                       fname_sufix=fname_sufix, is_test=is_test)

    # yelp
    build_index_l = [14.7319825, 15.05507694, 15.84419111, 17.26884111, 19.50160583]
    # query_time_top20_l = [1.601517, 0.616994, 0.617298, 0.616695, 0.617479]
    query_time_top50_l = [1.638969, 0.647136, 0.645459, 0.644734, 0.644843]
    # query_time_top100_l = [1.706138, 0.706304, 0.696815, 0.692691, 0.692236]
    # query_time_l_l = [query_time_top100_l, query_time_top50_l, query_time_top20_l]
    fname_sufix = '2_yelp'
    y1lim = [14, 20]
    ylim = [0.5, 2.3]
    sample_query_curve(n_sample_query_l=n_sample_query_l,
                       build_index_l=build_index_l,
                       query_time_top50_l=query_time_top50_l, y1lim=y1lim, ylim=ylim,
                       fname_sufix=fname_sufix, is_test=is_test)
