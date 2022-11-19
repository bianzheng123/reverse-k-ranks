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
    dataset_name = 'yahoomusic_big'  # yelp, yahoomusic_big
    n_data_item = polyu.dataset_m[dataset_name][0]
    n_user = polyu.dataset_m[dataset_name][2]
    memory_capacity = 8
    method_name = "QueryRankSampleSearchKthRank"
    for n_sample_item in [1000, 2000, 4000]:
        n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
        os.system(
            f"cd build && ./qdi --index_dir {index_dir} --dataset_dir {dataset_dir} --dataset_name {dataset_name} " +
            f"--n_sample_item {n_sample_item} --sample_topk {sample_topk}"
        )

    dataset_l = ['amazon-home-kitchen']
    n_sample_item = 5000
    sample_topk = 600
    for ds in dataset_l:
        memory_capacity = 8
        n_sample = polyu.compute_n_sample_by_memory_index_sample_only(ds, memory_capacity)
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_amazon_dir} " +
            f"--test_topk {'false'} --method_name {'QueryRankSampleSearchKthRank'} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )
        n_sample = polyu.compute_n_sample_by_memory_index_intlr(ds, memory_capacity)
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_amazon_dir} " +
            f"--test_topk {'false'} --method_name {'QueryRankSampleMinMaxIntLR'} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_amazon_dir} " +
            f"--test_topk {'false'} --method_name {'QueryRankSampleDirectIntLR'} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )

    dataset_l = ['movielens-27m', 'yahoomusic_big', 'yelp']
    n_sample_item = 5000
    sample_topk = 600
    for ds in dataset_l:
        memory_capacity = 8
        n_sample = polyu.compute_n_sample_by_memory_index_intlr(ds, memory_capacity)
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_dir} " +
            f"--test_topk {'false'} --method_name {'QueryRankSampleDirectIntLR'} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )


if __name__ == '__main__':
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    index_amazon_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index_amazon')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
