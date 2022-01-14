import vecs_io
import numpy as np
import matplotlib.pyplot as plt
import os
import measure_n_dims
import sys
import time

# 原始数据, 对每一个向量来说到达80%norm需要经过多少维度
if __name__ == '__main__':
    dataset_type = sys.argv[1]
    ds = sys.argv[2]
    method_name = '%s_free_dim' % dataset_type

    start_time = time.time()
    base, dim = vecs_io.fvecs_read('data/%s/%s.fvecs' % (ds, dataset_type))
    norm_max = 0.8

    norm_l = np.linalg.norm(base, axis=1)
    norm_l = norm_l * norm_l

    freq_l = np.zeros(shape=dim, dtype=np.int32)
    for i, vecs in enumerate(base, 0):
        arr = sorted(abs(vecs), reverse=True)
        arr = np.array(arr)
        arr = arr * arr
        cum_sum = 0
        for j in range(len(arr)):
            cum_sum += arr[j]
            if cum_sum > norm_l[i] * norm_max:
                freq_l[j] += 1
                break
    measure_n_dims.show_hist(freq_l, ds, method_name, len(base))
    end_time = time.time()
    print(method_name, ds, base.shape, "time consumed", end_time - start_time)
