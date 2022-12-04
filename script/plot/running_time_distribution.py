import numpy as np
import matplotlib.pyplot as plt
import os
import re
import pandas as pd

dataset_m = {
    'movielens-27m': [283228, 53889],
    'netflix': [480189, 17770],
    'yahoomusic_big': [1948882, 98213],
    'yelp': [2189457, 160585],
    'book-crossing': [1, 2],
}


def show_bin_hist(arr, name, dataset_name):
    # 直方图会进行统计各个区间的数值
    fig, ax = plt.subplots()
    ax.hist(arr, bins='auto', color='#828487')

    # ax.set(xlim=(-5, 10), xticks=np.arange(-5, 10),   #)
    # ylim=(0, 1e8), yticks=np.arange(10000000, 90000000))
    n_user = dataset_m[dataset_name][0]
    n_data_item = dataset_m[dataset_name][1]
    # ax.set_yscale('log')
    # ax.set_title(
    #     '{}, user: {}, item: {}'.format(dataset_name, n_user, n_data_item))
    ax.set_xlabel('running time (million second)')
    ax.set_ylabel('frequency')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig('{}.jpg'.format(name), bbox_inches='tight', dpi=600)
    plt.close()


if __name__ == '__main__':
    dataset_name_l = ['netflix']
    basic_dir = '../../result/attribution'
    # for file in os.listdir('../../result/attribution'):
    kth_rank_fname = os.path.join("../../result/laptop/single_query_performance_dbg",
                                  'movielens-27m-QueryRankSampleSearchKthRank-top{}-n_sample_105-single-query-performance.csv'.format(
                                      100))
    df = pd.read_csv(kth_rank_fname)
    show_bin_hist(df['total_time'] * 1000, 'movielens-27m-QueryRankSampleSearchKthRank-running-time-distribution',
                  'movielens-27m')

    baseline_fname = os.path.join("../../result/laptop/single_query_performance_dbg",
                                  'movielens-27m-RankSample-top{}-n_sample_105-single-query-performance.csv'.format(
                                      100))
    df = pd.read_csv(baseline_fname)
    show_bin_hist(df['total_time'] * 1000, 'movielens-27m-RankSample-running-time-distribution',
                  'movielens-27m')

    # file_dir = 'score-distribution-{}.csv'.format(dataset_name)
    # arr = np.loadtxt(file_dir, delimiter=',')
    # n_interval = len(arr)
    # arr_idx_l = np.arange(0, n_interval, 1)
    # new_arr = arr[arr_idx_l]
    # print(np.sum(arr[:, 1]))
    # show_bin_hist(new_arr[:, 0], new_arr[:, 1], file_dir.split('.')[0], dataset_name)
    # print(file_dir)
