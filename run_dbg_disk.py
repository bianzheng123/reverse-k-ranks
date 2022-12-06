import os
import filecmp
import numpy as np
from script.data_convert import vecs_io
import run_polyuhost as polyu


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp']
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    sample_topk = 600
    memory_capacity = 8
    method_name = "QueryRankSampleSearchUniformRankMinMaxIntLR"
    n_sample_item = 5000
    dataset_name = "yahoomusic_big"
    for dataset_name in ['yahoomusic_big']:
        for method_name in ["QueryRankSampleSearchUniformRankMinMaxIntLR", 'QueryRankSampleSearchUniformRank']:
            n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
            os.system(
                f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
                f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
            )

    dataset_name = "yahoomusic_big"  # movielens-27m yahoomusic_big yelp amazon-home-kitchen
    for dataset_name in ['yahoomusic_big', 'yelp']:
        for memory_capacity in [2, 4, 8, 16, 32]:
            k_max = polyu.compute_k_max_in_reverse_mips(dataset_name, memory_capacity)
            os.system(
                f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
                f"--test_topk {'false'} --method_name {'SimpferOnly'} --simpfer_k_max {k_max} --stop_time {70000} "
            )

    # sample_topk = 600
    # memory_capacity = 8
    # method_name = "QueryRankSampleUniformIntLR"
    # n_sample_item = 5000
    # for dataset_name in ['movielens-27m', 'yahoomusic_big', 'yelp']:
    #     n_sample = polyu.compute_n_sample_by_memory_index_intlr(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
    #         f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #     )
    #
    # for dataset_name in ['yahoomusic_big', 'yelp']:
    #     for memory_capacity in [2, 4, 16, 32]:
    #         n_sample = polyu.compute_n_sample_by_memory_index_intlr(dataset_name, memory_capacity)
    #         os.system(
    #             f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
    #             f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #         )
    #
    # for dataset_name in ['amazon-home-kitchen']:
    #     n_sample = polyu.compute_n_sample_by_memory_index_intlr(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_amazon_dir} " +
    #         f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #     )

    # sample_topk = 600
    # memory_capacity = 8
    # method_name = "QueryRankSampleSearchKthRank"
    # for dataset_name in ['yahoomusic_big', 'yelp']:
    #     for n_sample_item in [1000, 2000, 3000, 4000, 5000]:
    #         n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #         os.system(
    #             f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
    #             f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #         )


if __name__ == '__main__':
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    index_amazon_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index_amazon')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
