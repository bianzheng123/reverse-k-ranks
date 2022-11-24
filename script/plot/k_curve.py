import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 15})


def plot_figure(*, fname_l: list, dataset_l: list,
                ylim_l: list,
                name_m: dict, method_m: dict, result_fname: str, test: bool, set_log_l: list, labelpad_l: list,
                loc_l: list):
    assert len(fname_l) == len(dataset_l) == len(set_log_l) == len(ylim_l) == len(loc_l)
    n_fig = len(fname_l)
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(len(dataset_l) * 5 + 3, 4))
    # fig.text(label_pos_l[0], label_pos_l[1], name_m['fig_y'], va='center', rotation='vertical')
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
        ax.set_ylabel(name_m['fig_y'], labelpad=labelpad_l[fig_i])
        if ylim_l[fig_i]:
            ax.set_ylim(ylim_l[fig_i])
        if set_log_l[fig_i]:
            ax.set_yscale('log')
        ax.set_title(dataset_l[fig_i])
        ax.legend(frameon=False, loc=loc_l[fig_i][0], bbox_to_anchor=loc_l[fig_i][1])
    if test:
        plt.savefig("{}.jpg".format(result_fname), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}.pdf".format(result_fname), bbox_inches='tight')


if __name__ == "__main__":
    fname_l = ['./data/k_curve/Movielens.csv',
               './data/k_curve/Yahoomusic.csv',
               './data/k_curve/Yelp.csv',
               './data/k_curve/Amazon.csv']
    dataset_l = ['Movielens', 'Yahoomusic', 'Yelp', 'Amazon']
    # RS QRS QRSMinMaxIntLR QRSLSIntLR
    is_test = True

    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'RunningTime', 'fig_y': 'Running Time (Second)'}
    # method_m = {'RS': 'US', 'QRSMinMaxLR': 'ASLR', 'RMIPS': 'RMIPS'}
    # result_fname = 'k_running_time_overall_performance'
    # set_log_l = [True, True, True, True]
    # # ylim_l = [[0.01, 1], [0.1, 100], [0.1, 100], [1, 100001]]
    # ylim_l = [[0.01, 10000], [0.1, 100000], [0.1, 100000], [1, 100001]]
    # loc_l = [('best', None), ('center right', (1, 0.6)), ('center right', (1, 0.6)), ('center right', (1, 0.7))]
    # labelpad_l = [0, -5, -5, 0]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, labelpad_l=labelpad_l,
    #             loc_l=loc_l)
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IOCost', 'fig_y': 'IO Cost'}
    # method_m = {'RS': 'US', 'QRSMinMaxLR': 'ASLR'}
    # result_fname = 'k_io_cost_overall_performance'
    # set_log_l = [True, True, True, True]
    # ylim_l = [[10, 10000], [100, 1000000], [100, 10000000], [100000, 100000000]]
    # # ylim_l = [None, None, None, None]
    # loc_l = [('lower right', None), ('best', None), ('best', None), ('lower right', None)]
    # labelpad_l = [0, 0, 0, 0]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, labelpad_l=labelpad_l,
    #             loc_l=loc_l)
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IPCost', 'fig_y': 'IP Cost'}
    # method_m = {'RS': 'US', 'QRSMinMaxLR': 'ASLR', 'RMIPS': 'RMIPS'}
    # result_fname = 'k_ip_cost_overall_performance'
    # set_log_l = [True, True, True, True]
    # ylim_l = [[1e4, 1e10], [1e4, 1e11], [1e4, 1e10], [1e5, 1e10]]
    # # ylim_l = [None, None, None, None]
    # loc_l = [('best', None), ('best', None), ('center right', (1, 0.6)), ('best', None)]
    # labelpad_l = [-5, -5, -7, -5]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, labelpad_l=labelpad_l,
    #             loc_l=loc_l)
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'RunningTime', 'fig_y': 'Running Time (Second)'}
    # method_m = {'QRS': 'AS', 'QRSMinMaxLR': 'ASLR', 'QRSDLR': 'ASDLR'}
    # result_fname = 'k_running_time_component_performance'
    # set_log_l = [False, False, False, False]
    # ylim_l = [[0, 0.35], [0.0, 0.8], [0.2, 0.9], [2.5, 16.0]]
    # # ylim_l = [None, None, None, None]
    # loc_l = [('best', None), ('best', None), ('best', None), ('best', None)]
    # labelpad_l = [0, 0, 0, 0]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, labelpad_l=labelpad_l,
    #             loc_l=loc_l)
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IOCost', 'fig_y': 'IO Cost'}
    # method_m = {'QRS': 'AS', 'QRSMinMaxLR': 'ASLR', 'QRSDLR': 'ASDLR'}
    # result_fname = 'k_io_cost_component_performance'
    # set_log_l = [False, False, False, False]
    # ylim_l = [None, [0, 3000], [0, 4500], [2e5, 1.3e6]]
    # # ylim_l = [None, None, None, None]
    # loc_l = [('lower right', None), ('lower right', None), ('lower right', None), ('lower right', None)]
    # labelpad_l = [0, 0, 0, 0]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, labelpad_l=labelpad_l,
    #             loc_l=loc_l)
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IPCost', 'fig_y': 'IPCost'}
    # method_m = {'QRS': 'AS', 'QRSMinMaxLR': 'ASLR', 'QRSDLR': 'ASDLR'}
    # result_fname = 'k_ip_cost_component_performance'
    # set_log_l = [True, True, True, True]
    # ylim_l = [[1e4, 1e6], [1e4, 5e6], [1e4, 5e6], [1e5, 4e6]]
    # # ylim_l = [None, None, None, None]
    # loc_l = [('best', None), ('best', None), ('center right', (1, 0.45)), ('lower right', None)]
    # labelpad_l = [0, 0, 0, 0]
    # plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
    #             name_m=name_m, method_m=method_m,
    #             result_fname=result_fname, test=is_test, set_log_l=set_log_l, labelpad_l=labelpad_l,
    #             loc_l=loc_l)

    name_m = {'csv_x': 'topk', 'fig_x': r'k',
              'csv_y': 'RunningTime', 'fig_y': 'Running Time (Second)'}
    method_m = {'RS': 'US', 'RMIPS': 'RMIPS', 'QRS': 'AS',
                'QRSMinMaxLR': 'ASLR', 'QRSDLR': 'ASDLR'}
    result_fname = 'k_running_time_total_overall_performance'
    set_log_l = [True, True, True, True]
    ylim_l = [[0.01, 10000], [0.1, 100000], [0.1, 100000], [1, 100001]]
    # ylim_l = [None, None, None, None]
    loc_l = [('best', None), ('best', None), ('center right', (1, 0.45)), ('lower right', None)]
    labelpad_l = [0, 0, 0, 0]
    plot_figure(fname_l=fname_l, dataset_l=dataset_l, ylim_l=ylim_l,
                name_m=name_m, method_m=method_m,
                result_fname=result_fname, test=is_test, set_log_l=set_log_l, labelpad_l=labelpad_l,
                loc_l=loc_l)
