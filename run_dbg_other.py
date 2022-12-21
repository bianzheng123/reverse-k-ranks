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

    dataset_name = 'amazon-home-kitchen'
    k_max = polyu.compute_k_max_in_reverse_mips(dataset_name, 8)
    os.system(
        f'cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} '
        f'--test_topk {"false"} --method_name {"Simpfer"} --simpfer_k_max {k_max} --stop_time {43200}')

    # dataset_name = 'movielens-27m'
    # os.system(
    #     'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --test_topk {} --method_name {} --stop_time {}'.format(
    #         dataset_dir, dataset_name, index_dir, 'false', 'GridIndex', 9000))

    # for method_name, dataset_name, n_sample in [('MinMaxLinearRegression', 'yahoomusic_big', 588),
    #                                             ('UniformLinearRegression', 'yelp', 490)]:
    #     n_sample_query = 5000
    #     for sample_topk in [10, 50, 100, 150, 200, 300, 400, 500, 600]:
    #         os.system(
    #             f'cd index/memory_index && '
    #             f'{method_name}-{dataset_name}-n_sample_{n_sample}-n_sample_query_{n_sample_query}-sample_topk_{sample_topk}.index '
    #             f'{method_name}-{dataset_name}_more_query-n_sample_{n_sample}-n_sample_query_{n_sample_query}-sample_topk_{sample_topk}.index')
    #         os.system(
    #             f'cd index/memory_index && '
    #             f'{method_name}-{dataset_name}-n_sample_{n_sample}-n_sample_query_{n_sample_query}-sample_topk_{sample_topk}.index '
    #             f'QueryRankSampleSearchKthRank-{dataset_name}_more_query-n_sample_{n_sample}-n_sample_query_{n_sample_query}-sample_topk_{sample_topk}.index')
    #
    # dataset_name = "yahoomusic_big"  # movielens-27m yahoomusic_big yelp amazon-home-kitchen
    # k_max = polyu.compute_k_max_in_reverse_mips(dataset_name, 0.5)
    # os.system(
    #     f"cd build && ./rtk --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
    #     f"--test_topk {'false'} --method_name {'SimpferFEXIPROOnly'} --simpfer_k_max {k_max} --stop_time {70000} "
    #     f"--test_reverse_topk {64}"
    # )
    #
    # dataset_name = "yelp"  # movielens-27m yahoomusic_big yelp amazon-home-kitchen
    # k_max = polyu.compute_k_max_in_reverse_mips(dataset_name, 0.5)
    # os.system(
    #     f"cd build && ./rtk --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} "
    #     f"--test_topk {'false'} --method_name {'SimpferFEXIPROOnly'} --simpfer_k_max {k_max} --stop_time {70000} "
    #     f"--test_reverse_topk {32}"
    # )

    # n_sample_item = 5000
    # dataset_name = 'yelp'  # yelp, yahoomusic_big
    # memory_capacity = 8
    # method_name = "QueryRankSampleMinMaxIntLR"  # QueryRankSampleUniformIntLR QueryRankSampleMinMaxIntLR
    # for sample_topk in [100, 200, 300, 400, 500]:
    #     n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
    #     os.system(
    #         f"cd index/memory_index && mv UniformLinearRegression-yahoomusic_big-n_sample_{n_sample}.index "
    #         f"UniformLinearRegression-yahoomusic_big_n_sample_{n_sample}_n_sample_query_{n_sample_item}_sample_topk_{sample_topk}.index"
    #     )
    #
    # sample_topk = 600
    # dataset_name = 'yahoomusic_big'  # yelp, yahoomusic_big
    # memory_capacity = 8
    # method_name = "QueryRankSampleSearchUniformRankUniformIntLR"  # QueryRankSampleUniformIntLR QueryRankSampleMinMaxIntLR
    # for n_sample_item in [5000]:
    #     n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
    #
    # dataset_name = 'yahoomusic_big'  # yelp, yahoomusic_big
    # n_data_item = polyu.dataset_m[dataset_name][0]
    # n_user = polyu.dataset_m[dataset_name][2]
    # memory_capacity = 8
    # method_name = "QueryRankSampleSearchKthRank"
    # n_sample_item = 5000
    # for sample_topk in [150]:
    #     n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {method_name} " +
    #         f"--n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    #     )
    #     os.system(
    #         f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} " +
    #         f"--method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
    #
    # os.system('cd index/memory_index && '
    #           'cp QueryRankSampleSearchKthRank-yahoomusic_big-n_sample_588-n_sample_query_5000-sample_topk_150.index'
    #           'QueryRankSampleIntLR-yahoomusic_big-n_sample_588-n_sample_query_5000-sample_topk_150.index')
    # n_sample_item = 5000
    # dataset_name = 'yahoomusic_big'  # yelp, yahoomusic_big
    # memory_capacity = 8
    # method_name = "QueryRankSampleMinMaxIntLR"  # QueryRankSampleUniformIntLR QueryRankSampleMinMaxIntLR
    # for sample_topk in [150]:
    #     n_sample = polyu.compute_n_sample_by_memory_index_sample_only(dataset_name, memory_capacity)
    #     os.system(
    #         f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")


if __name__ == '__main__':
    index_dir = os.path.join('/home', 'zhengbian', 'reverse-k-ranks', 'index')
    dataset_dir = os.path.join('/home', 'zhengbian', 'Dataset', 'ReverseMIPS')
    # index_dir = os.path.join('/data', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()
