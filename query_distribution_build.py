import os

if __name__ == '__main__':
    old_dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    for ds in old_dataset_l:
        os.system('cd build/attribution && ./srtku --dataset_name {}'.format(ds))
        os.system('cd build/attribution && ./dbt --dataset_name {}'.format(ds))
    os.system('cd attribution/query-distribution-index && python3 create_test_dataset.py')

    dataset_l = ['fake-normal-query-distribution', 'fake-uniform-query-distribution',
                 'fakebig-query-distribution', 'netflix-small-query-distribution']
    for ds in dataset_l:
        os.system('cd build && ./rri --dataset_name {} --method_name {} --n_sample {}'.format(ds, 'RankSample', 5))
        os.system(
            'cd build && ./progress --method_name {} --dataset_name {} --n_sample {}'.format('QueryRankSample', ds, 5))
