import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, fname: str, dataset_name: str, ylim: list,
                name_m: dict, method_m: dict, test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))
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
    ax.set_xlim([1.8, 35])
    ax.set_ylim(ylim)
    ax.set_xlabel(name_m['fig_x'])
    ax.set_ylabel(name_m['fig_y'])
    ax.set_xscale('log', base=2)
    ax.set_xticks([2, 4, 8, 16, 32])
    ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    ax.legend(frameon=False, loc='best')
    if test:
        plt.savefig("memory_capacity_running_time_{}.jpg".format(dataset_name), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("memory_capacity_running_time_{}.pdf".format(dataset_name), bbox_inches='tight')


if __name__ == "__main__":
    name_m = {'csv_x': 'MemoryCapacity', 'fig_x': 'Memory capacity (GB)',
              'csv_y': 'RunningTime', 'fig_y': 'Query Time (Second)'}
    method_m = {'QRS': 'AS', 'QRSMinMaxLR': 'ASLR'}
    is_test = False

    fname_l = ['./data/memory_capacity_curve/Yahoomusic.csv',
               './data/memory_capacity_curve/Yelp.csv']
    dataset_name_l = ['1_yahoomusic', '2_yelp']
    # ylim_l = [None, None]
    ylim_l = [[0, 1.5], [0, 5]]
    for fname, dataset_name, ylim in zip(fname_l, dataset_name_l, ylim_l):
        plot_figure(fname=fname, dataset_name=dataset_name, ylim=ylim,
                    name_m=name_m, method_m=method_m, test=is_test)
