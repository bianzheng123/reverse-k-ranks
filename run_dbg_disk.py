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

    '''rmips'''
    # dataset_name = "yahoomusic_big"  # movielens-27m yahoomusic_big yelp amazon-home-kitchen
    # for memory_capacity in [32]:
    #     for dataset_name in ['yahoomusic_big', 'yelp']:
    #         k_max = polyu.compute_k_max_in_reverse_mips(dataset_name, memory_capacity)
    #         os.system(
    #             f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
    #             f"--test_topk {'false'} --method_name {'SimpferOnly'} --simpfer_k_max {k_max} --stop_time {70000} "
    #         )

    '''sample_heuristic'''
    sample_topk = 600
    memory_capacity = 8
    n_sample_item = 5000
    dataset_name = 'yahoomusic_big'
    method_name = "QueryRankSampleUniformIntLR"
    n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    os.system(
        f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
        f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )
    method_name = 'QueryRankSampleSearchUniformRankUniformIntLR'
    os.system(
        f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
        f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )

    dataset_name = 'yelp'
    method_name = "QueryRankSampleMinMaxIntLR"
    n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    os.system(
        f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
        f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )
    method_name = 'QueryRankSampleSearchUniformRankMinMaxIntLR'
    os.system(
        f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
        f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )

    '''n_sample_item'''
    # sample_topk = 600
    # memory_capacity = 8
    # for n_sample_item in [1000, 2000, 3000, 4000, 5000]:
    #     dataset_name = 'yahoomusic_big'
    #     method_name = "QueryRankSampleUniformIntLR"
    #     n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
    #         f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #     )
    #
    #     dataset_name = 'yelp'
    #     method_name = "QueryRankSampleMinMaxIntLR"
    #     n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
    #         f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #     )

    '''sample_topk'''
    # n_sample_item = 5000
    # memory_capacity = 8
    # for sample_topk in [100, 200, 300, 400, 500]:
    #     dataset_name = 'yahoomusic_big'
    #     method_name = "QueryRankSampleUniformIntLREstimate"
    #     n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
    #         f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #     )
    #
    #     dataset_name = 'yelp'
    #     method_name = "QueryRankSampleMinMaxIntLREstimate"
    #     n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
    #         f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #     )


if __name__ == '__main__':
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    index_amazon_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index_amazon')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
