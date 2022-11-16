import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, fname_l: list, dataset_l: list,
                name_m: dict, method_m: dict, result_fname: str, test: bool, set_ylog: bool):
    assert len(fname_l) == len(dataset_l)
    n_fig = len(fname_l)
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(len(dataset_l) * 4 + 2, 4))
    fig.text(0.07, 0.5, name_m['fig_y'], va='center', rotation='vertical')
    for fig_i in range(n_fig):
        subplot_str = int('1' + str(n_fig) + str(fig_i + 1))
        ax = fig.add_subplot(subplot_str)
        df = pd.read_csv(fname_l[fig_i])
        for method_i, key in enumerate(method_m.keys()):
            x_name = name_m['csv_x']
            y_name = key + name_m['csv_y']
            ax.plot(df[x_name], df[y_name],
                    color='#000000', linewidth=2.5, linestyle='-',
                    label=method_m[key],
                    marker=marker_l[method_i], fillstyle='none', markersize=markersize)

        ax.set_xlabel(name_m['fig_x'])
        if set_ylog:
            ax.set_yscale('log')
        ax.set_title(dataset_l[fig_i])
        if fig_i == n_fig - 1:
            ax.legend(frameon=True, loc='center right')
        pass
    pass
    if test:
        plt.savefig("{}.jpg".format(result_fname), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format(result_fname), bbox_inches='tight')


if __name__ == "__main__":
    fname_l = ['./data/k_curve/Movielens-27m.csv',
               './data/k_curve/Yahoomusic.csv',
               './data/k_curve/Yelp.csv']
    dataset_l = ['Movielens', 'Yahoomusic', 'Yelp']
    name_m = {'csv_x': 'topk', 'fig_x': r'k',
              'csv_y': 'RunningTime', 'fig_y': 'Running Time (Seconds)'}
    method_m = {'RankSample': 'US', 'QueryRankSample': 'AS'}
    result_fname = 'k_io'
    is_test = True
    plot_figure(fname_l=fname_l, dataset_l=dataset_l,
                name_m=name_m, method_m=method_m,
                result_fname='curve_plot', test=True, set_ylog=True)
