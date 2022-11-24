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
                       io_cost_l: list, is_test: bool):
    fig = plt.figure(figsize=(4 + 2, 4))
    subplot_str = int('111')
    ax1 = fig.add_subplot(subplot_str)
    ax2 = ax1.twinx()
    ax1.bar(x=n_sample_query_l, height=build_index_l, label='Index construction time', width=400,
            hatch='///', color='#ffffff', edgecolor='#000000')
    ax2.plot(n_sample_query_l, np.array(io_cost_l) / 1000,
             color='#000000', linewidth=2.5, linestyle='-',
             label='IO Cost',
             marker='s', fillstyle='none', markersize=markersize)

    ax1.set_xlabel('No.training query')
    ax1.set_ylabel('Running Time (Second)')
    ax1.set_yscale('log')
    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, frameon=False, loc='best')

    ax2.set_ylabel('IO Cost')
    ax2.set_yscale('log')
    ax2.set_ylim([1e2, 1e5])
    if is_test:
        plt.savefig("{}.jpg".format('n_sample_query'), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format('n_sample_query'), bbox_inches='tight')


if __name__ == "__main__":
    n_sample_query_l = [1000, 2000, 3000, 4000, 5000]
    build_index_l = [107.373, 1139.486, 3812.193, 8235.456, 15278.652]
    io_cost_l = [4474560, 1276224, 824576, 747520, 713600]
    is_test = False

    sample_query_curve(n_sample_query_l=n_sample_query_l,
                       build_index_l=build_index_l,
                       io_cost_l=io_cost_l, is_test=is_test)
