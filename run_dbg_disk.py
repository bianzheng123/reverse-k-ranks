import os
import filecmp
import numpy as np
from script.data_convert import vecs_io
import run_dbg_other as dbg


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp']
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    for dataset_name in ['yahoomusic_big', 'yelp']:
        memory_capacity = 110
        n_data_item = dbg.dataset_m[dataset_name][0]
        n_user = dbg.dataset_m[dataset_name][2]
        n_sample = dbg.compute_n_sample_by_memory_index_sample_only(dataset_name=dataset_name,
                                                                    memory_capacity=memory_capacity)
        n_sample_item = 5000
        sample_topk = 500
        method_name = 'QueryRankSampleSearchKthRank'
        os.system(
            f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {method_name} --n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )
        os.system(
            f"cd build && ./bsibs --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
        # os.system(
        #     f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )

        method_name = 'QueryRankSampleComputeAll'
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )


if __name__ == '__main__':
    username = 'zhengbian'
    dataset_dir = f'/home/{username}/Dataset/ReverseMIPS'
    index_dir = f'/home/{username}/reverse-k-ranks/index'
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
