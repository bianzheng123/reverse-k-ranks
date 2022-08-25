import os
import create_test_dataset

if __name__ == '__main__':
    ds_m = {'fake-normal': 'fake-normal-query-distribution', 'fake-uniform': 'fake-uniform-query-distribution',
            'fakebig': 'fakebig-query-distribution', 'netflix-small': 'netflix-small-query-distribution'}
    basic_dir = '/home/bianzheng/Dataset/ReverseMIPS'
    # basic_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    n_sample_item = 150
    sample_topk = 10

    for ds in ds_m.keys():
        os.system(
            'cd /home/bianzheng/reverse-k-ranks/build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {}'.format(
                ds, n_sample_item, sample_topk))

    for from_ds in ds_m.keys():
        create_test_dataset.change_index(from_ds, ds_m[from_ds], basic_dir, n_sample_item, sample_topk)

    for ds in ds_m.values():
        os.system(
            'cd /home/bianzheng/reverse-k-ranks/build && ./rri --dataset_name {} --method_name {} --n_sample {}'.format(
                ds, 'RankSample', 20))
        os.system(
            'cd /home/bianzheng/reverse-k-ranks/build && ./rri --method_name {} --dataset_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
                'QueryRankSample',
                ds, 20, n_sample_item, sample_topk))
