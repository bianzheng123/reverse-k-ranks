import struct
import matplotlib.pyplot as plt
import matplotlib
import numpy as np
from scipy.stats import norm

matplotlib.rcParams.update({'font.size': 20})
hatch = ['--', '+', 'x', '\\']
width = 0.35  # the width of the bars: can also be len(x) sequence


def get_sample_ip_l(file_name, userid_l):
    sizeof_size_t = 8
    sizeof_int = 4
    sizeof_double = 8

    f = open(
        '/home/bianzheng/reverse-k-ranks/index/memory_index_important/{}.index'.format(file_name),
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

    sample_arr_m = {}

    for userID in userid_l:
        f.seek(sizeof_size_t * 3 + sizeof_int * n_sample + userID * n_sample * sizeof_double, 0)
        sample_ip_l_b = f.read(n_sample * sizeof_double)
        sample_ip_l = struct.unpack("d" * n_sample, sample_ip_l_b)
        sample_arr_m[userID] = sample_ip_l

    f.close()
    return sample_arr_m


def show_hist(bins, dataset_name, name):
    # 直方图会进行统计各个区间的数值
    fig, ax = plt.subplots()
    ax.hist(bins, color='#b2b2b2', bins=50, width=0.1)
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
    # plt.savefig('{}.pdf'.format(name), bbox_inches='tight')
    plt.close()


def plot_figure(*, result_fname: str,
                sample_score_l: list,
                rank_l: list, lim=None):
    assert len(sample_score_l) == len(rank_l)
    fig = plt.figure(figsize=(1 * 4 + 2, 4))

    subplot_str = int('111')
    ax = fig.add_subplot(subplot_str)
    ax.plot(sample_score_l, rank_l, color='#b2b2b2')

    ax.set_xlabel('Score')
    ax.set_ylabel('Rank')
    ax.set_xlim(lim)
    # ax.legend(frameon=False, bbox_to_anchor=(0.5, 1), loc="center", ncol=len(dataset_l), borderaxespad=5)
    # ax.set_xticks(np.arange(n_dataset), dataset_l)

    fig.tight_layout()
    plt.savefig("{}.jpg".format(result_fname), dpi=600)


sample_score_l = get_sample_ip_l(
    'QueryRankSampleSearchKthRank-yahoomusic_big-n_sample_588-n_sample_query_5000-sample_topk_600',
    [3])[3]
rank_l = np.arange(len(sample_score_l))
rank_l = [norm.ppf(_ / len(sample_score_l)) for _ in rank_l]
# yahoomusic_rs_m = get_sample_ip_l(
#     'RankSample-yahoomusic_big-n_sample_588',
#     [3])
# yelp_qrs_m = get_sample_ip_l(
#     'QueryRankSampleSearchKthRank-yelp-n_sample_490-n_sample_query_5000-sample_topk_600',
#     [7])
# yelp_rs_m = get_sample_ip_l(
#     'RankSample-yelp-n_sample_490',
#     [7])

result_fname = 'SampleDistribution'

plot_figure(sample_score_l=sample_score_l, rank_l=rank_l, result_fname=result_fname)

for i in range(20):
    sample_score_l = get_sample_ip_l(
        'QueryRankSampleSearchKthRank-yahoomusic_big-n_sample_588-n_sample_query_5000-sample_topk_600',
        [i])[i]
    # sample_score_l = np.array(sample_score_l) * 2
    rank_l = np.arange(len(sample_score_l))
    result_fname = 'SampleDistribution_{}'.format(i)
    plot_figure(sample_score_l=sample_score_l, rank_l=rank_l, result_fname=result_fname, lim=[0, 10])

result_fname = 'SampleDistribution_before'

rank_l = np.arange(len(sample_score_l))
plot_figure(sample_score_l=sample_score_l, rank_l=rank_l, result_fname=result_fname)

x_l = np.linspace(start=-4, stop=4, num=100)
y_l = [-norm.cdf(_) for _ in x_l]
result_fname = 'simulation'
plot_figure(sample_score_l=x_l, rank_l=y_l, result_fname=result_fname)