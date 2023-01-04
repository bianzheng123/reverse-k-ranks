import math

import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import pandas as pd
import re

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 20})


def compile_rmips(*, fname: str, require_topk: int):
    file = open(fname)
    lines = file.read().split("\n")
    skip_queryID = -1
    result_query_l = []
    for line in lines:
        match_obj = re.match(
            r'\[.*\] \[info\] queryID (.*), result_size (.*), rtk_topk (.*), accu_ip_cost (.*), accu_query_time (.*)s,',
            line)
        if match_obj:
            queryID = int(match_obj.group(1))
            result_size = int(match_obj.group(2))
            rtk_topk = int(match_obj.group(3))
            accu_ip_cost = int(match_obj.group(4))
            accu_query_time = float(match_obj.group(5))

            if skip_queryID == queryID:
                continue
            else:
                if result_size > require_topk:
                    result_query_l.append((queryID, result_size, accu_ip_cost, accu_query_time))
                    skip_queryID = queryID
        # else:
        #     print("No match!!")
    total_ip = np.sum([_[2] for _ in result_query_l])
    total_time = np.sum([_[3] for _ in result_query_l])
    # print(np.setdiff1d(np.arange(670), np.array([_[0] for _ in result_query_l])))
    print("No.total query {}, total IP {}, total time {}s".format(len(result_query_l), total_ip, total_time))
    return result_query_l


def scale_time_by_ip_cost(result_query_l: list, ip_cost=3933048998, total_time=5554.200):
    pred_running_time_l = [float(_[2]) / ip_cost * total_time for _ in result_query_l]
    # print(pred_running_time_l)
    return pred_running_time_l


def plot_figure(*, method_name: str, total_time_l: list,
                xlim: list, ylim: list, n_bin: int,
                time95: float, time_median: float, time_max: float,
                weight: float,
                name_m: dict, is_test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))
    subplot_str = 111
    ax = fig.add_subplot(subplot_str)
    # counts, bins = np.histogram(total_time_l, bins=n_bin, weights=np.ones(len(total_time_l)) * weight)
    # print(counts)
    # assert 1000 - 0.1 <= np.sum(counts) <= 1000 + 0.1
    # ax.stairs(counts, bins, color='#828487', fill=True)
    # print(np.logspace(2, 5, num=100))

    ax.hist(total_time_l, bins=np.logspace(math.log10(0.01), math.log10(15000), num=n_bin),
            weights=np.ones(len(total_time_l)) * weight, color='#828487', density=False)

    ax.plot(np.ones(len(np.arange(1, 10002, 20))) * time95, np.arange(1, 10002, 20),
            color='#ee1c25', linewidth=3, linestyle='solid')
    ax.plot(np.ones(len(np.arange(1, 10002, 20))) * time_median, np.arange(1, 10002, 20),
            color='#0000ff', linewidth=3, linestyle='solid')
    ax.plot(np.ones(len(np.arange(1, 10002, 20))) * time_max, np.arange(1, 10002, 20),
            color='#00ff00', linewidth=3, linestyle='solid')
    # ax.plot(time95)

    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel(name_m['fig_x'])
    ax.set_ylabel(name_m['fig_y'])
    ax.set(xticks=[1e-2, 1e-1, 1e0, 1e1, 1e2, 1e3, 1e4])
    if xlim:
        ax.set_xlim(xlim)
    if ylim:
        ax.set_ylim(ylim)

    if is_test:
        plt.savefig("single_query_performance_{}.jpg".format(method_name), bbox_inches='tight')
    else:
        plt.savefig("single_query_performance_{}.pdf".format(method_name), bbox_inches='tight')


# def plot_figure(*, method_name: str, total_time_l: list,
#                 xlim: list, ylim: list, n_bin: int, time95: float,
#                 weight: float,
#                 name_m: dict, is_test: bool):
#     # fig = plt.figure(figsize=(25, 4))
#     fig = plt.figure(figsize=(6, 4))
#     subplot_str = 111
#     ax = fig.add_subplot(subplot_str)
#     # counts, bins = np.histogram(total_time_l, bins=n_bin, weights=np.ones(len(total_time_l)) * weight)
#     # print(counts)
#     # assert 1000 - 0.1 <= np.sum(counts) <= 1000 + 0.1
#     # ax.stairs(counts, bins, color='#828487', fill=True)
#     ax.hist(total_time_l, bins=n_bin, weights=np.ones(len(total_time_l)) * weight, color='#828487', density=False)
#
#     ax.plot(np.ones(len(np.arange(1, 10002, 20))) * time95, np.arange(1, 10002, 20),
#             color='#ee1c25', linewidth=1, linestyle='dotted')
#     # ax.plot(time95)
#
#     ax.set_xscale('log')
#     ax.set_yscale('log')
#     ax.set_xlabel(name_m['fig_x'])
#     ax.set_ylabel(name_m['fig_y'])
#     if xlim:
#         ax.set_xlim(xlim)
#     if ylim:
#         ax.set_ylim(ylim)
#
#     if is_test:
#         plt.savefig("query_time_{}.jpg".format(method_name), bbox_inches='tight')
#     else:
#         plt.savefig("query_time_{}.pdf".format(method_name), bbox_inches='tight')


def count_percentile(running_time_l, percentile):
    sorted_time_l = np.sort(running_time_l)
    idx = int(len(sorted_time_l) * percentile)
    return sorted_time_l[idx]


if __name__ == '__main__':
    result_query_l = compile_rmips(fname='./data/single_query_performance/raw_data/rmips-movielens-27m.log',
                                   require_topk=50)
    rmips_running_time_l = scale_time_by_ip_cost(result_query_l=result_query_l)

    fname_l = [
        './data/single_query_performance/movielens-27m-RankSample-top50-n_sample_3791-single-query-performance.csv',
        './data/single_query_performance/movielens-27m-QueryRankSampleMinMaxIntLR-top50-n_sample_3698-n_sample_query_5000-sample_topk_600-single-query-performance.csv']
    # fname_l = [
    #     './data/single_query_performance/yahoomusic_big-RankSample-top50-n_sample_588-single-query-performance.csv',
    #     './data/single_query_performance/yahoomusic_big-QueryRankSampleSearchKthRank-top50-n_sample_588-single-query-performance.csv']
    data_l = []
    for fname in fname_l:
        data_l.append(pd.read_csv(fname)['total_time'])
    data_l.append(rmips_running_time_l)

    xlim = [0.01, 1.5e4]
    ylim = [8, 1e4]
    time95_l = [count_percentile(_, 0.95) for _ in data_l]
    median_l = [count_percentile(_, 0.5) for _ in data_l]
    max_l = [np.max(_) for _ in data_l]

    # method_name_l = ['1_uniform_sample', '2_query_aware_sample_regression_optimization']
    # bins_l = [5000, 200]
    # weights_l = [1, 1, 1000 / 659]

    method_name_l = ['1_uniform_sample', '2_query_aware_sample_regression_optimization', '3_rmips']
    bins_l = [50, 30, 30]
    weights_l = [1 * 10, 1 * 10, 1000 / 659 * 10]

    name_m = {'csv_x': 'total_time', 'fig_x': 'Query Time (Second)',
              'fig_y': 'Frequency'}
    is_test = False
    for total_time_l, method_name, n_bin, time95, time_median, time_max, weight in zip(data_l, method_name_l,
                                                                                       bins_l,
                                                                                       time95_l, median_l, max_l,
                                                                                       weights_l):
        print("method_name {}, min_time {}, max_time {}".format(method_name,
                                                                np.min(total_time_l),
                                                                np.max(total_time_l)))
        plot_figure(method_name=method_name, total_time_l=total_time_l,
                    xlim=xlim, ylim=ylim, n_bin=n_bin,
                    time95=time95, time_median=time_median, time_max=time_max,
                    weight=weight,
                    name_m=name_m, is_test=is_test)
