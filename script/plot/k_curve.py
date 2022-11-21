import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})


def plot_figure(*, fname_l: list, dataset_l: list,
                ylim_l: list,
                name_m: dict, method_m: dict, result_fname: str, test: bool, set_log_l: list, label_pos_l: list):
    assert len(fname_l) == len(dataset_l)
    n_fig = len(fname_l)
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(len(dataset_l) * 5 + 2, 4))
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
        ax.set_xlim([0, 210])
        ax.set_ylim(ylim_l[fig_i])
        if set_log_l[fig_i]:
            ax.set_yscale('log')
        ax.set_title(dataset_l[fig_i])
        ax.legend(frameon=False, loc='best')
    if test:
        plt.savefig("{}.jpg".format(result_fname), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format(result_fname), bbox_inches='tight')


if __name__ == "__main__":
    fname_l = ['./data/k_curve/Movielens-27m.csv',
               './data/k_curve/Yahoomusic.csv',
               './data/k_curve/Yelp.csv']
    dataset_l = ['Movielens', 'Yahoomusic', 'Yelp']
    # RS QRS QRSMinMaxIntLR QRSLSIntLR

    name_m = {'csv_x': 'topk', 'fig_x': r'k',
              'csv_y': 'RunningTime', 'fig_y': 'Running Time (Second)'}
    method_m = {'RS': 'US', 'QRSMinMaxIntLR': 'ASLR'}
    result_fname = 'k_running_time_overall_performance'
    is_test = True
    set_log_l = [True, True, True]
    label_pos_l = [0.06, 0.5]
    ylim_l = [[0.01, 1], [0.1, 100], [0.1, 100]]
    # ylim_l = [None, None, None]
    plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
                name_m=name_m, method_m=method_m,
                result_fname=result_fname, test=is_test, set_log_l=set_log_l, label_pos_l=label_pos_l)

    name_m = {'csv_x': 'topk', 'fig_x': r'k',
              'csv_y': 'IOCost', 'fig_y': 'IOCost'}
    method_m = {'RS': 'US', 'QRSMinMaxIntLR': 'ASLR'}
    result_fname = 'k_io_cost_overall_performance'
    set_log_l = [True, True, True]
    label_pos_l = [0.04, 0.5]
    ylim_l = [[0.01, 0.1], [0.1, 10], [0.1, 100]]
    plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
                name_m=name_m, method_m=method_m,
                result_fname=result_fname, test=is_test, set_log_l=set_log_l, label_pos_l=label_pos_l)

    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IPCost', 'fig_y': 'IPCost'}
    # method_m = {'RS': 'US', 'QRSMinMaxIntLR': 'ASLR'}
    # result_fname = 'k_ip_cost_overall_performance'
    # set_log_l = [True, True, True]
    # label_pos_l = [0.05, 0.5]
    # ylim_l = [[], [], []]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, label_pos_l=label_pos_l)
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'RunningTime', 'fig_y': 'Running Time (Second)'}
    # method_m = {'QRS': 'AS', 'QRSMinMaxIntLR': 'ASLR', 'QRSLSIntLR': 'ASLSLR'}
    # result_fname = 'k_running_time_component_performance'
    # set_log_l = [False, False, False]
    # label_pos_l = [0.05, 0.5]
    # ylim_l = [[], [], []]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, label_pos_l=label_pos_l)
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IOCost', 'fig_y': 'IOCost'}
    # method_m = {'QRS': 'AS', 'QRSMinMaxIntLR': 'ASLR', 'QRSLSIntLR': 'ASLSLR'}
    # result_fname = 'k_io_cost_component_performance'
    # set_log_l = [False, False, False]
    # label_pos_l = [0.05, 0.5]
    # ylim_l = [[], [], []]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, label_pos_l=label_pos_l)
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IPCost', 'fig_y': 'IPCost'}
    # method_m = {'QRS': 'AS', 'QRSMinMaxIntLR': 'ASLR', 'QRSLSIntLR': 'ASLSLR'}
    # result_fname = 'k_ip_cost_component_performance'
    # set_log_l = [True, True, True]
    # label_pos_l = [0.05, 0.5]
    # ylim_l = [[], [], []]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, label_pos_l=label_pos_l)
