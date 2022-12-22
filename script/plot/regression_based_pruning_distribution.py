import struct
import matplotlib.pyplot as plt
import numpy as np
import matplotlib
import scipy.stats as stats

matplotlib.rcParams.update({'font.size': 35})
hatch = ['--', '+', 'x', '\\']
width = 0.35  # the width of the bars: can also be len(x) sequence

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


def get_sample_ip_l(dir_name, file_name, userid_l):
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


def get_sample_lr(dir_name, file_name, userid_l):
    sizeof_size_t = 8
    sizeof_int = 4
    sizeof_double = 8

    f = open(
        '{}/{}.index'.format(dir_name, file_name),
        'rb')

    n_data_item_b = f.read(sizeof_size_t)
    n_user_b = f.read(sizeof_size_t)
    n_sample_rank_b = f.read(sizeof_int)

    n_data_item = struct.unpack("N", n_data_item_b)[0]
    n_user = struct.unpack("N", n_user_b)[0]
    n_sample_rank = struct.unpack("i", n_sample_rank_b)[0]

    print(n_sample_rank)
    print(n_data_item, n_user)

    # userid_l = np.random.choice(n_user, 20, replace=False)

    sample_arr_m = {}

    for userID in userid_l:
        f.seek(sizeof_size_t * 2 + sizeof_int + sizeof_int * n_sample_rank + userID * 2 * sizeof_double, 0)
        pred_para_l_b = f.read(2 * sizeof_double)
        pred_para_l = struct.unpack("d" * 2, pred_para_l_b)

        f.seek(sizeof_size_t * 2 + sizeof_int + sizeof_int * n_sample_rank + n_user * 2 * sizeof_double
               + userID * 2 * sizeof_double, 0)
        distribution_para_l_b = f.read(2 * sizeof_double)
        distribution_para_l = struct.unpack("d" * 2, distribution_para_l_b)

        f.seek(sizeof_size_t * 2 + sizeof_int + sizeof_int * n_sample_rank + n_user * 4 * sizeof_double
               + userID * sizeof_int, 0)
        error_b = f.read(sizeof_int)
        error = struct.unpack("i", error_b)

        sample_arr_m[userID] = [pred_para_l, distribution_para_l, error]

    f.close()
    return sample_arr_m


def plot_hist_figure(*, method_name: str,
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
        plt.savefig("regression_based_pruning_score_distribution_{}.jpg".format(method_name), bbox_inches='tight',
                    dpi=600)
    else:
        plt.savefig("regression_based_pruning_score_distribution_{}.pdf".format(method_name), bbox_inches='tight')


def plot_figure(*, method_name: str,
                score_l_l: list,
                rank_l_l: list,
                ylim_l: list,
                legend_loc: list,
                is_test: bool):
    # fig = plt.figure(figsize=(25, 4))
    fig = plt.figure(figsize=(6, 4))

    subplot_str = 111
    ax = fig.add_subplot(subplot_str)

    # ax.scatter(x=score_l_l[0], y=rank_l_l[0], s=2, label='sampled score')
    ax.plot(score_l_l[0], rank_l_l[0], color='#000000', linestyle='dashed', linewidth=3)

    ax.legend(frameon=False, loc=legend_loc[0], bbox_to_anchor=legend_loc[1])

    # for score_l, rank_l, i in zip(score_l_l, rank_l_l, np.arange(len(score_l_l))):
    #     ax.plot(score_l, rank_l, color='#b2b2b2', marker=marker_l[i], fillstyle='none', markersize=markersize)

    ax.set_xlabel('Score')
    ax.set_ylabel('Rank')
    # ax.legend(frameon=False, bbox_to_anchor=(0.5, 1), loc="center", ncol=len(dataset_l), borderaxespad=5)
    # ax.set_xticks(np.arange(n_dataset), dataset_l)
    # ax.set_xlim([0, 2.5])
    ax.set_ylim(ylim_l)

    # ax.margins(y=0.3)
    # fig.tight_layout(rect=(0.01, -0.07, 1.02, 1.05))
    if is_test:
        plt.savefig("regression_based_pruning_fitting_{}.jpg".format(method_name), bbox_inches='tight', dpi=600)
    else:
        plt.savefig("regression_based_pruning_fitting_{}.pdf".format(method_name), bbox_inches='tight')


userID = 1200
dir_name = '/home/zhengbian/reverse-k-ranks/index'
yelp_score_table_m = get_score_table_ip_l(
    dir_name=dir_name,
    file_name='yelp',
    dataset_name='yelp', userid_l=[userID])

score_l_l = [yelp_score_table_m[userID]]
method_l = ['1_Yelp']
is_test = True
for score_l, method_name in zip(score_l_l, method_l):
    plot_hist_figure(method_name=method_name, score_l=score_l, is_test=is_test)

dir_name = '/home/zhengbian/reverse-k-ranks/index/memory_index'

yelp_pre_m = get_sample_lr(dir_name, 'MinMaxLinearRegression-yelp-n_sample_405',
                           [userID])
yelp_qrs_m = get_sample_ip_l(
    dir_name,
    'QueryRankSampleIntLR-yelp-n_sample_405-n_sample_query_5000-sample_topk_600',
    [userID])

score_l_l = [yelp_qrs_m[userID]]
method_l = ['before_transformation', 'after_transformation']

score_l = score_l_l[0]
method_name = method_l[0]
rank_l = np.arange(len(score_l))
yelp_pre = yelp_pre_m[userID]
ylim_l = [0, 420]
print("second error {}".format(yelp_pre[2]))
rank_pred_l = [
    yelp_pre[0][0] * stats.norm.cdf((_ - yelp_pre[1][0]) / yelp_pre[1][1]) + yelp_pre[0][1]
    for _ in score_l
]
# rank_pred_error_l = [
#     yelp_pre[0][0] * stats.norm.cdf((_ - yelp_pre[1][0]) / yelp_pre[1][1]) + yelp_pre[0][1] + yelp_pre[2][0]
#     for _ in score_l
# ]
legend_loc = ['upper right', (1.05, 1.05)]
plot_figure(method_name=method_name, score_l_l=[score_l, rank_pred_l], rank_l_l=[rank_l, rank_l],
            ylim_l=ylim_l, legend_loc=legend_loc, is_test=is_test)
