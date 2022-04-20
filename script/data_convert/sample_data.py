import vecs_io
import os
import numpy as np


def delete_file_if_exist(dire):
    if os.path.exists(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


if __name__ == '__main__':
    ds_m = {'yahoomusic': 'yahoomusic-small', 'yelp': 'yelp-small'}
    # basic_dir = '/home/bianzheng/Dataset/ReverseMIPS'
    basic_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    n_user = 500000
    n_item = 50000
    for from_ds in ds_m.keys():
        to_ds = ds_m[from_ds]
        data_item, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_data_item.dvecs' % from_ds))
        print("len ", data_item.shape)
        data_item_idx = np.random.permutation(len(data_item))[:n_item]
        data_item = data_item[data_item_idx]
        print(data_item.shape)

        query_item, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_query_item.dvecs' % from_ds))
        # query_item = query_item[[0]]

        user, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_user.dvecs' % from_ds))
        print("len ", data_item.shape)
        user_idx = np.random.permutation(len(user))[:n_user]
        user = user[user_idx]
        print(user.shape)

        delete_file_if_exist(os.path.join(basic_dir, to_ds))
        os.mkdir(os.path.join(basic_dir, to_ds))
        vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_data_item.dvecs' % to_ds), data_item)
        vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_query_item.dvecs' % to_ds), query_item)
        vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_user.dvecs' % to_ds), user)
