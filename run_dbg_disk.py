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

    dataset_l = ['amazon-home-kitchen']
    n_sample_item = 5000
    sample_topk = 600
    for ds in dataset_l:
        memory_capacity = 8
        n_sample = polyu.compute_n_sample_by_memory_index_sample_only(ds, memory_capacity)
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_amazon_dir} " +
            f"--test_topk {'false'} --method_name {'RankSample'} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        )
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_amazon_dir} " +
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
