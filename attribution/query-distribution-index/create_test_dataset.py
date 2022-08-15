import vecs_io
import os
import numpy as np


def delete_file_if_exist(dire):
    if os.path.exists(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


if __name__ == '__main__':
    ds_m = {'fake-normal': 'fake-normal-query-distribution', 'fake-uniform': 'fake-uniform-query-distribution',
            'fakebig': 'fakebig-query-distribution', 'netflix-small': 'netflix-small-query-distribution'}
    basic_dir = '/home/bianzheng/Dataset/ReverseMIPS'
    # basic_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    n_sample_query = 500
    topk = 10
    for from_ds in ds_m.keys():
        to_ds = ds_m[from_ds]
        data_item, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_data_item.dvecs' % from_ds))
        print("len data_item ", data_item.shape)

        query_idx_l = np.loadtxt(os.path.join('/home/bianzheng/reverse-k-ranks/index/query_distribution',
                                              '{}-sample-itemID-n_sample_query_{}-topk_{}.txt'.format(from_ds,
                                                                                                      n_sample_query,
                                                                                                      topk)))
        query_idx_l = np.array(query_idx_l, dtype=np.int32)
        query_item = data_item[query_idx_l]
        print("len query_item ", query_item.shape)
        # query_item = query_item[[0]]

        user, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_user.dvecs' % from_ds))
        print("len user", user.shape)

        delete_file_if_exist(os.path.join(basic_dir, to_ds))
        os.mkdir(os.path.join(basic_dir, to_ds))
        vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_data_item.dvecs' % to_ds), data_item)
        vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_query_item.dvecs' % to_ds), query_item)
        vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_user.dvecs' % to_ds), user)

        below_topk_from_ds_path = os.path.join('/home/bianzheng/reverse-k-ranks/index/query_distribution',
                                               '{}-below-topk-n_sample_query_{}-topk_{}.index'.format(from_ds,
                                                                                                    n_sample_query,
                                                                                                    topk))
        below_topk_to_ds_path = os.path.join('/home/bianzheng/reverse-k-ranks/index/query_distribution',
                                             '{}-below-topk-n_sample_query_{}-topk_{}.index'.format(to_ds,
                                                                                                  n_sample_query,
                                                                                                  topk))
        os.system('cp {} {}'.format(below_topk_from_ds_path, below_topk_to_ds_path))

        kth_rank_from_ds_path = os.path.join('/home/bianzheng/reverse-k-ranks/index/query_distribution',
                                             '{}-kth-rank-n_sample_query_{}-topk_{}.index'.format(from_ds,
                                                                                                n_sample_query,
                                                                                                topk))
        kth_rank_to_ds_path = os.path.join('/home/bianzheng/reverse-k-ranks/index/query_distribution',
                                           '{}-kth-rank-n_sample_query_{}-topk_{}.index'.format(to_ds,
                                                                                              n_sample_query,
                                                                                              topk))
        os.system('cp {} {}'.format(kth_rank_from_ds_path, kth_rank_to_ds_path))

        sample_itemID_from_ds_path = os.path.join('/home/bianzheng/reverse-k-ranks/index/query_distribution',
                                                  '{}-sample-itemID-n_sample_query_{}-topk_{}.txt'.format(from_ds,
                                                                                                          n_sample_query,
                                                                                                          topk))
        sample_itemID_to_ds_path = os.path.join('/home/bianzheng/reverse-k-ranks/index/query_distribution',
                                                '{}-sample-itemID-n_sample_query_{}-topk_{}.txt'.format(to_ds,
                                                                                                        n_sample_query,
                                                                                                        topk))
        os.system('cp {} {}'.format(sample_itemID_from_ds_path, sample_itemID_to_ds_path))
