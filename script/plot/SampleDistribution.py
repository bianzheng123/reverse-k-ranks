import struct
import matplotlib.pyplot as plt
import matplotlib

matplotlib.rcParams.update({'font.size': 35})
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


def plot_figure(*, method_name: str,
                score_l: list,
                is_test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))

    subplot_str = 111
    ax = fig.add_subplot(subplot_str)

    ax.hist(score_l, color='#b2b2b2', bins=50, width=0.1)

    ax.set_xlabel('Score')
    ax.set_ylabel('Frequency')
    # ax.legend(frameon=False, bbox_to_anchor=(0.5, 1), loc="center", ncol=len(dataset_l), borderaxespad=5)
    # ax.set_xticks(np.arange(n_dataset), dataset_l)
    ax.set_xlim([0, 2.5])
    ax.set_ylim([0, 32])

    ax.margins(y=0.3)
    # fig.tight_layout(rect=(0.01, -0.07, 1.02, 1.05))
    if is_test:
        plt.savefig("SampleDistribution_{}.jpg".format(method_name), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("SampleDistribution_{}.pdf".format(method_name), bbox_inches='tight')


yahoomusic_qrs_m = get_sample_ip_l(
    'QueryRankSampleSearchKthRank-yahoomusic_big-n_sample_588-n_sample_query_5000-sample_topk_600',
    [3])
yahoomusic_rs_m = get_sample_ip_l(
    'RankSample-yahoomusic_big-n_sample_588',
    [3])
# yelp_qrs_m = get_sample_ip_l(
#     'QueryRankSampleSearchKthRank-yelp-n_sample_490-n_sample_query_5000-sample_topk_600',
#     [7])
# yelp_rs_m = get_sample_ip_l(
#     'RankSample-yelp-n_sample_490',
#     [7])

# score_l_l = [yahoomusic_qrs_m[3], yahoomusic_rs_m[3], yelp_qrs_m[7], yelp_rs_m[7]]
score_l_l = [yahoomusic_qrs_m[3], yahoomusic_rs_m[3]]
# title_l = ['Yahoomusic QAS', 'Yahoomusic US', 'Yelp QAS', 'Yelp US']
method_l = ['query_aware_sample', 'uniform_sample']
is_test = False
for score_l, method_name in zip(score_l_l, method_l):
    plot_figure(method_name=method_name, score_l=score_l, is_test=is_test)
