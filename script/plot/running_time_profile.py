import matplotlib.pyplot as plt
import numpy as np
from matplotlib import ticker
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})
hatch = ['--', '+', 'x', '\\']
width = 0.35  # the width of the bars: can also be len(x) sequence


def plot_fig(*, label_l: list, label_m: dict, io_time_l: list, ip_time_l: list, is_test: bool):
    io_time_l = np.array(io_time_l)
    ip_time_l = np.array(ip_time_l)

    total_time_l = io_time_l + ip_time_l

    io_time_perc_l = io_time_l / total_time_l
    ip_time_perc_l = ip_time_l / total_time_l

    fig, ax = plt.subplots()
    ax.yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1, decimals=0))
    # ax.set_yscale('log')

    name_label_l = [label_m[_] for _ in label_l]

    ax.bar(name_label_l, ip_time_perc_l, width, color='#080604', edgecolor='#000000',
           hatch='//', label='Memory Index Time')
    ax.bar(name_label_l, io_time_perc_l, width, bottom=ip_time_perc_l, color='#ffffff', edgecolor='#000000',
           hatch='', label='Disk IO Time')

    # ax.set_yscale('log')

    ax.set_ylabel('Percentage of Running Time')
    ax.legend(frameon=False, loc='upper center')

    if is_test:
        plt.savefig("running_time_profile.jpg", dpi=600, bbox_inches='tight')
    else:
        plt.savefig("running_time_profile.pdf", bbox_inches='tight')


def plot_figure(*, dataset_l: list,
                fig_y: str, result_fname: str,
                io_time_l: list, ip_time_l: list,
                is_test: bool):
    n_dataset = len(dataset_l)
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(len(dataset_l) * 4 + 2, 4))
    fig.text(0.02, 0.5, fig_y, va='center', rotation='vertical')

    subplot_str = int('111')
    ax = fig.add_subplot(subplot_str)

    io_time_l = np.array(io_time_l)
    ip_time_l = np.array(ip_time_l)

    total_time_l = io_time_l + ip_time_l

    io_time_perc_l = io_time_l / total_time_l
    ip_time_perc_l = ip_time_l / total_time_l
    assert len(io_time_l) == len(ip_time_l)

    width = 0.35
    position_l = np.sort(np.append(np.arange(n_dataset) - width / 2, np.arange(n_dataset) + width / 2))
    assert len(io_time_l) == len(position_l)

    ip_time_ins = ax.bar(position_l, ip_time_perc_l, width - 0.1, color='#080604', edgecolor='#000000',
           hatch='//', label='CPU Time')
    io_time_ins = ax.bar(position_l, io_time_perc_l, width - 0.1, bottom=ip_time_perc_l, color='#ffffff',
           edgecolor='#000000',
           hatch='', label='IO Time')
    # ax.set_xlabel(dataset_l[fig_i])
    # ax.set_ylabel('')
    # ax.set_title(dataset_l[fig_i])
    # ax.legend(frameon=False, bbox_to_anchor=(0.5, 1), loc="center", ncol=len(dataset_l), borderaxespad=5)
    ax.legend(frameon=False, loc="upper center", ncol=len(dataset_l), borderaxespad=-0)
    ax.set_xticks(np.arange(n_dataset), dataset_l)

    ax.bar_label(io_time_ins, labels=['top10', 'top100', 'top10', 'top100'], padding=3)

    ax.yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1, decimals=0))
    ax.margins(y=0.3)

    fig.tight_layout(rect=(0.02, 0, 1, 1))
    if is_test:
        plt.savefig("{}.jpg".format(result_fname), dpi=600)
    else:
        plt.savefig("{}.pdf".format(result_fname))


if __name__ == "__main__":
    dataset_l = ['Movielens', 'Yahoomusic']
    fig_y = 'Percentage of running time'
    result_fname = 'RunningTimeProfileBeforeAdvanced'
    is_test = False

    # before advanced sampling solution
    io_time_l = [119.249 + 0.026, 227.787 + 0.049,
                 9863.397 + 2.568, 12872.496 + 3.319]
    ip_time_l = [41.884 + 71.492, 43.114 + 72.645,
                 273.071 + 318.303, 275.620 + 317.832]
    plot_figure(dataset_l=dataset_l,
                fig_y=fig_y, result_fname=result_fname,
                io_time_l=io_time_l, ip_time_l=ip_time_l,
                is_test=is_test)
    # after advanced sampling solution
    io_time_l = [5.895 + 0.001, 24.757 + 0.004,
                 24.197 + 0.002, 105.543 + 0.018]
    ip_time_l = [41.172 + 64.482, 41.850 + 65.017,
                 263.429 + 259.692, 271.123 + 261.595]
    result_fname = 'RunningTimeProfileAfterAdvanced'
    plot_figure(dataset_l=dataset_l,
                fig_y=fig_y, result_fname=result_fname,
                io_time_l=io_time_l, ip_time_l=ip_time_l,
                is_test=is_test)
