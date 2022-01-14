import vecs_io
import numpy as np
import matplotlib.pyplot as plt
import os
import sys
import time


# 对于原始数据, 当所有向量达到80%norm时, 每一维度出现的次数

def show_hist(hist, dataset_name, method):
    if not os.path.isdir('result/%s' % dataset_name):
        os.system('mkdir -p result/%s' % dataset_name)

    bins = np.arange(1, len(hist) + 1)  # 设置连续的边界值，即直方图的分布区间[0,10],[10,20]...
    # 直方图会进行统计各个区间的数值
    plt.bar(bins, hist, color='fuchsia')  # alpha设置透明度，0为完全透明

    plt.xlabel('dimension')
    plt.ylabel('frequency')
    # plt.xlim(0, 100)  # 设置x轴分布范围
    plt.savefig('result/%s/%s.jpg' % (dataset_name, method))
    plt.close()
    np.savetxt('result/%s/%s.txt' % (dataset_name, method), hist, fmt='%d')


if __name__ == '__main__':
    dataset_type = sys.argv[1]
    ds = sys.argv[2]
    method_intermediate_name = '%s_dim_weight_intermediate' % dataset_type
    method_name = '%s_dim_weight' % dataset_type

    start_time = time.time()
    base, dim = vecs_io.fvecs_read('data/%s/%s.fvecs' % (ds, dataset_type))
    query, dim = vecs_io.fvecs_read('data/%s/%s_query.fvecs' % (ds, dataset_type))

    freq_l = np.zeros(shape=dim, dtype=np.float32)
    for i, vecs in enumerate(base, 0):
        arr = np.array(abs(vecs))
        arr = arr * arr
        arg_arr = np.argsort(-arr)
        frac = lambda x: 1.0 / (x + 1)
        for j in range(len(arg_arr)):
            freq_l[arg_arr[j]] += frac(j)
    show_hist(freq_l, ds, method_intermediate_name)

    arg_dim = np.argsort(-freq_l)
    base = base[:, arg_dim]
    query = query[:, arg_dim]
    vecs_io.fvecs_write('data/%s/%s.fvecs' % (ds, method_name), base)
    vecs_io.fvecs_write('data/%s/%s_query.fvecs' % (ds, method_name), query)
    end_time = time.time()
    print(method_name, ds, base.shape, "time consumed", end_time - start_time)
