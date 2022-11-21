import json
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['D', "v", "o", "x", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, fname_l: list, dataset_l: list,
                name_m: dict, method_m: dict, result_fname: str, test: bool):
    assert len(fname_l) == len(dataset_l)
    n_fig = len(fname_l)
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(len(dataset_l) * 4 + 1, 4))
    fig.text(0.03, 0.5, name_m['fig_y'], va='center', rotation='vertical')
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
        if fig_i == 0:
            ax.set_ylim([0.2, 0.6])

        ax.set_xlabel(name_m['fig_x'])
        ax.set_title(dataset_l[fig_i])
        if fig_i == n_fig - 1:
            ax.legend(frameon=False, loc='best', borderaxespad=-0)
        pass
    pass
    if test:
        plt.savefig("{}.jpg".format(result_fname), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format(result_fname), bbox_inches='tight')


def transform_data(*, dataset_l: list, dim_l: list):
    for dataset in dataset_l:
        hit_50_l = []
        hit_100_l = []
        hit_200_l = []
        for dim in dim_l:
            with open('data/dimension_selection/raw_data/hitting_rate-{}-{}-new.json'.format(dataset, dim), 'r') as f:
                json_ins = json.load(f)
            hit_50_l.append(json_ins['test_result']['hit@50'])
            hit_100_l.append(json_ins['test_result']['hit@100'])
            hit_200_l.append(json_ins['test_result']['hit@200'])
        df = pd.DataFrame({'dimension': dim_l, 'HR@50': hit_50_l, 'HR@100': hit_100_l, 'HR@200': hit_200_l})
        df.to_csv("data/dimension_selection/{}.csv".format(dataset), index=False)


def plot_data():
    fname_l = ['./data/dimension_selection/lastfm.csv',
               './data/dimension_selection/ml-1m.csv']
    dataset_l = ['Last.fm', 'Movielens-1m']
    name_m = {'csv_x': 'dimension', 'fig_x': 'Dimension',
              'csv_y': '', 'fig_y': 'Hitting ratio'}
    method_m = {'HR@50': 'HR@50', 'HR@100': 'HR@100', 'HR@200': 'HR@200'}
    result_fname = 'DimensionSelection'
    is_test = False
    plot_figure(fname_l=fname_l, dataset_l=dataset_l,
                name_m=name_m, method_m=method_m,
                result_fname=result_fname, test=is_test)


if __name__ == "__main__":
    dim_l = [4, 16, 64, 256]
    dataset_l = ['lastfm', 'ml-1m']
    # transform_data(dataset_l=dataset_l, dim_l=dim_l)
    plot_data()
