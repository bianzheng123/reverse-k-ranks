import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import numpy as np

linestyle_l = ['_', '-', '--', ':']
# color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15
hatch_l = ['//', '\\', '||', '++']
color_l = ['#ffffff', '#ffffff', '#000000']
style_l = [(None, '#ffffff'), ('\\', '#ffffff'), ('//', '#ffffff'), (None, '#000000')]

matplotlib.rcParams.update({'font.size': 15})


def plot_figure(*, fname: str, dataset: str, set_log: bool, ylim: list, labelpad: int,
                name_m: dict, method_m: dict, result_fname_prefix: str, is_test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))
    subplot_str = 111
    ax = fig.add_subplot(subplot_str)
    df = pd.read_csv(fname)
    topk_l = [int(topk) for topk in df['topk']]

    width = 0.25 if len(method_m.keys()) != 4 else 0.2

    offset = width / 2 if len(method_m.keys()) == 2 else width if len(method_m.keys()) == 3 else 1.5 * width

    # offset = width / len(method_m.keys())
    rects_l = []

    for method_i, key in enumerate(method_m.keys()):
        y_name = key + name_m['csv_y']
        x_l = np.arange(len(topk_l))
        y_l = df[y_name]
        if name_m['csv_y'] == 'RunningTime' or name_m['csv_y'] == 'IPCost' or name_m['csv_y'] == "MemoryIndexTime":
            y_l = y_l / 1000
        elif name_m['csv_y'] == 'IOCost':
            y_l = y_l / 1000 * 8 / 1024
        else:
            raise Exception('not find csv_y')
        if len(method_m.keys()) == 2:
            rects = ax.bar(x_l + offset - method_i * width,
                           y_l, width,
                           color=style_l[method_i + 1][1], edgecolor='#000000',
                           hatch=style_l[method_i + 1][0], label=method_m[key])
        elif len(method_m.keys()) == 3:
            rects = ax.bar(x_l + offset - method_i * width,
                           y_l, width,
                           color=style_l[method_i][1], edgecolor='#000000',
                           hatch=style_l[method_i][0], label=method_m[key])
        elif len(method_m.keys()) == 4:
            rects = ax.bar(x_l + offset - method_i * width,
                           y_l, width,
                           color=style_l[method_i][1], edgecolor='#000000',
                           hatch=style_l[method_i][0], label=method_m[key])
        rects_l.append(rects)

    ax.set_ylim(ylim)
    # ax.set_xlabel(dataset_l[fig_i])
    ax.set_ylabel(name_m['fig_y'], labelpad=labelpad)
    if set_log:
        ax.set_yscale('log')
    # ax.set_title(dataset_l[fig_i])
    # ax.legend(frameon=False, bbox_to_anchor=(0.5, 1), loc="center", ncol=len(dataset_l), borderaxespad=5)
    ax.legend(frameon=False, loc="upper center", ncol=len(method_m.keys()), borderaxespad=-0)
    # ax.set_xticks(np.arange(n_dataset), dataset_l)
    # ax.set_yticks([0, 0.5, 1.0])
    x_name = name_m['csv_x']
    ax.set_xticks(np.arange(len(topk_l)), ['{:.0f}'.format(topk) for topk in df[x_name]])
    ax.set_xlabel('k')

    # ax.bar_label(io_time_ins, labels=['Top-10', 'Top-100', 'Top-10', 'Top-100'], padding=7)
    # ax.margins(y=50)
    # fig.tight_layout()
    if is_test:
        plt.savefig("{}_{}.jpg".format(result_fname_prefix, dataset), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("{}_{}.pdf".format(result_fname_prefix, dataset), bbox_inches='tight')


'''
    # fname_l = ['./data/k_curve/Movielens.csv',
    #            './data/k_curve/Yahoomusic.csv',
    #            './data/k_curve/Yelp.csv',
    #            './data/k_curve/Amazon.csv']
    # dataset_l = ['1_Movielens', '2_Yahoomusic', '3_Yelp', '4_Amazon']
    # set_log_l = [True, True, True, True]
    # # ylim_l = [[0.01, 1], [0.1, 100], [0.1, 100], [1, 100001]]
    # ylim_l = [[0.01, 9000], [0.1, 200000], [0.1, 600000], [1, 3e18]]
    # labelpad_l = [0, -5, -5, 0]
    # # labelpad_l = [0, 0, 0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'RunningTime', 'fig_y': 'Query Time (Second)'}
    # method_m = {'RMIPS': 'RMIPS', 'RS': 'US', 'QRSMinMax': 'QSRO'}
    # result_fname_prefix = 'k_running_time_overall_performance_bar'
    # for fname, dataset, set_log, ylim, labelpad in zip(fname_l, dataset_l,
    #                                                    set_log_l, ylim_l,
    #                                                    labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, is_test=is_test)
    
    # fname_l = ['./data/k_curve/Yahoomusic.csv',
    #            './data/k_curve/Yelp.csv']
    # dataset_l = ['1_Yahoomusic', '2_Yelp']
    # set_log_l = [False, False]
    # ylim_l = [[0.0, 0.8], [0.2, 0.9]]
    # # ylim_l = [None, None, None, None]
    # labelpad_l = [0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'RunningTime', 'fig_y': 'Query Time (Second)'}
    # method_m = {'QRS': 'QS', 'QRSDLR': 'QSRP-DT', 'QRSMinMax': 'QSRO'}
    # result_fname_prefix = 'k_running_time_component_performance'
    # for fname, dataset, set_log, ylim, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, is_test=is_test)
    '''

if __name__ == "__main__":
    is_test = True

    # fname_l = ['./data/k_curve/Movielens.csv',
    #            './data/k_curve/Yahoomusic.csv',
    #            './data/k_curve/Yelp.csv',
    #            './data/k_curve/Amazon.csv']
    # dataset_l = ['1_Movielens', '2_Yahoomusic', '3_Yelp', '4_Amazon']
    # set_log_l = [True, True, True, True]
    # ylim_l = [[1e-1, 7e1], [1e-1, 4e4], [1e-1, 8e4], [1e3, 6e5]]
    # # ylim_l = [None, None, None, None]
    # labelpad_l = [0, 0, 0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IOCost', 'fig_y': 'Disk Read (KB)'}
    # method_m = {'RS': 'US', 'QRSMinMax': 'QSRP'}
    # result_fname_prefix = 'k_io_cost_overall_performance_bar'
    # for fname, dataset, set_log, ylim, labelpad in zip(fname_l, dataset_l,
    #                                                    set_log_l, ylim_l,
    #                                                    labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, is_test=is_test)

    fname_l = ['./data/k_curve/Movielens.csv',
               './data/k_curve/Yahoomusic.csv',
               './data/k_curve/Yelp.csv',
               './data/k_curve/Amazon.csv']
    dataset_l = ['1_Movielens', '2_Yahoomusic', '3_Yelp', '4_Amazon']
    set_log_l = [True, True, True, True]
    ylim_l = [[1e4, 8e9], [1e4, 5e10], [1e4, 4e10], [1e5, 5e12]]
    # ylim_l = [None, None, None, None]
    labelpad_l = [0, -3, -3, 0]

    name_m = {'csv_x': 'topk', 'fig_x': r'k',
              'csv_y': 'IPCost', 'fig_y': '# Score Computation'}
    method_m = {'RMIPS': 'RMIPS', 'Grid': 'Grid', 'RS': 'US', 'QRSMinMax': 'QSRP'}
    result_fname_prefix = 'k_ip_cost_overall_performance_bar'
    for fname, dataset, set_log, ylim, labelpad in zip(fname_l, dataset_l,
                                                       set_log_l, ylim_l,
                                                       labelpad_l):
        plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, labelpad=labelpad,
                    name_m=name_m, method_m=method_m,
                    result_fname_prefix=result_fname_prefix, is_test=is_test)

    # fname_l = ['./data/k_curve/Yahoomusic.csv',
    #            './data/k_curve/Yelp.csv']
    # dataset_l = ['1_Yahoomusic', '2_Yelp']
    # set_log_l = [False, False]
    # ylim_l = [[0, 24], [0, 37]]
    # # ylim_l = [None, None, None, None]
    # labelpad_l = [0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IOCost', 'fig_y': 'Disk Read (KB)'}
    # # method_m = {'QRS': 'QS', 'QRSDLR': 'QSRP-DT', 'QRSMinMax': 'QSRP'}
    # method_m = {'QRSDLR': 'QSRP-DT', 'QRSMinMax': 'QSRP'}
    # result_fname_prefix = 'k_io_cost_component_performance_bar'
    # for fname, dataset, set_log, ylim, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, is_test=is_test)

    # fname_l = ['./data/k_curve/Yahoomusic.csv',
    #            './data/k_curve/Yelp.csv']
    # dataset_l = ['1_Yahoomusic', '2_Yelp']
    # set_log_l = [True, True]
    # ylim_l = [[1e2, 7e4], [1e4, 1e6]]
    # # ylim_l = [None, None, None, None]
    # labelpad_l = [0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IPCost', 'fig_y': '# Score Computation'}
    # # method_m = {'QRS': 'QS', 'QRSDLR': 'QSRP-DT', 'QRSMinMax': 'QSRP'}
    # method_m = {'QRSDLR': 'QSRP-DT', 'QRSMinMax': 'QSRP'}
    # result_fname_prefix = 'k_ip_cost_component_performance_bar'
    # for fname, dataset, set_log, ylim, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, is_test=is_test)

    # fname_l = ['./data/k_curve/Yahoomusic_memory_index_time.csv',
    #            './data/k_curve/Yelp_memory_index_time.csv']
    # dataset_l = ['1_Yahoomusic', '2_Yelp']
    # set_log_l = [False, False]
    # ylim_l = [[0.0, 0.19], [0, 0.55]]
    # # ylim_l = [None, None]
    # labelpad_l = [0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'MemoryIndexTime', 'fig_y': 'Memory Index Time (Second)'}
    # # method_m = {'QS': 'QS', 'QSDT': 'QSRP-DT', 'QSRO': 'QSRP'}
    # method_m = {'QSDT': 'QSRP-DT', 'QSRO': 'QSRP'}
    # result_fname_prefix = 'k_memory_index_time_component_performance_bar'
    # for fname, dataset, set_log, ylim, labelpad in zip(fname_l, dataset_l,
    #                                                    set_log_l, ylim_l,
    #                                                    labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, is_test=is_test)

    # fname_l = ['./data/k_curve/Movielens_grid.csv']
    # dataset_l = ['1_Movielens']
    # set_log_l = [True]
    # ylim_l = [[1e4, 5e7]]
    # # ylim_l = [None]
    # legend_loc_l = [('best', None)]
    # labelpad_l = [0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IPCost', 'fig_y': '# Score Computation'}
    # method_m = {'Grid': 'Grid', 'RS': 'US', 'QRSMinMax': 'QSRP'}
    # result_fname_prefix = 'k_ip_cost_grid_performance_bar'
    # for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l, legend_loc_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, is_test=is_test)
