import os
import filecmp
import numpy as np
from script.data_convert import vecs_io
import run_polyuhost as polyu


def test_build_score_table(dataset_name: str, eval_size_gb: int = 100):
    username = 'bianzheng'
    dataset_dir = f'/home/{username}/Dataset/ReverseMIPS'
    index_dir = f'/home/{username}/reverse-k-ranks/index'
    os.system(
        f'cd build && ./bstb --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --eval_size_gb {eval_size_gb} ')
    os.system(f'cd index && rm {dataset_name}.index')


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp',
                 'amazon-home-kitchen']
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    dataset_name = 'amazon-home-kitchen'
    k_max = polyu.compute_k_max_in_reverse_mips(dataset_name, 8)
    os.system(
        f'cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} '
        f'--test_topk {"false"} --method_name {"Simpfer"} --simpfer_k_max {k_max} --stop_time {43200}')


if __name__ == '__main__':
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    # run()
    test_build_score_table(dataset_name='movielens-27m', eval_size_gb=500)
    test_build_score_table(dataset_name='netflix', eval_size_gb=500)
    test_build_score_table(dataset_name='yahoomusic_big', eval_size_gb=500)
    test_build_score_table(dataset_name='yelp', eval_size_gb=500)
    test_build_score_table(dataset_name='amazon-home-kitchen', eval_size_gb=500)
