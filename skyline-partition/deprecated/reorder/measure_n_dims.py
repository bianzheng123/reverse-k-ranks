import vecs_io
import numpy as np
import matplotlib.pyplot as plt
import os
import json
import sys

norm_max = 0.8


# 原始排序到达80%的norm需要经过多少个维度
def show_hist(hist, dataset_name, filename, n_item):
    if not os.path.isdir('result/%s' % dataset_name):
        os.system('mkdir -p result/%s' % dataset_name)

    bins = np.arange(1, len(hist) + 1)  # 设置连续的边界值，即直方图的分布区间[0,10],[10,20]...
    # 直方图会进行统计各个区间的数值
    plt.bar(bins, hist, color='fuchsia')  # alpha设置透明度，0为完全透明

    plt.xlabel('n_dimension')
    plt.ylabel('frequency')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig('result/%s/%s.jpg' % (dataset_name, filename))
    plt.close()
    np.savetxt('result/%s/%s.txt' % (dataset_name, filename), hist, fmt='%d')

    avg_freq = np.sum([(i + 1) * hist[i] for i in range(len(hist))]) / n_item
    json_f = {
        'average_frequency': avg_freq
    }
    with open('result/%s/%s.json' % (dataset_name, filename), 'w') as f:
        json.dump(json_f, f)


def measure_number_dims(dataset, method_name):
    base, dim = vecs_io.fvecs_read('data/%s/%s.fvecs' % (dataset, method_name))
    print(method_name, dataset, base.shape)

    norm_l = np.linalg.norm(base, axis=1)
    norm_l = norm_l * norm_l

    freq_l = np.zeros(shape=dim, dtype=np.int32)
    for i, vecs in enumerate(base, 0):
        arr = np.array(abs(vecs))
        arr = arr * arr
        cum_sum = 0
        for j in range(len(arr)):
            cum_sum += arr[j]
            if cum_sum > norm_l[i] * norm_max:
                freq_l[j] += 1
                break
    show_hist(freq_l, dataset, method_name, len(base))

    query, dim = vecs_io.fvecs_read('data/%s/%s_query.fvecs' % (dataset, method_name))
    print(method_name, dataset, query.shape)

    norm_l = np.linalg.norm(query, axis=1)
    norm_l = norm_l * norm_l

    freq_l = np.zeros(shape=dim, dtype=np.int32)
    for i, vecs in enumerate(query, 0):
        arr = np.array(abs(vecs))
        arr = arr * arr
        cum_sum = 0
        for j in range(len(arr)):
            cum_sum += arr[j]
            if cum_sum > norm_l[i] * norm_max:
                freq_l[j] += 1
                break
    show_hist(freq_l, dataset, '%s-query' % method_name, len(query))


if __name__ == '__main__':
    method_name = sys.argv[1]
    ds = sys.argv[2]
    print("measure", ds, method_name)
    measure_number_dims(ds, method_name)
