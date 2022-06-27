import numpy as np
import matplotlib.pyplot as plt


def show_bin_hist(hist, name):
    bins = np.arange(1, len(hist) + 1)  # 设置连续的边界值，即直方图的分布区间[0,10],[10,20]...
    # 直方图会进行统计各个区间的数值
    plt.bar(bins, hist, color='fuchsia')  # alpha设置透明度，0为完全透明

    dataset_name = 'movielens-27m'
    n_user = 283228
    n_data_item = 53889
    plt.title(r'rank distribution of a single query, $\tau=512$' + '\n{}, user: {}, item: {}'.format(dataset_name, n_user, n_data_item))
    plt.xlabel('column means # user within the rank interval')
    plt.ylabel('number of user')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig('{}.jpg'.format(name))
    plt.close()


if __name__ == '__main__':
    arr = np.loadtxt('../../result/attribution/PrintUserRank/rank-hist-movielens-27m.csv', delimiter=',')
    print(len(arr))
    print(np.sum(arr))
    # for i in range(100):
    show_bin_hist(arr[793], 'fig-{}'.format(1))
