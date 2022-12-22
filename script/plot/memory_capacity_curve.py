import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, fname: str, dataset_name: str, ylim: list,
                name_m: dict, method_m: dict, ylog: bool, legend_loc: list, test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))
    subplot_str = 111
    ax = fig.add_subplot(subplot_str)
    df = pd.read_csv(fname)
    for method_i, key in enumerate(method_m.keys()):
        x_name = name_m['csv_x']
        y_name = key + name_m['csv_y']
        x_l = df[x_name]
        y_l = df[y_name]
        if name_m['csv_y'] == 'RunningTime':
            y_l = y_l / 1000
        elif name_m['csv_y'] == 'BuildIndex':
            y_l = y_l
        ax.plot(x_l, y_l,
                color='#000000', linewidth=2.5, linestyle='-',
                label=method_m[key],
                marker=marker_l[method_i], fillstyle='none', markersize=markersize)
    ax.set_xlim([1.8, 35])
    if ylim:
        ax.set_ylim(ylim)
    ax.set_xlabel(name_m['fig_x'])
    ax.set_ylabel(name_m['fig_y'])
    ax.set_xscale('log', base=2)
    ax.set_xticks([2, 4, 8, 16, 32])
    if ylog:
        ax.set_yscale('log', base=10)
    ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    # ax.legend(frameon=False, loc='best')
    ax.legend(frameon=False, loc=legend_loc[0], bbox_to_anchor=legend_loc[1])
    if test:
        plt.savefig("memory_capacity_{}.jpg".format(dataset_name), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("memory_capacity_{}.pdf".format(dataset_name), bbox_inches='tight')


if __name__ == "__main__":
    is_test = False

    name_m = {'csv_x': 'MemoryCapacity', 'fig_x': 'Memory Capacity (GB)',
              'csv_y': 'RunningTime', 'fig_y': 'Query Time (Second)'}
    method_m = {'RMIPS': 'RMIPS', 'RS': 'US', 'QRSMinMaxLR': 'QSRP'}
    fname_l = ['./data/memory_capacity_curve/Yahoomusic.csv',
               './data/memory_capacity_curve/Yelp.csv']
    dataset_name_l = ['1_yahoomusic_query_time', '2_yelp_query_time']
    # ylim_l = [None, None]
    ylog_l = [True, True]
    legend_loc_l = [('center right', (1, 0.6)), ('center right', (1, 0.6))]
    ylim_l = [[1e-1, 1e5], [1e-1, 1e5]]
    # ylim_l = [None, None]
    for fname, dataset_name, ylim, ylog, legend_loc in zip(fname_l, dataset_name_l, ylim_l, ylog_l, legend_loc_l):
        plot_figure(fname=fname, dataset_name=dataset_name, ylim=ylim,
                    name_m=name_m, method_m=method_m, ylog=ylog, legend_loc=legend_loc, test=is_test)

    name_m = {'csv_x': 'MemoryCapacity', 'fig_x': 'Memory Capacity (GB)',
              'csv_y': 'BuildIndex', 'fig_y': 'Build Index Time (Hour)'}
    method_m = {'RMIPS': 'RMIPS', 'RS': 'US', 'QRSMinMaxLR': 'QSRP'}
    fname_l = ['./data/memory_capacity_curve/Yahoomusic_build_index.csv',
               './data/memory_capacity_curve/Yelp_build_index.csv']
    dataset_name_l = ['1_yahoomusic_build_index', '2_yelp_build_index']
    # ylim_l = [None, None]
    ylog_l = [False, False]
    # legend_loc_l = [('center left', (1, 0.32)), ('center left', (1, 0.32))]
    legend_loc_l = [('upper left', None), ('upper left', None)]
    ylim_l = [[-2, 32], [-2, 40]]
    # ylim_l = [None, None]
    for fname, dataset_name, ylim, ylog, legend_loc in zip(fname_l, dataset_name_l, ylim_l, ylog_l, legend_loc_l):
        plot_figure(fname=fname, dataset_name=dataset_name, ylim=ylim,
                    name_m=name_m, method_m=method_m, ylog=ylog, legend_loc=legend_loc, test=is_test)
