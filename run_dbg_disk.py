import os
import filecmp
import numpy as np
from script.data_convert import vecs_io
import run_polyuhost as polyu


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'amazon-home-kitchen']
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp']
    dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['amazon-home-kitchen']
    # dataset_l = ['netflix', 'movielens-27m']

    # for ds in dataset_l:
    #     # os.system('cd build && ./bst --dataset_dir {} --dataset_name {} --index_dir {}'.format(
    #     #     dataset_dir, ds, index_dir))
    #     # os.system(
    #     #     'cd build && ./qdi --dataset_dir {} --dataset_name {} --index_dir {} --n_sample_item {} --sample_topk {}'.format(
    #     #         dataset_dir, ds, index_amazon_dir, 5000, 600
    #     #     ))
    #     # TODO
    #     os.system('cd build && ./brsi --dataset_dir {} --dataset_name {} --index_dir {}'.format(
    #         dataset_dir, ds, index_amazon_dir))
    #     n_sample = compute_n_sample_by_memory_index(ds, 64)
    #     os.system('cd build && ./brsi --dataset_dir {} --dataset_name {} --index_dir {} --n_sample {}'.format(
    #         dataset_dir, ds, index_amazon_dir, n_sample))
    #
    #     # os.system(
    #     #     'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
    #     #         dataset_dir, ds, index_dir, 'QueryRankSampleSearchKthRank', n_sample, 5000, 600))
    #     # os.system(
    #     #     'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --simpfer_k_max {}'.format(
    #     #         dataset_dir, ds, index_dir, "Simpfer", 35))

    dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp']
    n_sample_query = 5000
    sample_topk = 600
    for ds in dataset_l:
        for memory_capacity in [2, 4, 8, 16, 32]:
            n_sample = polyu.compute_n_sample_by_memory_index_sample_only(ds, memory_capacity)
            os.system(
                f'cd build && ./ti --index_dir {index_dir} --dataset_name {ds} --old_method_name {"RankSample"} --new_method_name {"RankSample"} --index_type {"RankSample"} --n_sample {n_sample} --n_sample_query {n_sample_query} --sample_topk {sample_topk} --has_sample_score {"true"}'
            )
            os.system(
                f'cd build && ./ti --index_dir {index_dir} --dataset_name {ds} --old_method_name {"QueryRankSearchKthRank"} --new_method_name {"QueryRankSampleSearchKthRank"} --index_type {"QueryRankSearchKthRank"} --n_sample {n_sample} --n_sample_query {n_sample_query} --sample_topk {sample_topk} --has_sample_score {"true"}'
            )
            os.system(
                f'cd build && ./ti --index_dir {index_dir} --dataset_name {ds} --old_method_name {"QueryRankSearchKthRank"} --new_method_name {"QueryRankSampleSearchKthRank"} --index_type {"QueryRankSearchKthRank"} --n_sample {n_sample} --n_sample_query {n_sample_query} --sample_topk {sample_topk} --has_sample_score {"false"}'
            )

    dataset_l = ['amazon-home-kitchen']
    for ds in dataset_l:
        for memory_capacity in [2, 4, 8, 16, 32]:
            n_sample = polyu.compute_n_sample_by_memory_index_sample_only(ds, memory_capacity)
            os.system(
                f'cd build && ./ti --index_dir {index_amazon_dir} --dataset_name {ds} --old_method_name {"RankSample"} --new_method_name {"RankSample"} --index_type {"RankSample"} --n_sample {n_sample} --n_sample_query {n_sample_query} --sample_topk {sample_topk} --has_sample_score {"true"}'
            )
            os.system(
                f'cd build && ./ti --index_dir {index_amazon_dir} --dataset_name {ds} --old_method_name {"QueryRankSearchKthRank"} --new_method_name {"QueryRankSampleSearchKthRank"} --index_type {"QueryRankSearchKthRank"} --n_sample {n_sample} --n_sample_query {n_sample_query} --sample_topk {sample_topk} --has_sample_score {"false"}'
            )


if __name__ == '__main__':
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    index_amazon_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index_amazon')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
