import math
import struct
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm


def get_sample_ip_l():
    sizeof_size_t = 8
    sizeof_int = 4
    sizeof_double = 8

    f = open(
        '/home/bianzheng/reverse-k-ranks/index/memory_index_important/RankSample-movielens-27m-n_sample_947.index',
        'rb')

    n_sample_b = f.read(sizeof_size_t)
    max_sample_every_b = f.read(sizeof_size_t)
    n_data_item_b = f.read(sizeof_size_t)
    n_user_b = f.read(sizeof_size_t)

    n_sample = struct.unpack("N", n_sample_b)[0]
    max_sample_every = struct.unpack("N", max_sample_every_b)[0]
    n_data_item = struct.unpack("N", n_data_item_b)[0]
    n_user = struct.unpack("N", n_user_b)[0]
    print(n_sample, max_sample_every)
    print(n_data_item, n_user)

    # userid_l = np.random.choice(n_user, 20, replace=False)

    f.seek(sizeof_size_t * 4, 0)
    sample_ip_l_b = f.read(sizeof_int * n_sample)
    sample_ip_l = struct.unpack("i" * n_sample, sample_ip_l_b)

    f.close()
    return sample_ip_l


def show_hist(bins, dataset_name, name):
    # 直方图会进行统计各个区间的数值
    fig, ax = plt.subplots()
    ax.hist(bins, color='#b2b2b2', bins='auto')
    # ax.bar(bins, hist, color='#b2b2b2', width=30)  # alpha设置透明度，0为完全透明

    # ax.set(xlim=(-5, 10), xticks=np.arange(-5, 10),   #)
    # ylim=(0, 1e8), yticks=np.arange(10000000, 90000000))
    # n_user = dataset_m[dataset_name][0]
    # n_data_item = dataset_m[dataset_name][1]
    # ax.set_yscale('log')
    ax.set_title(
        'Uniform Sample IP Score Distribution, dataset: {}'.format(dataset_name))
    ax.set_xlabel('Rank')
    ax.set_ylabel('Frequency')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig(name, dpi=600, bbox_inches='tight')
    plt.close()


sample_rank_l = get_sample_ip_l()
show_hist(sample_rank_l, 'Movielens', 'pdf_rank_sample_user_{}.jpg'.format(0))
