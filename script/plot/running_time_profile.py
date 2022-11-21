import matplotlib.pyplot as plt
import numpy as np
from matplotlib import ticker
import matplotlib
import math

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
    fig = plt.figure(figsize=(len(dataset_l) * 4 + 2, 5))
    fig.text(0.01, 0.5, fig_y, va='center', rotation='vertical')

    subplot_str = int('111')
    ax = fig.add_subplot(subplot_str)

    io_time_l = np.array(io_time_l)
    ip_time_l = np.array(ip_time_l)

    total_time_l = io_time_l + ip_time_l

    io_time_perc_l = io_time_l / total_time_l
    ip_time_perc_l = ip_time_l / total_time_l
    assert len(io_time_l) == len(ip_time_l)

    width = 0.35
    position_l = np.sort(np.append(np.arange(n_dataset) - width / 7 * 5, np.arange(n_dataset) + width / 7 * 5))
    assert len(io_time_l) == len(position_l)

    ip_time_ins = ax.bar(position_l, ip_time_perc_l, width - 0.1, color='#080604', edgecolor='#000000',
           hatch='//', label='CPU Time')
    io_time_ins = ax.bar(position_l, io_time_perc_l, width - 0.1, bottom=ip_time_perc_l, color='#ffffff',
           edgecolor='#000000',
           hatch='', label='IO Time')
    ax.set_ylim([0, 1.19])
    # ax.set_xlabel(dataset_l[fig_i])
    # ax.set_ylabel('')
    # ax.set_title(dataset_l[fig_i])
    # ax.legend(frameon=False, bbox_to_anchor=(0.5, 1), loc="center", ncol=len(dataset_l), borderaxespad=5)
    ax.legend(frameon=False, loc="upper center", ncol=len(dataset_l), borderaxespad=-0.2)
    ax.set_xticks(np.arange(n_dataset), dataset_l)

    ax.bar_label(io_time_ins, labels=['top10', 'top100', 'top10', 'top100'], padding=3)

    ax.yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1.0, decimals=0))
    ax.margins(y=50)

    fig.tight_layout(rect=(0.01, -0.05, 1.02, 1.05))
    # fig.tight_layout()
    if is_test:
        plt.savefig("{}.jpg".format(result_fname), dpi=600)
    else:
        plt.savefig("{}.pdf".format(result_fname))


if __name__ == "__main__":
    dataset_l = ['Yahoomusic', 'Yelp']
    fig_y = 'Percentage of running time'
    result_fname = 'RunningTimeProfileBeforeAdvanced'
    is_test = False

    # before advanced sampling solution
    io_time_l = [9863.397, 12872.496,
                 14179.423, 15957.713]
    ip_time_l = [273.071 + 318.303 + 2.568, 275.620 + 317.832 + 3.319,
                 325.387 + 332.568 + 3.741, 328.890 + 332.496 + 4.242]
    plot_figure(dataset_l=dataset_l,
                fig_y=fig_y, result_fname=result_fname,
                io_time_l=io_time_l, ip_time_l=ip_time_l,
                is_test=is_test)
    # after advanced sampling solution
    io_time_l = [24.197, 105.543,
                 23.604, 108.599]
    ip_time_l = [263.429 + 259.692 + 0.002, 271.123 + 261.595 + 0.018,
                 318.403 + 304.638 + 0.003, 323.632 + 305.373 + 0.022]
    result_fname = 'RunningTimeProfileAfterAdvanced'
    plot_figure(dataset_l=dataset_l,
                fig_y=fig_y, result_fname=result_fname,
                io_time_l=io_time_l, ip_time_l=ip_time_l,
                is_test=is_test)
