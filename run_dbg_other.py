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

    n_sample_item = 5000
    sample_topk = 600
    # for dataset_name in dataset_l:
    #     n_data_item = polyu.dataset_m[dataset_name][0]
    #     n_user = polyu.dataset_m[dataset_name][2]
    #     method_name = "QueryRankSampleMinMaxIntLR"
    #     os.system(
    #         f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {-1} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
    #
    #     if method_name == 'QueryRankSampleLeastSquareIntLR' or method_name == 'QueryRankSampleMinMaxIntLR' or method_name == "QueryRankSampleDirectIntLR":
    #         os.system(
    #             f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {-1} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    dataset_name = 'amazon-home-kitchen'
    n_data_item = polyu.dataset_m[dataset_name][0]
    n_user = polyu.dataset_m[dataset_name][2]

    for memory_capacity in [16]:  # TODO specify the memory capacity
        n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
        os.system(
            f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {'QueryRankSampleSearchKthRank'} --n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )
        # os.system(
        #     f"cd build && ./bsibs --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

        # os.system(
        #     f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        # )


if __name__ == '__main__':
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
