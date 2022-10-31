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


def compute_n_sample_by_memory_index_qrssd(dataset_name, memory_capacity):
    n_user = dataset_m[dataset_name][2]
    sizeof_char = 1
    sizeof_double = 8
    n_sample = 1.0 * (memory_capacity * 1024 * 1024 * 1024 + n_user * sizeof_char) / (
            sizeof_double + sizeof_char) / n_user
    return int(n_sample)


def compute_n_sample_by_memory_index_qrsintlr(dataset_name, memory_capacity):
    n_user = dataset_m[dataset_name][2]
    sizeof_char = 1
    sizeof_double = 8
    n_sample = 1.0 * (memory_capacity * 1024 * 1024 * 1024 - n_user * 4 * sizeof_double) / sizeof_double / n_user
    return int(n_sample)


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'goodreads']
    # dataset_l = ['movielens-27m', 'yahoomusic_big', 'yelp', 'goodreads']
    dataset_l = ['movielens-27m']
    # dataset_l = ['yahoomusic_big', 'yelp', 'goodreads']

    # for ds in dataset_l:
    #     os.system('cd build/attribution && ./ibc --dataset_name {} --dataset_dir {}'.format(ds, dataset_dir))

    dataset_l = ['movielens-27m', 'netflix']
    for ds in dataset_l:
        # os.system('cd build && ./bst --dataset_dir {} --dataset_name {} --index_dir {}'.format(
        #     dataset_dir, ds, index_dir))
        memory_capacity = 2
        n_sample = compute_n_sample_by_memory_index(ds, memory_capacity)
        # os.system(
        #     'cd build && ./rri --dataset_dir {} --dataset_name {} --test_topk {} --index_dir {} --method_name {}'.format(
        #         dataset_dir, ds, 'false', index_dir, 'LinearModel'))
        os.system(
            'cd build && ./rri --dataset_dir {} --dataset_name {} --test_topk {} --index_dir {} --method_name {} --n_sample {}'.format(
                dataset_dir, ds, 'false', index_dir, 'RankSample', n_sample))
        n_sample_query = 5000
        sample_topk = 600
        n_data_item = dataset_m[ds][0]
        n_user = dataset_m[ds][2]
        # os.system(
        #     'cd build && ./qdi --dataset_dir {} --dataset_name {} --index_dir {} --n_sample_item {} --sample_topk {}'.format(
        #         dataset_dir, ds, index_dir, n_sample_query, sample_topk
        #     ))
        # os.system(
        #     "cd build && ./fsr --dataset_name {} --index_dir {} --n_sample {} --n_sample_query {} --sample_topk {} --n_data_item {} --n_user {}".format(
        #         ds, index_dir, n_sample, n_sample_query, sample_topk, n_data_item, n_user
        #     ))
        # os.system(
        #     "cd build && ./bqrsi --dataset_dir {} --dataset_name {} --index_dir {} --n_sample {} --n_sample_query {} --sample_topk {}".format(
        #         dataset_dir, ds, index_dir, n_sample, n_sample_query, sample_topk
        #     ))
        # os.system(
        #     'cd build && ./rri --dataset_dir {} --dataset_name {} --test_topk {} --index_dir {} --method_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
        #         dataset_dir, ds, 'false', index_dir, 'QueryRankSampleSearchKthRank', n_sample, n_sample_query, sample_topk))

        n_sample_qrsintlr = compute_n_sample_by_memory_index_qrsintlr(ds, memory_capacity)
        os.system(
            "cd build && ./fsr --dataset_name {} --index_dir {} --n_sample {} --n_sample_query {} --sample_topk {} --n_data_item {} --n_user {}".format(
                ds, index_dir, n_sample_qrsintlr, n_sample_query, sample_topk, n_data_item, n_user
            ))
        os.system(
            "cd build && ./bqrsi --dataset_dir {} --dataset_name {} --index_dir {} --n_sample {} --n_sample_query {} --sample_topk {}".format(
                dataset_dir, ds, index_dir, n_sample_qrsintlr, n_sample_query, sample_topk
            ))
        os.system(
            'cd build && ./rri --dataset_dir {} --dataset_name {} --test_topk {} --index_dir {} --method_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
                dataset_dir, ds, 'false', index_dir, 'QueryRankSampleIntLR',
                n_sample_qrsintlr, n_sample_query, sample_topk))

        n_sample_qrssd = compute_n_sample_by_memory_index_qrssd(ds, memory_capacity)
        os.system(
            "cd build && ./fsr --dataset_name {} --index_dir {} --n_sample {} --n_sample_query {} --sample_topk {} --n_data_item {} --n_user {}".format(
                ds, index_dir, n_sample_qrssd, n_sample_query, sample_topk, n_data_item, n_user
            ))
        os.system(
            "cd build && ./bqrsi --dataset_dir {} --dataset_name {} --index_dir {} --n_sample {} --n_sample_query {} --sample_topk {}".format(
                dataset_dir, ds, index_dir, n_sample_qrssd, n_sample_query, sample_topk
            ))
        os.system(
            'cd build && ./rri --dataset_dir {} --dataset_name {} --test_topk {} --index_dir {} --method_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
                dataset_dir, ds, 'true', index_dir, 'QueryRankSampleScoreDistribution',
                n_sample_qrssd, n_sample_query, sample_topk))


if __name__ == '__main__':
    dataset_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    # basic_dir = os.path.join('/home', 'bianzheng', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'bianzheng', 'CLionProjects', 'reverse-k-ranks', 'index')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
