import vecs_io
import numpy as np
import matplotlib.pyplot as plt
import os
import measure_n_dims
import sys
import time

# 对于原始数据进行维度排序, 到达80%norm需要经过的维度数量
# 排序方式是得到这个数据集上不同item每一个维度的norm, 并从大到小排序

if __name__ == '__main__':
    dataset_type = sys.argv[1]
    ds = sys.argv[2]
    method_name = '%s_desc_total_norm' % dataset_type

    start_time = time.time()
    base, dim = vecs_io.fvecs_read('data/%s/%s.fvecs' % (ds, dataset_type))
    query, dim = vecs_io.fvecs_read('data/%s/%s_query.fvecs' % (ds, dataset_type))
    norm_dim = np.linalg.norm(base, axis=0)
    arg_dim = np.argsort(-norm_dim)
    base = base[:, arg_dim]
    query = query[:, arg_dim]
    vecs_io.fvecs_write('data/%s/%s.fvecs' % (ds, method_name), base)
    vecs_io.fvecs_write('data/%s/%s_query.fvecs' % (ds, method_name), query)
    end_time = time.time()
    print(method_name, ds, base.shape, "time consumed", end_time - start_time)
