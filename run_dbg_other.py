import os
import filecmp
import numpy as np
from script.data_convert import vecs_io
import run_polyuhost as polyu


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
             'amazon-home-kitchen': [409243, 1000, 2511610],
             'yahoomusic_big_more_query': [135736, 1000, 1823179],
             'yelp_more_query': [159585, 1000, 2189457], }
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


def compute_k_max_in_reverse_mips(dataset_name, memory_capacity):
    n_user = dataset_m[dataset_name][2]
    n_data_item = dataset_m[dataset_name][0]
    sizeof_double = 8
    k_max = 1.0 * (
            memory_capacity * 1024 * 1024 * 1024) / (n_user * 2 * sizeof_double)
    k_max = min(n_data_item, k_max)
    return int(k_max)


def run_sample_method(method_name, dataset_name, n_sample, n_data_item, n_user, n_sample_item, sample_topk,
                      other_config=""):
    os.system(
        f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {method_name} --n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )
    os.system(
        f"cd build && ./bsibs --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
    # os.system(
    #     f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    if method_name == 'QueryRankSampleLeastSquareIntLR' or method_name == 'QueryRankSampleMinMaxIntLR' or \
            method_name == "QueryRankSampleDirectIntLR" or method_name == "QueryRankSampleGlobalIntLR":
        os.system(
            f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    os.system(
        f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk} {other_config}"
    )


def test_build_score_table(dataset_name: str, eval_size_gb: int = 100):
    username = 'bianzheng'
    dataset_dir = f'/home/{username}/Dataset/ReverseMIPS'
    index_dir = f'/home/{username}/reverse-k-ranks/index'
    os.system(
        f'cd build && ./bstb --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --eval_size_gb {eval_size_gb} ')
    os.system(f'cd index && rm {dataset_name}.index')


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp',
                 'amazon-home-kitchen']
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    dataset_name = 'amazon-home-kitchen'
    k_max = compute_k_max_in_reverse_mips(dataset_name, 8)
    os.system(
        f'cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} '
        f'--test_topk {"false"} --method_name {"Simpfer"} --simpfer_k_max {k_max} --stop_time {43200}')


if __name__ == '__main__':
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    # run()
    test_build_score_table(dataset_name='movielens-27m', eval_size_gb=500)
    test_build_score_table(dataset_name='netflix', eval_size_gb=500)
    test_build_score_table(dataset_name='yahoomusic_big', eval_size_gb=500)
    test_build_score_table(dataset_name='yelp', eval_size_gb=500)
    test_build_score_table(dataset_name='amazon-home-kitchen', eval_size_gb=500)
