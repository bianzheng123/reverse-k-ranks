import math
import struct
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm


def get_sample_ip_l():
    sizeof_size_t = 8
    sizeof_int = 4
    sizeof_double = 8

    # f = open(
    #     '/home/zhengbian/reverse-k-ranks/index/memory_index/QueryRankSampleSearchKthRank-yahoomusic_big-n_sample_588-n_sample_query_5000-sample_topk_600.index',
    #     'rb')
    # f = open(
    #     '/home/zhengbian/reverse-k-ranks/index/memory_index/RankSample-yahoomusic_big-n_sample_588.index',
    #     'rb')
    f = open(
        '/home/zhengbian/reverse-k-ranks/index/memory_index/QueryRankSampleSearchKthRank-yelp-n_sample_490-n_sample_query_5000-sample_topk_600.index',
        'rb')
    f = open(
        '/home/zhengbian/reverse-k-ranks/index/memory_index/RankSample-yahoomusic_big-n_sample_588.index',
        'rb')

    n_sample_b = f.read(sizeof_size_t)
    n_data_item_b = f.read(sizeof_size_t)
    n_user_b = f.read(sizeof_size_t)

    n_sample = struct.unpack("N", n_sample_b)[0]
    n_data_item = struct.unpack("N", n_data_item_b)[0]
    n_user = struct.unpack("N", n_user_b)[0]
    print(n_sample)
    print(n_data_item, n_user)

    # userid_l = np.random.choice(n_user, 20, replace=False)
    userid_l = np.arange(20)

    sample_arr_m = {}

    for userID in userid_l:
        f.seek(sizeof_size_t * 5 + sizeof_int * n_sample + userID * n_sample * sizeof_double, 0)
        sample_ip_l_b = f.read(n_sample * sizeof_double)
        sample_ip_l = struct.unpack("d" * n_sample, sample_ip_l_b)
        sample_arr_m[userID] = sample_ip_l

    f.close()
    return sample_arr_m


def show_hist(bins, dataset_name, name):
    # 直方图会进行统计各个区间的数值
    fig, ax = plt.subplots()
    ax.hist(bins, color='#b2b2b2', bins='auto', width=0.1)
    # ax.bar(bins, hist, color='#b2b2b2', width=30)  # alpha设置透明度，0为完全透明

    # ax.set(xlim=(-5, 10), xticks=np.arange(-5, 10),   #)
    # ylim=(0, 1e8), yticks=np.arange(10000000, 90000000))
    # n_user = dataset_m[dataset_name][0]
    # n_data_item = dataset_m[dataset_name][1]
    # ax.set_yscale('log')
    ax.set_title('{}'.format(dataset_name))
    ax.set_xlabel('Score')
    ax.set_ylabel('Frequency')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig('{}.jpg'.format(name), dpi=600, bbox_inches='tight')
    plt.savefig('{}.pdf'.format(name), bbox_inches='tight')
    plt.close()


sample_userID_ip_m = get_sample_ip_l()
for i, userID in enumerate(sample_userID_ip_m.keys(), 0):
    ip_l = sample_userID_ip_m[userID]
    show_hist(ip_l, 'Yahoomusic', 'pdf_query_rank_sample_user_{}'.format(i))
