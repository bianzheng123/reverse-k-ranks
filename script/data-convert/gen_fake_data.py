import numpy as np
import vecs_io
import multiprocessing
import time
import os


def delete_file_if_exist(dire):
    if os.path.exists(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


if __name__ == '__main__':
    # reverse k ranks是给定item, 需要输出user
    n_query_item = 100
    n_dim = 30
    n_data_item = 1000
    n_user = 1000

    query_item_l = np.random.normal(scale=10, size=(n_query_item, n_dim))
    data_item_l = np.random.normal(scale=10, size=(n_data_item, n_dim))
    user_l = np.random.normal(scale=10, size=(n_user, n_dim))

    dataset = 'fake'
    output_dir = '/home/bianzheng/Dataset/ReverseMIPS/%s' % dataset
    delete_file_if_exist(output_dir)
    os.mkdir(output_dir)

    vecs_io.dvecs_write("%s/%s_query_item.dvecs" % (output_dir, dataset), query_item_l)
    vecs_io.dvecs_write("%s/%s_data_item.dvecs" % (output_dir, dataset), data_item_l)
    vecs_io.dvecs_write("%s/%s_user.dvecs" % (output_dir, dataset), user_l)
