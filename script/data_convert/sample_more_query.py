import vecs_io
import os
import numpy as np


def delete_file_if_exist(dire):
    if os.path.exists(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


if __name__ == '__main__':
    ds_m = {'yahoomusic_big': 'yahoomusic_big_more_query', 'yelp': 'yelp_more_query'}
    # basic_dir = '/home/bianzheng/Dataset/ReverseMIPS'
    basic_dir = '/home/zhengbian/Dataset/ReverseMIPS'
    # basic_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    # n_user = 1000
    # n_item = 5000
    n_query = 10000
    for from_ds in ds_m.keys():
        to_ds = ds_m[from_ds]
        data_item, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_data_item.dvecs' % from_ds))
        print("len ", data_item.shape)
        print(data_item.shape)

        query_item, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_query_item.dvecs' % from_ds))
        print("len ", query_item.shape)
        query_item_idx = np.random.permutation(len(data_item))[:n_query]
        query_item = data_item[query_item_idx]
        print(query_item.shape)
        # query_item = query_item[[0]]

        user, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_user.dvecs' % from_ds))
        print("len ", user.shape)
        print(user.shape)

        delete_file_if_exist(os.path.join(basic_dir, to_ds))
        os.mkdir(os.path.join(basic_dir, to_ds))
        vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_data_item.dvecs' % to_ds), data_item)
        vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_query_item.dvecs' % to_ds), query_item)
        vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_user.dvecs' % to_ds), user)
