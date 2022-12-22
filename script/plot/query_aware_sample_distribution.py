import struct
import matplotlib.pyplot as plt
import matplotlib

matplotlib.rcParams.update({'font.size': 35})
hatch = ['--', '+', 'x', '\\']
width = 0.35  # the width of the bars: can also be len(x) sequence


def get_sample_ip_l(*, dir_name: str, file_name: str, userid_l: list):
    sizeof_size_t = 8
    sizeof_int = 4
    sizeof_double = 8

    f = open(
        '{}/{}.index'.format(dir_name, file_name),
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


dataset_m = {'movielens-27m': [52889, 1000, 283228],
             'netflix': [16770, 1000, 480189],
             'yahoomusic_big': [135736, 1000, 1823179],
             'yahoomusic': [97213, 1000, 1948882],
             'yelp': [159585, 1000, 2189457],
             'goodreads': [2359650, 1000, 876145],
             'amazon-home-kitchen': [409243, 1000, 2511610],
             'yahoomusic_big_more_query': [135736, 1000, 1823179],
             'yelp_more_query': [159585, 1000, 2189457], }


def get_score_table_ip_l(*, dir_name: str, file_name: str,
                         dataset_name: str,
                         userid_l: list):
    # sizeof_size_t = 8
    # sizeof_int = 4
    sizeof_double = 8

    f = open(
        '{}/{}.index'.format(dir_name, file_name),
        'rb')
    n_user = dataset_m[dataset_name][2]
    n_data_item = dataset_m[dataset_name][0]

    # n_sample_b = f.read(sizeof_size_t)
    # n_data_item_b = f.read(sizeof_size_t)
    # n_user_b = f.read(sizeof_size_t)

    # n_sample = struct.unpack("N", n_sample_b)[0]
    # n_data_item = struct.unpack("N", n_data_item_b)[0]
    # n_user = struct.unpack("N", n_user_b)[0]
    # print(n_sample)
    print(n_data_item, n_user)

    sample_arr_m = {}

    for userID in userid_l:
        f.seek(userID * n_data_item * sizeof_double, 0)
        sample_ip_l_b = f.read(n_data_item * sizeof_double)
        sample_ip_l = struct.unpack("d" * n_data_item, sample_ip_l_b)
        sample_arr_m[userID] = sample_ip_l

    f.close()
    return sample_arr_m


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
        plt.savefig("query_aware_sample_distribution_{}.jpg".format(method_name), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("query_aware_sample_distribution_{}.pdf".format(method_name), bbox_inches='tight')


# yahoomusic_qrs_m = get_sample_ip_l(
#     '/home/zhengbian/reverse-k-ranks/index/memory_index',
#     'QueryRankSampleSearchKthRank-yelp-n_sample_490-n_sample_query_5000-sample_topk_600',
#     [3])
# yahoomusic_rs_m = get_sample_ip_l(
#     '/home/zhengbian/reverse-k-ranks/index/memory_index',
#     'RankSample-yelp-n_sample_490',
#     [3])

dir_name = '/home/zhengbian/reverse-k-ranks/index/memory_index'
yelp_qrs_m = get_sample_ip_l(
    dir_name=dir_name,
    file_name='QueryRankSampleSearchKthRank-yelp-n_sample_490-n_sample_query_5000-sample_topk_600',
    userid_l=[7])
yelp_rs_m = get_sample_ip_l(
    dir_name=dir_name,
    file_name='RankSample-yelp-n_sample_490',
    userid_l=[7])

dir_name = '/home/zhengbian/reverse-k-ranks/index'
yelp_score_table_m = get_score_table_ip_l(
    dir_name=dir_name,
    file_name='yelp',
    dataset_name='yelp', userid_l=[7])

score_l_l = [yelp_score_table_m[7], yelp_rs_m[7], yelp_qrs_m[7]]
# score_l_l = [yahoomusic_qrs_m[3], yahoomusic_rs_m[3]]
# title_l = ['Yahoomusic QAS', 'Yahoomusic US', 'Yelp QAS', 'Yelp US']
method_l = ['query_aware_sample', 'uniform_sample']
is_test = True
for score_l, method_name in zip(score_l_l, method_l):
    plot_figure(method_name=method_name, score_l=score_l, is_test=is_test)
