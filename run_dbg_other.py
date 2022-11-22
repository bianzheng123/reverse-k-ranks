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
    os.system(
        f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
        f"--test_topk {'false'} --method_name {'Simpfer'} --simpfer_k_max {k_max} --stop_time {86400} "
    )

    # sample_topk = 600
    # dataset_name = 'yahoomusic_big'  # yelp, yahoomusic_big
    # n_data_item = polyu.dataset_m[dataset_name][0]
    # n_user = polyu.dataset_m[dataset_name][2]
    # memory_capacity = 8
    # method_name = "QueryRankSampleSearchKthRank"
    # for n_sample_item in [2000, 3000, 4000, 5000, 6000]:
    #     n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {method_name} " +
    #         f"--n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #     )
    #     os.system(
    #         f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
    #         f"--method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    dataset_name = 'amazon-home-kitchen'  # yelp, yahoomusic_big
    memory_capacity = 16
    method_name = "QueryRankSampleSearchKthRank"
    k_max = polyu.compute_k_max_in_reverse_mips(dataset_name, memory_capacity)
    os.system(
        f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --test_topk {'false'} --method_name {method_name} --simpfer_k_max {k_max} --stop_time {7200}"
    )


if __name__ == '__main__':
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
