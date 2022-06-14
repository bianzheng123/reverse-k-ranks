import numpy as np
import matplotlib.pyplot as plt
import os


def show_bin_hist(bins, hist, name):
    # 直方图会进行统计各个区间的数值
    plt.bar(bins, hist, color='fuchsia')  # alpha设置透明度，0为完全透明

    dataset_name = 'movielens-27m'
    n_user = 283228
    n_data_item = 53889
    plt.title(
        r'rank distribution of a single query, $\tau=512$' + '\n{}, user: {}, item: {}'.format(dataset_name, n_user,
                                                                                               n_data_item))
    plt.xlabel('column means # user within the rank interval')
    plt.ylabel('number of user')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig('{}.jpg'.format(name))
    plt.close()


if __name__ == '__main__':
    basic_dir = '../../result/attribution'
    for file in os.listdir('../../result/attribution'):
        if 'score-distribution-movielens-1m' in file:
            file_dir = os.path.join(basic_dir, file)
            arr = np.loadtxt(file_dir, delimiter=',')
            show_bin_hist(arr[:, 0], arr[:, 1], file.split('.')[0])
            print(file)
