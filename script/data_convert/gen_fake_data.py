import numpy as np
import vecs_io
import os


def delete_file_if_exist(dire):
    if os.path.exists(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


def gen_data(n_user, n_data_item, n_query_item, vec_dim, dataset):
    query_item_l = np.random.normal(scale=0.1, size=(n_query_item, vec_dim))
    data_item_l = np.random.normal(scale=0.1, size=(n_data_item, vec_dim))
    user_l = np.random.normal(scale=0.1, size=(n_user, vec_dim))

    output_dir = '/home/bianzheng/Dataset/ReverseMIPS/%s' % dataset
    delete_file_if_exist(output_dir)
    os.mkdir(output_dir)

    vecs_io.dvecs_write("%s/%s_query_item.dvecs" % (output_dir, dataset), query_item_l)
    vecs_io.dvecs_write("%s/%s_data_item.dvecs" % (output_dir, dataset), data_item_l)
    vecs_io.dvecs_write("%s/%s_user.dvecs" % (output_dir, dataset), user_l)


if __name__ == '__main__':
    # reverse k ranks是给定item, 需要输出user
    n_query_item = 100
    n_dim = 30
    n_data_item = 2000
    n_user = 500

    gen_data(n_user, n_data_item, n_query_item, n_dim, "fake")
