import os
import filecmp
import numpy as np
from script.data_convert import vecs_io
import run_polyuhost as polyu


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp',
                 'amazon-home-kitchen']
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    dataset_name = "yahoomusic_big"  # yahoomusic_big yelp amazon-home-kitchen
    k_max = polyu.compute_k_max_in_reverse_mips(dataset_name, 32)
    for dataset_name in ["yahoomusic_big", 'yelp', 'amazon-home-kitchen']:
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
            f"--test_topk {'false'} --method_name {'Simpfer'} --simpfer_k_max {k_max} --stop_time {14400} "
        )

    n_sample_item = 5000
    dataset_name = 'yelp'  # yelp, yahoomusic_big
    n_data_item = polyu.dataset_m[dataset_name][0]
    n_user = polyu.dataset_m[dataset_name][2]
    memory_capacity = 8
    method_name = "QueryRankSampleSearchKthRank"
    for sample_topk in [100, 200, 300, 400, 500]:
        os.system(
            "cd build && ./qdibc --index_dir {} --dataset_dir {} --dataset_name {} --n_sample_item {} --sample_topk {}".format(
                index_dir, dataset_dir, dataset_name, n_sample_item, sample_topk
            ))
        n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
        os.system(
            f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {method_name} " +
            f"--n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )
        os.system(
            f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
            f"--method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    sample_topk = 600
    dataset_name = 'yahoomusic_big'  # yelp, yahoomusic_big
    n_data_item = polyu.dataset_m[dataset_name][0]
    n_user = polyu.dataset_m[dataset_name][2]
    memory_capacity = 8
    method_name = "QueryRankSampleSearchKthRank"
    for n_sample_item in [1000, 2000, 4000, 8000, 16000]:
        os.system(
            "cd build && ./qdibc --index_dir {} --dataset_dir {} --dataset_name {} --n_sample_item {} --sample_topk {}".format(
                index_dir, dataset_dir, dataset_name, n_sample_item, sample_topk
            ))
        n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
        os.system(
            f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {method_name} " +
            f"--n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )
        os.system(
            f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
            f"--method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")


if __name__ == '__main__':
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
