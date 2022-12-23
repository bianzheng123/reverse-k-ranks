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
                       query_time_top50_l: np.ndarray, y1lim:list,ylim: list, fname_sufix: str, is_test: bool):
    fig = plt.figure(figsize=(4 + 2, 4))
    subplot_str = int('111')
    ax1 = fig.add_subplot(subplot_str)
    ax2 = ax1.twinx()
    ax1.bar(x=n_sample_query_l, height=build_index_l, label='Index Construction Time', width=400,
            hatch='///', color='#ffffff', edgecolor='#000000')
    ax2.plot(n_sample_query_l, query_time_top50_l / 1000,
             color='#000000', linewidth=2.5, linestyle='-',
             label='Query Time',
             marker='s', fillstyle='none', markersize=markersize)

    ax1.set_xlabel('# Training Query')
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
    yahoomusic_df = pd.read_csv('./data/n_sample_query/yahoomusic.csv')
    fname_sufix = '1_yahoomusic'
    y1lim = [0, 20]
    ylim = [0, 0.5]
    # y1lim = None
    # ylim = None

    sample_query_curve(n_sample_query_l=n_sample_query_l,
                       build_index_l=yahoomusic_df['index_build_time'],
                       query_time_top50_l=yahoomusic_df['query_time'], y1lim=y1lim, ylim=ylim,
                       fname_sufix=fname_sufix, is_test=is_test)

    # yelp
    yelp_df = pd.read_csv('./data/n_sample_query/yelp.csv')
    fname_sufix = '2_yelp'
    y1lim = [0, 25]
    ylim = [0, 2.0]
    # y1lim = None
    # ylim = None
    sample_query_curve(n_sample_query_l=n_sample_query_l,
                       build_index_l=yelp_df['index_build_time'],
                       query_time_top50_l=yelp_df['query_time'], y1lim=y1lim, ylim=ylim,
                       fname_sufix=fname_sufix, is_test=is_test)
