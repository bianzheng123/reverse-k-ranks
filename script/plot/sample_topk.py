import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 15})


def plot_figure(*, fname: str, dataset: str, set_log: bool, ylim: list, legend_loc: tuple, labelpad: int,
                name_m: dict, method_m: dict, result_fname_prefix: str, test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))
    # fig.text(label_pos_l[0], label_pos_l[1], name_m['fig_y'], va='center', rotation='vertical')
    subplot_str = 111
    ax = fig.add_subplot(subplot_str)
    df = pd.read_csv(fname)
    for method_i, key in enumerate(method_m.keys()):
        x_name = name_m['csv_x']
        y_name = key + name_m['csv_y']
        ax.plot(df[x_name], df[y_name] / 1000,
                color='#000000', linewidth=2.5, linestyle='-',
                label=method_m[key],
                marker=marker_l[method_i], fillstyle='none', markersize=markersize)

    ax.set_xlabel(name_m['fig_x'])
    ax.set_xlim([0, 210])
    ax.set_ylabel(name_m['fig_y'], labelpad=labelpad)
    if ylim:
        ax.set_ylim(ylim)
    if set_log:
        ax.set_yscale('log')
    ax.legend(frameon=False, loc=legend_loc[0], bbox_to_anchor=legend_loc[1])
    if test:
        plt.savefig("{}_{}.jpg".format(result_fname_prefix, dataset), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}_{}.pdf".format(result_fname_prefix, dataset), bbox_inches='tight')


if __name__ == "__main__":
    is_test = False
    fname_l = ['./data/sample_topk/Yahoomusic.csv',
               './data/sample_topk/Yelp.csv']
    dataset_l = ['1_Yahoomusic', '2_Yelp']
    set_log_l = [False, True]
    # ylim_l = [[0, 0.64], [0, 0.75]]
    # ylim_l = [[0, 0.7], [0, 0.8]]
    ylim_l = [[0, 1.6], [0.1, 1400]]
    legend_loc_l = [('upper right', None), ('upper right', (1.01, 1.02))]
    labelpad_l = [0, 0]

    name_m = {'csv_x': 'train_topk', 'fig_x': r'Train Top-k',
              'csv_y': 'RunningTime', 'fig_y': 'Query Time (Second)'}
    method_m = {'200': 'Query Top-k=200', '100': 'Query Top-k=100', '10': 'Query Top-k=10'}
    result_fname_prefix = 'sample_topk'
    for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
                                                                   set_log_l, ylim_l, legend_loc_l,
                                                                   labelpad_l):
        plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, legend_loc=legend_loc, labelpad=labelpad,
                    name_m=name_m, method_m=method_m,
                    result_fname_prefix=result_fname_prefix, test=is_test)
