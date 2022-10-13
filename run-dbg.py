import os
import filecmp
import numpy as np
from script.data_convert import vecs_io


class CMDcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


dataset_m = {'movielens-27m': [52889, 1000, 283228],
             'netflix': [16770, 1000, 480189],
             'yahoomusic_big': [135736, 1000, 1823179],
             'yahoomusic': [97213, 1000, 1948882],
             'yelp': [159585, 1000, 2189457],
             'goodreads': [2359650, 1000, 876145]}
element_size = 8


def compute_n_sample_by_block(dataset_name, memory_capacity):
    disk_page = 4096
    n_element_per_block = disk_page / element_size
    n_data_item = dataset_m[dataset_name][0]
    n_user = dataset_m[dataset_name][2]
    require_n_sample = np.ceil(n_data_item / n_element_per_block) + 1
    require_memory_capacity = require_n_sample * n_user * element_size / 1024 / 1024 / 1024

    n_sample = require_n_sample
    if require_memory_capacity > memory_capacity:
        print(
            CMDcolors.WARNING + "Warning: required memory capacity {}GB > the required memory capacity {}GB, change n_sample to 32".format(
                require_memory_capacity, memory_capacity) + CMDcolors.ENDC)
        n_sample = memory_capacity * 1024 * 1024 * 1024 / element_size / n_user
    return int(n_sample)


def compute_n_sample_by_memory_index(dataset_name, memory_capacity):
    n_user = dataset_m[dataset_name][2]
    n_sample = memory_capacity * 1024 * 1024 * 1024 / element_size / n_user
    return int(n_sample)


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    dataset_l = ['yahoomusic_big', 'yelp', 'movielens-27m', 'netflix']
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    for ds in dataset_l:
        # os.system('cd build && ./bst --dataset_dir {} --dataset_name {} --index_dir {}'.format(
        #     dataset_dir, ds, index_dir))
        os.system(
            'cd build && ./dbt --dataset_dir {} --dataset_name {} --index_dir {} --n_sample_item {} --sample_topk {}'.format(
                dataset_dir, ds, index_dir, 5000, 600
            ))
        n_sample = compute_n_sample_by_memory_index(ds, 16)
        n_part = 4
        for i in range(1, n_part + 1, 1):
            tmp_i = i / n_part
            tmp_n_sample = int(n_sample * tmp_i)
            os.system(
                'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
                    dataset_dir, ds, index_dir, 'QueryRankSampleSearchKthRank', tmp_n_sample, 5000, 600))
            os.system(
                'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --n_sample {}'.format(
                    dataset_dir, ds, index_dir, 'RankSample', tmp_n_sample))
        # os.system(
        #     'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --simpfer_k_max {}'.format(
        #         dataset_dir, ds, index_dir, "Simpfer", 35))
        # os.system('cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {}'.format(
        #     ds, 5000, 500,
        # ))

    # for ds in dataset_l:
    #     n_sample = compute_n_sample(ds, 16)
    #     os.system(
    #         'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --simpfer_k_max {}'.format(
    #             dataset_dir, ds, index_dir, "Simpfer", 1025))
    #     os.system(
    #         'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
    #             dataset_dir, ds, index_dir, 'QueryRankSampleSearchAllRank', n_sample, 5000, 600))


if __name__ == '__main__':
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()

    # os.system(
    #     'cd build/attribution && ./qdur --dataset_name {} --basic_dir {} --n_sample_item {}'.format(
    #         'yahoomusic_big', basic_dir, 5000))
    # os.system(
    #     'cd build/attribution && ./bfcs --dataset_name {} --basic_dir {} --n_sample_item {} --n_sample_rank {} --topk {}'.format(
    #         'yahoomusic_big', basic_dir, 5000, 1500, 30))

    # TODO run
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'goodreads', 'amazon']
    # for ds in dataset_l:
    #     os.system('cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {}'.format(ds, 1000, 10))
    #     os.system(
    #         'cd build/attribution && ./ppr --dataset_name {} --n_sample_query {} --sample_topk {}'.format(
    #             ds, 1000, 10))

    # os.system('cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {}'.format('amazon', 5000, 50))
    # os.system('cd build && ./brrstt --dataset_name {}'.format('amazon'))
