import os
import filecmp
import numpy as np
from script.data_convert import vecs_io
import run_polyuhost as polyu


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp',
                 'amazon-home-kitchen']  # TODO specify the dataset name
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    # os.system(
    #     f"cd build/attribution && ./rmips --dataset_dir {dataset_dir} --dataset_name {'movielens-27m'} --simpfer_k_max {1000}"
    # )

    n_sample_item = 5000
    sample_topk = 600

    dataset_name = 'movielens-27m'
    n_data_item = polyu.dataset_m[dataset_name][0]
    n_user = polyu.dataset_m[dataset_name][2]
    method_name = 'QueryRankSampleDirectIntLR'  # QueryRankSampleSearchAllRank QueryRankSampleSearchKthRank

    # memory capacity input in unit of MB
    for dataset_name in ['movielens-27m', 'yahoomusic_big', 'yelp']:
        for memory_capacity in [2, 4, 8, 16, 32]:
            n_sample = polyu.compute_n_sample_by_memory_index_intlr(dataset_name, memory_capacity)
            os.system(
                f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
                f"--method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    # for memory_capacity in memory_capacity_l:  # TODO specify the memory capacity
    #     n_sample = polyu.compute_n_sample_by_memory_index_intlr(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")


if __name__ == '__main__':
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
