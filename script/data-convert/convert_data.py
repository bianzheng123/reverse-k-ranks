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
    rank_k = 10
    dimension = 150
    n_query_item = 1000

    # ds_l = ['movielens-small', 'movielens-27m', 'netflix', 'yelp']
    ds_l = ['movielens-27m']
    for dataset in ds_l:
        input_dir = '/home/bianzheng/Dataset/MIPS'

        item_dir = os.path.join(input_dir, '%s-%dd' % (dataset, dimension), '%s_item.dvecs' % dataset)
        user_dir = os.path.join(input_dir, '%s-%dd' % (dataset, dimension), '%s_user.dvecs' % dataset)

        item_l, d = vecs_io.dvecs_read(item_dir)
        user_l, d = vecs_io.dvecs_read(user_dir)

        output_dir = '/home/bianzheng/Dataset/ReverseMIPS/%s' % dataset
        delete_file_if_exist(output_dir)
        os.mkdir(output_dir)

        n_item = len(item_l)
        item_idx_l = np.random.permutation(n_item)
        query_idx_l = np.sort(item_idx_l[:n_query_item])
        data_idx_l = np.sort(item_idx_l[n_query_item:])

        np.savetxt('%s/query_item.txt' % output_dir, query_idx_l, fmt="%d")
        np.savetxt('%s/data_item.txt' % output_dir, data_idx_l, fmt="%d")

        query_item_l = item_l[query_idx_l, :]
        data_item_l = item_l[data_idx_l, :]

        vecs_io.dvecs_write("%s/%s_query_item.dvecs" % (output_dir, dataset), query_item_l)
        vecs_io.dvecs_write("%s/%s_data_item.dvecs" % (output_dir, dataset), data_item_l)
        vecs_io.dvecs_write("%s/%s_user.dvecs" % (output_dir, dataset), user_l)
        print("write %s complete" % dataset)
