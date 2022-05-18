import numpy as np
import matplotlib.pyplot as plt


def show_bin_hist(hist, name):
    bins = np.arange(1, len(hist) + 1)  # 设置连续的边界值，即直方图的分布区间[0,10],[10,20]...
    # 直方图会进行统计各个区间的数值
    plt.bar(bins, hist, color='fuchsia')  # alpha设置透明度，0为完全透明

    plt.title('movielens-27m, user: 283,228, item: 53,889')
    plt.xlabel('interval number, from top to bottom')
    plt.ylabel('number of user')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig('{}.jpg'.format(name))
    plt.close()


if __name__ == '__main__':
    arr = np.loadtxt('../../result/attribution/print-user-rank-movielens-27m.csv', delimiter=',')
    print(len(arr))
    print(np.sum(arr))
    # for i in range(100):
    show_bin_hist(arr, 'fig-{}'.format(1))
