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

    dataset_name = 'yelp'
    method_name = 'QueryRankSampleMinMaxIntLR'
    memory_capacity = 8
    n_sample = dbg.compute_n_sample_by_memory_index_sample_only(dataset_name=dataset_name,
                                                                memory_capacity=memory_capacity)
    n_sample_item = 5000
    sample_topk = 250
    # os.system(
    #     f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {method_name} --n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    # )
    os.system(
        f"cd build && ./bsibs --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
        f"--method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
    # os.system(
    #     f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    os.system(
        f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
        f"--method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    os.system(
        f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
        f"--test_topk {'false'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )

    '''remember to disable multithread'''
    dataset_name = 'amazon-home-kitchen'
    k_max = dbg.compute_k_max_in_reverse_mips(dataset_name, 8)
    os.system(
        f'cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} '
        f'--test_topk {"false"} --method_name {"Simpfer"} --simpfer_k_max {k_max} --stop_time {43200}')

    # dataset_name = 'amazon-home-kitchen'
    # os.system(
    #     'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --test_topk {} --method_name {} --stop_time {}'.format(
    #         dataset_dir, dataset_name, index_dir, 'false', 'GridIndex', 9000))

    '''rmips'''
    # dataset_name = "yahoomusic_big"  # movielens-27m yahoomusic_big yelp amazon-home-kitchen
    # for memory_capacity in [32]:
    #     for dataset_name in ['yahoomusic_big', 'yelp']:
    #         k_max = dbg.compute_k_max_in_reverse_mips(dataset_name, memory_capacity)
    #         os.system(
    #             f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
    #             f"--test_topk {'false'} --method_name {'SimpferOnly'} --simpfer_k_max {k_max} --stop_time {70000} "
    #         )


if __name__ == '__main__':
    username = 'zhengbian'
    dataset_dir = f'/home/{username}/Dataset/ReverseMIPS'
    index_dir = f'/home/{username}/reverse-k-ranks/index'
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    for ds in ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']:
        os.system(f'cd build && ./bst --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_dir}')

    # run()
