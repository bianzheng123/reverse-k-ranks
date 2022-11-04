import os
import filecmp
import numpy as np
from script.data_convert import vecs_io
import run_polyuhost as polyu


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp']  # TODO specify the dataset name
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    for ds in dataset_l:
        n_sample_item = 5000
        sample_topk = 600
        n_data_item = polyu.dataset_m[ds][0]
        n_user = polyu.dataset_m[ds][2]
        for memory_capacity in [16]:  # TODO specify the memory capacity
            n_sample = polyu.compute_n_sample_by_memory_index_intlr(ds, memory_capacity)
            method_name = "QueryRankSampleIntLR"
            sample_name = polyu.sample_name_m[method_name]
            os.system(
                f"cd build && ./fsr --index_dir {index_dir} --dataset_name {ds} --sample_name {sample_name} --method_name {method_name} --n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
            )


if __name__ == '__main__':
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
