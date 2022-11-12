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
    compute_time_rs = {
        'movielens-27m': 387.406,
        'yahoomusic_big': 13904.212,
        'yelp': 19473.305,
    }
    times = 11

    dataset_l = ['movielens-27m', 'yahoomusic_big', 'yelp']
    for ds in dataset_l:
        memory_capacity = 8
        stop_time = int(compute_time_rs[ds] * times)
        k_max = polyu.compute_k_max_in_reverse_mips(ds, memory_capacity)
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_dir} " +
            f"--test_topk {'false'} --method_name {'Simpfer'} --simpfer_k_max {k_max} --stop_time {stop_time}"
        )


if __name__ == '__main__':
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    index_amazon_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index_amazon')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
