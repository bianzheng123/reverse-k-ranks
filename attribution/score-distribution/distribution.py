import numpy as np
import matplotlib.pyplot as plt
import os

dataset_m = {
    'movielens-27m': [283228, 53889],
    'netflix': [480189, 17770]
}


def show_bin_hist(bins, hist, name, dataset_name):
    # 直方图会进行统计各个区间的数值
    print(bins)
    fig, ax = plt.subplots()
    ax.bar(bins, hist, color='fuchsia', width=1)  # alpha设置透明度，0为完全透明

    # ax.set(xlim=(-5, 10), xticks=np.arange(-5, 10),   #)
    # ylim=(0, 1e8), yticks=np.arange(10000000, 90000000))
    n_user = dataset_m[dataset_name][0]
    n_data_item = dataset_m[dataset_name][1]
    # ax.set_yscale('log')
    # ax.set_title(
    #     '{}, user: {}, item: {}'.format(dataset_name, n_user, n_data_item))
    ax.set_xlabel('score')
    ax.set_ylabel('frequency')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig('{}.jpg'.format(name))
    plt.close()


if __name__ == '__main__':
    basic_dir = '../../result/attribution'
    for file in os.listdir('../../result/attribution'):
        if 'score-distribution-netflix.csv' in file:
            file_dir = os.path.join(basic_dir, file)
            arr = np.loadtxt(file_dir, delimiter=',')
            n_interval = len(arr)
            arr_idx_l = np.arange(0, n_interval, 1)
            new_arr = arr[arr_idx_l]
            print(np.sum(arr[:, 1]))
            show_bin_hist(new_arr[:, 0], new_arr[:, 1], file.split('.')[0], 'netflix')
            print(file)
        # if 'score-distribution-movielens-27m.csv' in file:
        #     file_dir = os.path.join(basic_dir, file)
        #     arr = np.loadtxt(file_dir, delimiter=',')
        #     n_interval = len(arr)
        #     arr_idx_l = np.arange(0, n_interval, 1)
        #     new_arr = arr[arr_idx_l]
        #     print(np.sum(arr[:, 1]))
        #     show_bin_hist(new_arr[:, 0], new_arr[:, 1], file.split('.')[0], 'movielens-27m')
        #     print(file)
