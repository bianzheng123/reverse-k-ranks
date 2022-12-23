import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import numpy as np

linestyle_l = ['_', '-', '--', ':']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15
hatch_l = ['//', '\\', '||', '++']
color_l = ['#ffffff', '#ffffff', '#000000']
style_l = [(None, '#ffffff'), ('\\', '#ffffff'), (None, '#000000')]

matplotlib.rcParams.update({'font.size': 15})


def plot_figure(*, fname: str, dataset: str, set_log: bool, ylim: list,
                name_m: dict, method_m: dict, is_test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))
    subplot_str = 111
    ax = fig.add_subplot(subplot_str)
    df = pd.read_csv(fname)
    memory_capacity_l = [int(memory_capacity) for memory_capacity in df['MemoryCapacity']]
    width = 0.25
    offset = width / 2 if len(method_m.keys()) == 2 else width
    rects_l = []

    for method_i, key in enumerate(method_m.keys()):
        y_name = key + name_m['csv_y']
        x_l = np.arange(len(memory_capacity_l))
        y_l = df[y_name]
        if name_m['csv_y'] == 'RunningTime':
            y_l = y_l / 1000
        elif name_m['csv_y'] == 'BuildIndex':
            y_l = y_l
        else:
            raise Exception('not find csv_y')
        rects = ax.bar(x_l + offset - method_i * width,
                       y_l, width,
                       color=style_l[method_i][1], edgecolor='#000000',
                       hatch=style_l[method_i][0], label=method_m[key])
        rects_l.append(rects)

    ax.set_ylim(ylim)
    # ax.set_xlabel(dataset_l[fig_i])
    ax.set_ylabel(name_m['fig_y'])
    if set_log:
        ax.set_yscale('log')
    # ax.set_title(dataset_l[fig_i])
    # ax.legend(frameon=False, bbox_to_anchor=(0.5, 1), loc="center", ncol=len(dataset_l), borderaxespad=5)
    ax.legend(frameon=False, loc="upper center", ncol=len(method_m.keys()), borderaxespad=-0)
    # ax.set_xticks(np.arange(n_dataset), dataset_l)
    # ax.set_yticks([0, 0.5, 1.0])
    x_name = name_m['csv_x']
    ax.set_xticks(np.arange(len(memory_capacity_l)), [memory_capacity for memory_capacity in df[x_name]])

    # ax.bar_label(io_time_ins, labels=['Top-10', 'Top-100', 'Top-10', 'Top-100'], padding=7)
    # ax.margins(y=50)
    # fig.tight_layout()
    if is_test:
        plt.savefig("memory_capacity_{}_bar.jpg".format(dataset), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("memory_capacity_{}_bar.pdf".format(dataset), bbox_inches='tight')


if __name__ == "__main__":
    is_test = False

    # name_m = {'csv_x': 'MemoryCapacity', 'fig_x': 'Memory capacity (GB)',
    #           'csv_y': 'RunningTime', 'fig_y': 'Query Time (Second)'}
    # method_m = {'RMIPS': 'RMIPS', 'RS': 'US', 'QRSMinMaxLR': 'QSRO'}
    # fname_l = ['./data/memory_capacity_curve/Yahoomusic.csv',
    #            './data/memory_capacity_curve/Yelp.csv']
    # dataset_name_l = ['1_yahoomusic_query_time', '2_yelp_query_time']
    # # ylim_l = [None, None]
    # ylog_l = [True, True]
    # legend_loc_l = [('center right', (1, 0.6)), ('center right', (1, 0.6))]
    # ylim_l = [[1e-1, 1e5], [1e-1, 1e5]]
    # # ylim_l = [None, None]
    # for fname, dataset_name, ylim, ylog, legend_loc in zip(fname_l, dataset_name_l, ylim_l, ylog_l, legend_loc_l):
    #     plot_figure(fname=fname, dataset_name=dataset_name, ylim=ylim,
    #                 name_m=name_m, method_m=method_m, ylog=ylog, legend_loc=legend_loc, test=is_test)

    name_m = {'csv_x': 'MemoryCapacity', 'fig_x': 'Memory capacity (GB)',
              'csv_y': 'BuildIndex', 'fig_y': 'Build Index Time (Hour)'}
    method_m = {'RMIPS': 'RMIPS', 'RS': 'US', 'QRSMinMaxLR': 'QSRO'}
    fname_l = ['./data/memory_capacity_curve/Yahoomusic_build_index.csv',
               './data/memory_capacity_curve/Yelp_build_index.csv']
    dataset_name_l = ['1_yahoomusic_build_index', '2_yelp_build_index']
    # ylim_l = [None, None]
    ylog_l = [True, True]
    ylim_l = [[0.008, 90], [0.008, 90]]
    # ylim_l = [None, None]
    for fname, dataset_name, ylim, ylog in zip(fname_l, dataset_name_l, ylim_l, ylog_l):
        plot_figure(fname=fname, dataset=dataset_name, set_log=ylog, ylim=ylim,
                    name_m=name_m, method_m=method_m, is_test=is_test)
