import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 15})


def sample_topk_curve(*, sample_topk_l: list,
                      running_time_l: list, is_test: bool):
    fig = plt.figure(figsize=(6, 4))
    subplot_str = int('111')
    ax1 = fig.add_subplot(subplot_str)
    ax1.plot(sample_topk_l, np.array(running_time_l) / 1000,
             color='#000000', linewidth=2.5, linestyle='-',
             marker='s', fillstyle='none', markersize=markersize)

    ax1.set_xlabel('Train top-k')
    ax1.set_ylabel('Running Time (Second)')
    ax1.set_ylim([0.54, 0.6])
    if is_test:
        plt.savefig("{}.jpg".format('sample_topk'), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format('sample_topk'), bbox_inches='tight')


if __name__ == "__main__":
    sample_topk_l = [50, 100, 150, 200, 250]
    running_time_l = [562.158, 576.192, 576.089, 579.033, 579.164]
    is_test = False
    sample_topk_curve(sample_topk_l=sample_topk_l, running_time_l=running_time_l, is_test=is_test)
