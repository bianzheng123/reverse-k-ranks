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
dimension = 150


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


def compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity):
    n_user = dataset_m[dataset_name][2]
    n_sample = memory_capacity * 1024 * 1024 * 1024 / element_size / n_user
    return int(n_sample)


def compute_n_sample_by_memory_index_score_distribution(dataset_name, memory_capacity, n_bit):
    n_user = dataset_m[dataset_name][2]
    sizeof_char = 1
    sizeof_double = 8
    n_sample = 1.0 * (memory_capacity * 1024 * 1024 * 1024 + n_user * n_bit / 8) / (
            sizeof_double + n_bit / 8) / n_user
    return int(n_sample)


def compute_n_sample_by_memory_index_intlr(dataset_name, memory_capacity):
    n_user = dataset_m[dataset_name][2]
    n_data_item = dataset_m[dataset_name][0]
    sizeof_char = 1
    sizeof_double = 8
    sizeof_int = 4
    n_sample = 1.0 * (
            memory_capacity * 1024 * 1024 * 1024 - n_user * 4 * sizeof_double - n_user * dimension * sizeof_int - n_data_item * dimension * sizeof_int
    ) / sizeof_double / n_user
    return int(n_sample)


def run_sample_method(method_name, dataset_name, n_sample, n_data_item, n_user, n_sample_item, sample_topk,
                      other_config=""):
    os.system(
        f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {method_name} --n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )
    os.system(
        f"cd build && ./bsibs --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
    # os.system(
    #     f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    if method_name == 'QueryRankSampleLeastSquareIntLR' or method_name == 'QueryRankSampleMinMaxIntLR' or method_name == "QueryRankSampleDirectIntLR":
        os.system(
            f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    os.system(
        f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk} {other_config}"
    )


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'goodreads']
    # dataset_l = ['movielens-27m', 'yahoomusic_big', 'yelp', 'goodreads']
    dataset_l = ['movielens-27m']
    # dataset_l = ['yahoomusic_big', 'yelp', 'goodreads']

    # for ds in dataset_l:
    #     os.system('cd build/attribution && ./ibc --dataset_name {} --dataset_dir {}'.format(ds, dataset_dir))

    dataset_l = ['movielens-27m', 'netflix']
    for ds in dataset_l:
        # os.system('cd build && ./bst --dataset_name {}'.format(ds))

        # os.system('cd build && ./rri --dataset_name {} --test_topk {} --method_name {}'.format(ds, 'true', 'GridIndex'))
        # os.system('cd build && ./rri --dataset_name {} --test_topk {} --method_name {}'.format(ds, 'true', 'LinearModel'))

        n_sample_item = 5000
        sample_topk = 600
        memory_capacity = 1
        n_data_item = dataset_m[ds][0]
        n_user = dataset_m[ds][2]
        os.system(
            "cd build && ./qdi --index_dir {} --dataset_dir {} --dataset_name {} --n_sample_item {} --sample_topk {}".format(
                index_dir, dataset_dir, ds, n_sample_item, sample_topk
            ))

        # n_sample_intlr = compute_n_sample_by_memory_index_intlr(ds, memory_capacity)
        # run_sample_method('QueryRankSampleIntLR',
        #                   ds, n_sample_intlr,
        #                   n_data_item, n_user,
        #                   n_sample_item, sample_topk)
        # run_sample_method('QueryRankSampleLeastSquareIntLR', ds, n_sample_intlr,
        #                   n_data_item, n_user,
        #                   n_sample_item, sample_topk)

        for n_bit in [2, 4, 8, 16, 32, 64]:
            parameter_name = f"--n_bit {n_bit}"
            n_sample_score_distribution = compute_n_sample_by_memory_index_score_distribution(ds, memory_capacity,
                                                                                              n_bit)
            run_sample_method('QueryRankSampleScoreDistribution',
                              ds, n_sample_score_distribution,
                              n_data_item, n_user,
                              n_sample_item, sample_topk, parameter_name)

        # n_sample_sample_only = compute_n_sample_by_memory_index_sample_only(ds, memory_capacity)
        # run_sample_method('QueryRankSampleSearchAllRank',
        #                   ds, n_sample_sample_only,
        #                   n_data_item, n_user,
        #                   n_sample_item, sample_topk)
        # run_sample_method('QueryRankSampleSearchKthRank',
        #                   ds, n_sample_sample_only,
        #                   n_data_item, n_user,
        #                   n_sample_item, sample_topk)
        # run_sample_method('RankSample',
        #                   ds, n_sample_sample_only,
        #                   n_data_item, n_user,
        #                   n_sample_item, sample_topk)


if __name__ == '__main__':
    dataset_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    # basic_dir = os.path.join('/home', 'bianzheng', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'bianzheng', 'CLionProjects', 'reverse-k-ranks', 'index')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
