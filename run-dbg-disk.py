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
             'goodreads': [2359650, 1000, 876145],
             'amazon-home-kitchen': [409243, 1000, 2511610]}
element_size = 8


def compute_n_sample_by_memory_index(dataset_name, memory_capacity):
    n_user = dataset_m[dataset_name][2]
    n_sample = memory_capacity * 1024 * 1024 * 1024 / element_size / n_user
    return int(n_sample)


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp']
    dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    for ds in dataset_l:
        # os.system('cd build && ./bst --dataset_dir {} --dataset_name {} --index_dir {}'.format(
        #     dataset_dir, ds, index_dir))
        os.system(
            'cd build && ./qdi --dataset_dir {} --dataset_name {} --index_dir {} --n_sample_item {} --sample_topk {}'.format(
                dataset_dir, ds, index_amazon_dir, 5000, 600
            ))
        # TODO
        # os.system('cd build && ./brsi --dataset_dir {} --dataset_name {} --index_dir {}'.format(
        #     dataset_dir, ds, index_amazon_dir))
        # n_sample = compute_n_sample_by_memory_index(ds, 64)
        # os.system('cd build && ./brsi --dataset_dir {} --dataset_name {} --index_dir {} --n_sample {}'.format(
        #     dataset_dir, ds, index_amazon_dir, n_sample))

        # os.system(
        #     'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
        #         dataset_dir, ds, index_dir, 'QueryRankSampleSearchKthRank', n_sample, 5000, 600))
        # os.system(
        #     'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --simpfer_k_max {}'.format(
        #         dataset_dir, ds, index_dir, "Simpfer", 35))

    # dataset_l = ['yahoomusic_big', 'yelp']
    # for ds in dataset_l:
    #     for memory_capacity in [16]:
    #         n_sample = compute_n_sample_by_memory_index(ds, memory_capacity)
    #         os.system(
    #             'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --n_sample {}'.format(
    #                 dataset_dir, ds, index_dir, 'RankSample', n_sample))


if __name__ == '__main__':
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    index_amazon_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index_amazon')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
