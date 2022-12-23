import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 17})


def plot_figure(*, fname: str, dataset: str, set_log: bool, ylim: list, legend_loc: tuple, labelpad: int,
                name_m: dict, method_m: dict, result_fname_prefix: str, test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))
    # fig.text(label_pos_l[0], label_pos_l[1], name_m['fig_y'], va='center', rotation='vertical')
    subplot_str = 111
    ax = fig.add_subplot(subplot_str)
    df = pd.read_csv(fname)
    for method_i, key in enumerate(method_m.keys()):
        x_name = name_m['csv_x']
        y_name = key + name_m['csv_y']
        y_l = df[y_name]
        if name_m['csv_y'] == 'RunningTime' or name_m['csv_y'] == 'IPCost':
            y_l = y_l / 1000
        elif name_m['csv_y'] == 'IOCost':
            y_l = y_l / 1000 * 8 / 1024

        if result_fname_prefix == 'k_io_cost_overall_performance':
            ax.plot(df[x_name], y_l,
                    color='#000000', linewidth=2.5, linestyle='-',
                    label=method_m[key],
                    marker=marker_l[method_i + 1], fillstyle='none', markersize=markersize)
        else:
            ax.plot(df[x_name], y_l,
                    color='#000000', linewidth=2.5, linestyle='-',
                    label=method_m[key],
                    marker=marker_l[method_i], fillstyle='none', markersize=markersize)

    ax.set_xlabel(name_m['fig_x'])
    ax.set_xlim([0, 210])
    ax.set_ylabel(name_m['fig_y'], labelpad=labelpad)
    if ylim:
        ax.set_ylim(ylim)
    if set_log:
        ax.set_yscale('log')
    ax.legend(frameon=False, loc=legend_loc[0], bbox_to_anchor=legend_loc[1])
    if test:
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
    # ylim_l = [[1e-1, 4e1], [1e-1, 8e3], [1e-1, 2e4], [1e3, 3e5]]
    # # ylim_l = [None, None, None, None]
    # legend_loc_l = [('lower right', None), ('lower right', None), ('lower right', None), ('lower right', None)]
    # labelpad_l = [0, 0, 0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IOCost', 'fig_y': 'Disk Read (KB)'}
    # method_m = {'RS': 'US', 'QRSMinMax': 'QSRO'}
    # result_fname_prefix = 'k_io_cost_overall_performance'
    # for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l, legend_loc_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, legend_loc=legend_loc, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, test=is_test)
    
    # fname_l = ['./data/k_curve/Movielens.csv',
    #            './data/k_curve/Yahoomusic.csv',
    #            './data/k_curve/Yelp.csv',
    #            './data/k_curve/Amazon.csv']
    # dataset_l = ['1_Movielens', '2_Yahoomusic', '3_Yelp', '4_Amazon']
    # set_log_l = [True, True, True, True]
    # ylim_l = [[1e4, 3e9], [1e4, 1e10], [1e4, 1e10], [1e5, 1.6e12]]
    # # ylim_l = [None, None, None, None]
    # legend_loc_l = [('best', None), ('center right', (1, 0.6)), ('center right', (1, 0.6)), ('best', None)]
    # labelpad_l = [0, -5, -7, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IPCost', 'fig_y': '# Score Computation'}
    # method_m = {'RMIPS': 'RMIPS', 'RS': 'US', 'QRSMinMax': 'QSRO'}
    # result_fname_prefix = 'k_ip_cost_overall_performance'
    # for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l, legend_loc_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, legend_loc=legend_loc, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, test=is_test)
    
    # fname_l = ['./data/k_curve/Yahoomusic.csv',
    #            './data/k_curve/Yelp.csv']
    # dataset_l = ['1_Yahoomusic', '2_Yelp']
    # set_log_l = [False, False]
    # ylim_l = [[0, 22], [0, 35]]
    # # ylim_l = [None, None, None, None]
    # legend_loc_l = [('lower right', None), ('lower right', None)]
    # labelpad_l = [0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IOCost', 'fig_y': 'Disk Read (KB)'}
    # method_m = {'QRS': 'QS', 'QRSDLR': 'QS-DT', 'QRSMinMax': 'QSRO'}
    # result_fname_prefix = 'k_io_cost_component_performance'
    # for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l, legend_loc_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, legend_loc=legend_loc, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, test=is_test)
    
    # fname_l = ['./data/k_curve/Yahoomusic.csv',
    #            './data/k_curve/Yelp.csv']
    # dataset_l = ['1_Yahoomusic', '2_Yelp']
    # set_log_l = [True, True]
    # ylim_l = [[1e4, 5e6], [1e4, 5e6]]
    # # ylim_l = [None, None, None, None]
    # legend_loc_l = [('best', None), ('center right', (1, 0.43))]
    # labelpad_l = [0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IPCost', 'fig_y': '# Score Computation'}
    # method_m = {'QRS': 'QS', 'QRSDLR': 'QS-DT', 'QRSMinMax': 'QSRO'}
    # result_fname_prefix = 'k_ip_cost_component_performance'
    # for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l, legend_loc_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, legend_loc=legend_loc, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, test=is_test)
    
    # fname_l = ['./data/k_curve/Movielens_grid.csv']
    # dataset_l = ['1_Movielens']
    # set_log_l = [True]
    # ylim_l = [[1e4, 2e7]]
    # # ylim_l = [None]
    # legend_loc_l = [('best', None)]
    # labelpad_l = [0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'IPCost', 'fig_y': '# Score Computation'}
    # method_m = {'Grid': 'Grid', 'RS': 'US', 'QRSMinMax': 'QSRP'}
    # result_fname_prefix = 'k_ip_cost_grid_performance'
    # for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l, legend_loc_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, legend_loc=legend_loc, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, test=is_test)
    '''

if __name__ == "__main__":
    is_test = True

    fname_l = ['./data/k_curve/Movielens.csv',
               './data/k_curve/Yahoomusic.csv',
               './data/k_curve/Yelp.csv',
               './data/k_curve/Amazon.csv']
    dataset_l = ['1_Movielens', '2_Yahoomusic', '3_Yelp', '4_Amazon']
    set_log_l = [True, True, True, True]
    # ylim_l = [[0.01, 1], [0.1, 100], [0.1, 100], [1, 100001]]
    # ylim_l = [[0.01, 3000], [0.1, 100000], [0.1, 300000], [1, 5e7]] # for not grid index
    ylim_l = [[0.01, 1e4], [0.1, 100000], [0.1, 300000], [1, 1e6]]
    # ylim_l = [None, None, None, None]
    legend_loc_l = [('best', None), ('center right', (1, 0.6)), ('center right', (1, 0.6)), ('center right', (1, 0.6))]
    labelpad_l = [0, -5, -5, 0]
    # labelpad_l = [0, 0, 0, 0]

    name_m = {'csv_x': 'topk', 'fig_x': r'k',
              'csv_y': 'RunningTime', 'fig_y': 'Query Time (Second)'}
    method_m = {'RMIPS': 'RMIPS', 'RS': 'US', 'QRSMinMax': 'QSRP'}
    result_fname_prefix = 'k_running_time_overall_performance'
    for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
                                                                   set_log_l, ylim_l, legend_loc_l,
                                                                   labelpad_l):
        plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, legend_loc=legend_loc, labelpad=labelpad,
                    name_m=name_m, method_m=method_m,
                    result_fname_prefix=result_fname_prefix, test=is_test)

    # fname_l = ['./data/k_curve/Yahoomusic.csv',
    #            './data/k_curve/Yelp.csv']
    # dataset_l = ['1_Yahoomusic', '2_Yelp']
    # set_log_l = [False, False]
    # ylim_l = [[0.0, 0.6], [0.2, 0.9]]
    # # ylim_l = [None, None, None, None]
    # legend_loc_l = [('best', None), ('best', None)]
    # labelpad_l = [0, 0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'RunningTime', 'fig_y': 'Query Time (Second)'}
    # # method_m = {'QRS': 'QS', 'QRSDLR': 'QSRP-DT', 'QRSMinMax': 'QSRP'}
    # method_m = {'QRSDLR': 'QSRP-DT', 'QRSMinMax': 'QSRP'}
    # result_fname_prefix = 'k_running_time_component_performance'
    # for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l, legend_loc_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, legend_loc=legend_loc, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, test=is_test)

    # fname_l = ['./data/k_curve/Movielens_grid.csv']
    # dataset_l = ['1_Movielens']
    # set_log_l = [True]
    # ylim_l = [[1e-2, 5e4]]
    # # ylim_l = [None]
    # legend_loc_l = [('best', None)]
    # labelpad_l = [0]
    #
    # name_m = {'csv_x': 'topk', 'fig_x': r'k',
    #           'csv_y': 'RunningTime', 'fig_y': 'Query Time (Second)'}
    # method_m = {'Grid': 'Grid', 'RS': 'US', 'QRSMinMax': 'QSRP'}
    # result_fname_prefix = 'k_running_time_grid_performance'
    # for fname, dataset, set_log, ylim, legend_loc, labelpad in zip(fname_l, dataset_l,
    #                                                                set_log_l, ylim_l, legend_loc_l,
    #                                                                labelpad_l):
    #     plot_figure(fname=fname, dataset=dataset, set_log=set_log, ylim=ylim, legend_loc=legend_loc, labelpad=labelpad,
    #                 name_m=name_m, method_m=method_m,
    #                 result_fname_prefix=result_fname_prefix, test=is_test)
