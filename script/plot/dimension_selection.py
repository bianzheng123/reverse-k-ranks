import json
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['D', "v", "o", "x", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 15})


def transform_data(*, dataset_l: list, dim_l: list):
    for dataset in dataset_l:
        hit_50_l = []
        hit_100_l = []
        hit_200_l = []
        for dim in dim_l:
            with open('data/dimension_selection/previous_raw_data/hitting_rate-{}-{}-new.json'.format(dataset, dim), 'r') as f:
                json_ins = json.load(f)
            hit_50_l.append(json_ins['test_result']['hit@50'])
            hit_100_l.append(json_ins['test_result']['hit@100'])
            hit_200_l.append(json_ins['test_result']['hit@200'])
        df = pd.DataFrame({'dimension': dim_l, 'HR@50': hit_50_l, 'HR@100': hit_100_l, 'HR@200': hit_200_l})
        df.to_csv("data/dimension_selection/{}.csv".format(dataset), index=False)


def plot_figure(*, fname: str, result_fname: str,
                name_m: dict, method_m: dict, ylim: list, test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(5, 4))
    subplot_str = 111
    ax = fig.add_subplot(subplot_str)
    df = pd.read_csv(fname)
    for method_i, key in enumerate(method_m.keys()):
        x_name = name_m['csv_x']
        y_name = key + name_m['csv_y']
        ax.plot(df[x_name], df[y_name],
                color='#000000', linewidth=2.5, linestyle='-',
                label=method_m[key],
                marker=marker_l[method_i], fillstyle='none', markersize=markersize)
    ax.set_ylim(ylim)

    ax.set_xlabel(name_m['fig_x'])
    ax.set_ylabel(name_m['fig_y'])
    ax.legend(frameon=False, loc='best', borderaxespad=-0)
    # fig.tight_layout(rect=(0, 0.1, 1, 1))
    if test:
        plt.savefig("{}_{}.jpg".format('DimensionSelection', result_fname), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}_{}.pdf".format('DimensionSelection', result_fname), bbox_inches='tight')


if __name__ == "__main__":
    dim_l = [4, 16, 64, 256]
    # transform_data(dataset_l=dataset_l, dim_l=dim_l)
    fname_l = ['./data/dimension_selection/lastfm.csv',
               './data/dimension_selection/ml-1m.csv']
    result_fname_l = ['1_lastfm', '2_movielens-1m']
    ylim_l = [[0.2, 0.6], None]

    name_m = {'csv_x': 'dimension', 'fig_x': 'Dimensionality',
              'csv_y': '', 'fig_y': 'Hitting Ratio'}
    method_m = {'HR@200': 'HR@200', 'HR@100': 'HR@100', 'HR@50': 'HR@50'}
    is_test = False
    for fname, result_fname, ylim in zip(fname_l, result_fname_l, ylim_l):
        plot_figure(fname=fname, result_fname=result_fname,
                    name_m=name_m, method_m=method_m,
                    ylim=ylim, test=is_test)
