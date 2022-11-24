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

    ax1.set_xlabel('Query response Time (Second)')
    ax1.set_ylabel('Build index time (Second)')
    ax1.set_ylim([1e1, 1e5])
    ax1.set_yscale('log')
    ax1.legend(frameon=False, loc='best')
    if is_test:
        plt.savefig("{}.jpg".format('sample_heuristic'), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format('sample_heuristic'), bbox_inches='tight')


if __name__ == "__main__":
    no_heuristic_build_time_l = [12565.36, 15023.8, 17482.24, 20213.84, 22672.28]
    no_heuristic_retrieval_time_l = [2140.232, 2109.198, 2003.062, 1029.629, 921.836]

    heuristic_build_time_l = [12.94, 15.47173913, 18.00347826, 20.81652174, 23.34826087]
    heuristic_retrieval_time_l = [2140.564, 2109.409, 1988.466, 1027.141, 919.078]

    is_test = False
    sample_heuristic_curve(no_heuristic_build_time_l=no_heuristic_build_time_l,
                           no_heuristic_retrieval_time_l=no_heuristic_retrieval_time_l,
                           heuristic_build_time_l=heuristic_build_time_l,
                           heuristic_retrieval_time_l=heuristic_retrieval_time_l,
                           is_test=is_test)
