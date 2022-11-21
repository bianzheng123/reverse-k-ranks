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

    n_sample_item = 5000
    sample_topk = 600
    memory_capacity_l = np.array([20, 40, 60, 80, 100]) / 1024
    dataset_name = 'movielens-27m'  # yelp, yahoomusic_big
    for memory_capacity in memory_capacity_l:
        n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
            f"--test_topk {'false'} --method_name {'QueryRankSampleSearchKthRank'} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
            f"--test_topk {'false'} --method_name {'QueryRankSampleSearchAllRank'} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )

    dataset_l = ['amazon-home-kitchen']
    for ds in dataset_l:
        memory_capacity = 16
        k_max = polyu.compute_k_max_in_reverse_mips(ds, memory_capacity)
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_amazon_dir} " +
            f"--test_topk {'false'} --method_name {'Simpfer'} --simpfer_k_max {k_max} --stop_time {5000}"
        )

    dataset_name = 'yahoomusic_big'
    n_sample_item = 5000
    for sample_topk in [10, 20, 50, 100, 200]:
        memory_capacity = 8
        n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
            f"--test_topk {'false'} --method_name {'QueryRankSampleSearchKthRank'} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )


if __name__ == '__main__':
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    index_amazon_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index_amazon')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
