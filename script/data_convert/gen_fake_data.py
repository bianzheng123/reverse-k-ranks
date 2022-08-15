import numpy as np
import vecs_io
import os


def delete_file_if_exist(dire):
    if os.path.exists(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


def gen_data_normal(n_user, n_data_item, n_query_item, vec_dim, dataset):
    query_item_l = np.random.normal(loc=0, scale=1, size=(n_query_item, vec_dim))
    data_item_l = np.random.normal(loc=0, scale=1, size=(n_data_item, vec_dim))
    user_l = np.random.normal(loc=0, scale=1, size=(n_user, vec_dim))

    output_dir = '/home/bianzheng/Dataset/ReverseMIPS/%s' % dataset
    delete_file_if_exist(output_dir)
    os.mkdir(output_dir)

    vecs_io.dvecs_write("%s/%s_query_item.dvecs" % (output_dir, dataset), query_item_l)
    vecs_io.dvecs_write("%s/%s_data_item.dvecs" % (output_dir, dataset), data_item_l)
    vecs_io.dvecs_write("%s/%s_user.dvecs" % (output_dir, dataset), user_l)


def gen_data_uniform(n_user, n_data_item, n_query_item, vec_dim, dataset):
    query_item_l = np.random.uniform(0, 1000, size=(n_query_item, vec_dim))
    data_item_l = np.random.uniform(0, 1000, size=(n_data_item, vec_dim))
    user_l = np.random.uniform(0, 1000, size=(n_user, vec_dim))

    output_dir = '/home/bianzheng/Dataset/ReverseMIPS/%s' % dataset
    delete_file_if_exist(output_dir)
    os.mkdir(output_dir)

    vecs_io.dvecs_write("%s/%s_query_item.dvecs" % (output_dir, dataset), query_item_l)
    vecs_io.dvecs_write("%s/%s_data_item.dvecs" % (output_dir, dataset), data_item_l)
    vecs_io.dvecs_write("%s/%s_user.dvecs" % (output_dir, dataset), user_l)


def gen_data_independent(n_user, n_data_item, n_query_item, vec_dim, dataset):
    query_item_l = np.random.uniform(0, 1000, size=(n_query_item, vec_dim))
    data_item_l = np.random.uniform(0, 1000, size=(n_data_item, vec_dim))
    user_l = np.random.uniform(0, 1000, size=(n_user, vec_dim))

    output_dir = '/home/bianzheng/Dataset/ReverseMIPS/%s' % dataset
    delete_file_if_exist(output_dir)
    os.mkdir(output_dir)

    vecs_io.dvecs_write("%s/%s_query_item.dvecs" % (output_dir, dataset), query_item_l)
    vecs_io.dvecs_write("%s/%s_data_item.dvecs" % (output_dir, dataset), data_item_l)
    vecs_io.dvecs_write("%s/%s_user.dvecs" % (output_dir, dataset), user_l)


if __name__ == '__main__':
    # reverse k ranks是给定item, 需要输出user
    n_query_item = 30
    n_dim = 30
    n_data_item = 5000
    n_user = 1000

    gen_data_normal(n_user, n_data_item, n_query_item, n_dim, "fake-normal-query-distribution")
    gen_data_uniform(n_user, n_data_item, n_query_item, n_dim, "fake-uniform-query-distribution")
    # gen_data_uniform(n_user, n_data_item, n_query_item, n_dim, "fake-uniform")
    # gen_data_uniform(100, 500, 10, 10, "fake-small")
    # gen_data_independent(n_user, n_data_item, n_query_item, n_dim, "fake-independent")
    # gen_data_independent(100, 50, 10, n_dim, "fake-tiny")
