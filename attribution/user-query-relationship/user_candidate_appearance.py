import numpy as np
import matplotlib.pyplot as plt

# n_user, n_item, n_query
dataset_m = {
    'fake-normal': [1000, 5000, 100],
    'movielens-27m': [283228, 52889, 1000],
    'movielens-small': [610, 9524, 200],
    'netflix': [480189, 16770, 1000]
}


def show_bin_hist(bins, hist, name):
    # 直方图会进行统计各个区间的数值
    fig, ax = plt.subplots()
    ax.bar(bins, hist, color='#000000', width=1)  # alpha设置透明度，0为完全透明

    # ax.set(xlim=(-5, 10), xticks=np.arange(-5, 10),   #)
    # ylim=(0, 1e8), yticks=np.arange(10000000, 90000000))
    # ax.set_yscale('log')
    # ax.set_title(
    #     '{}, user: {}, item: {}'.format(dataset_name, n_user, n_data_item))
    ax.set_xlabel('# user')
    ax.set_ylabel('candidate frequency')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig('{}.jpg'.format(name), dpi=600, bbox_inches='tight')
    plt.close()


if __name__ == '__main__':
    # for dataset_name in ['fake-normal']:
    for dataset_name in ['movielens-27m', 'netflix']:
        for topk in [10, 50]:
            # dataset_name = 'movielens-27m'
            # topk = 50
            n_user, n_item, n_query = dataset_m[dataset_name]
            freq_l = np.zeros(n_user, dtype=np.int32)
            for qID in range(n_query):
                with open('../../result/attribution/Candidate-{}/{}-top{}-qID-{}.txt'.format(
                        dataset_name, dataset_name, topk, qID), 'r') as f:
                    lines = f.read().split("\n")
                    for count, line in enumerate(lines, start=0):
                        if count % 2 == 0 and line != "":
                            userID = int(line.split(":")[0])
                            freq_l[userID] += 1
            freq_l = np.sort(freq_l)
            show_bin_hist(np.arange(n_user), freq_l, '{}-top{}'.format(dataset_name, topk))
            np.savetxt('{}-top{}.txt'.format(dataset_name, topk), freq_l, fmt="%d")
