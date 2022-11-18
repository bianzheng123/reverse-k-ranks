import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, fname_l: list, dataset_l: list,
                name_m: dict, method_m: dict, result_fname: str, test: bool, set_log_l: list, label_pos_l: list):
    assert len(fname_l) == len(dataset_l)
    n_fig = len(fname_l)
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(len(dataset_l) * 4 + 2, 4))
    fig.text(label_pos_l[0], label_pos_l[1], name_m['fig_y'], va='center', rotation='vertical')
    for fig_i in range(n_fig):
        subplot_str = int('1' + str(n_fig) + str(fig_i + 1))
        ax = fig.add_subplot(subplot_str)
        df = pd.read_csv(fname_l[fig_i])
        for method_i, key in enumerate(method_m.keys()):
            x_name = name_m['csv_x']
            y_name = key + name_m['csv_y']
            ax.plot(df[x_name], df[y_name] / 1000,
                    color='#000000', linewidth=2.5, linestyle='-',
                    label=method_m[key],
                    marker=marker_l[method_i], fillstyle='none', markersize=markersize)

        ax.set_xlabel(name_m['fig_x'])
        if set_log_l[fig_i]:
            ax.set_yscale('log')
        ax.set_title(dataset_l[fig_i])
        if fig_i == n_fig - 1:
            ax.legend(frameon=False, loc='best')
        pass
    pass
    if test:
        plt.savefig("{}.jpg".format(result_fname), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format(result_fname), bbox_inches='tight')


if __name__ == "__main__":
    fname_l = ['./data/memory_capacity_curve/Yahoomusic.csv',
               './data/memory_capacity_curve/Yelp.csv']
    dataset_l = ['Yahoomusic', 'Yelp']
    # RS QRS QRSMinMaxIntLR QRSLSIntLR

    name_m = {'csv_x': 'MemoryCapacity', 'fig_x': 'Memory capacity (GB)',
              'csv_y': 'RunningTime', 'fig_y': 'Running Time (Second)'}
    method_m = {'QRS': 'AS', 'QRSMinMaxLR': 'ASLR'}
    result_fname = 'memory_capacity_running_time'
    is_test = False
    set_log_l = [False, False]
    label_pos_l = [0.04, 0.5]
    plot_figure(fname_l=fname_l, dataset_l=dataset_l,
                name_m=name_m, method_m=method_m,
                result_fname=result_fname, test=is_test, set_log_l=set_log_l, label_pos_l=label_pos_l)
