import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import os

params = {
    'axes.labelsize': 8,
    'font.size': 8,
    'legend.fontsize': 10,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'text.usetex': False,
    'figure.figsize': [4.5, 4.5]
}
linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['H', 'D', 'P', '>', '*', 'X', 's', '<', '^', 'p', 'v']
markersize = 10
matplotlib.RcParams.update(params)


def plot_figure(*, fname_l: list, dataset_l: list,
                name_m: dict, method_m: dict, result_fname: str):
    assert len(fname_l) == len(dataset_l)
    n_fig = len(fname_l)
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure()
    fig.text(0.1, 0.5, name_m['fig_y'], va='center', rotation='vertical')
    for fig_i in range(n_fig):
        subplot_str = int('1' + str(n_fig) + str(fig_i + 1))
        ax = fig.add_subplot(subplot_str)
        df = pd.read_csv(fname_l[fig_i])
        for method_i, key in enumerate(method_m.keys()):
            x_name = name_m['csv_x']
            y_name = key + name_m['csv_y']
            ax.plot(df[x_name], df[y_name],
                    color=color_l[method_i], linewidth=2.5, linestyle='-',
                    label=method_m[key],
                    marker=marker_l[method_i], markersize=markersize)

        ax.set_xlabel(name_m['fig_x'])
        # ax.set_yscale('log')
        ax.set_title(dataset_l[fig_i])
        if fig_i == n_fig - 1:
            ax.legend(frameon=False, loc='lower right')
        pass
    pass
    plt.savefig("curve_plot.jpg", bbox_inches='tight', dpi=600)


fname_l = ['./data/sensitive_with_k/Movielens-27m.csv',
           './data/sensitive_with_k/Yahoomusic.csv',
           './data/sensitive_with_k/Yelp.csv']
dataset_l = ['Movielens', 'Yahoomusic', 'Yelp']
name_m = {'csv_x': 'topk', 'fig_x': r'k',
          'csv_y': 'RunningTime', 'fig_y': 'RunningTime'}
method_m = {'RankSample': 'US', 'QueryRankSample': 'AS'}
result_fname = 'k_io'
is_test = True
plot_figure(fname_l=fname_l, dataset_l=dataset_l,
            name_m=name_m, method_m=method_m,
            result_fname=result_fname)
