import numpy as np
import os
import vecs_io


def delete_file_if_exist(dire):
    if os.path.exists(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


def get_sample_queryID_l(file_name, n_query, n_last_query):
    itemID_l = np.loadtxt(
        '/home/zhengbian/reverse-k-ranks/index/query_distribution/{}/sample_itemID_l.txt'.format(file_name))
    n_sample_item = len(itemID_l)
    last_itemID_l = itemID_l[-n_last_query:]
    n_remain_query = n_query - n_last_query
    remain_itemID_l = itemID_l[np.random.permutation(n_sample_item - n_last_query)[:n_remain_query]]

    sample_itemID_l = np.append(remain_itemID_l, last_itemID_l)
    sample_itemID_l = np.array(sample_itemID_l, dtype=np.int32)
    return sample_itemID_l


if __name__ == '__main__':
    ds_m = {'yahoomusic_big': 'yahoomusic_big_more_query', 'yelp': 'yelp_more_query'}
    # basic_dir = '/home/bianzheng/Dataset/ReverseMIPS'
    basic_dir = '/home/zhengbian/Dataset/ReverseMIPS'
    # basic_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    # n_user = 1000
    # n_item = 5000
    n_query = 1000
    n_last_query = 300

    for from_ds in ds_m.keys():
        to_ds = ds_m[from_ds]
        data_item, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_data_item.dvecs' % from_ds))
        print("len ", data_item.shape)
        print(data_item.shape)

        queryID_l = get_sample_queryID_l(
            '{}-n_sample_item_5000-sample_topk_600'.format(from_ds), n_query, n_last_query)
        query_item = data_item[queryID_l]
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
