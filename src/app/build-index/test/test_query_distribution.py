import os
import create_test_dataset


def run_sample_method(method_name, dataset_name, n_sample, n_sample_item, sample_topk):
    dataset_m = {'fake-normal': [5000, 100, 1000],
                 'fake-uniform': [5000, 100, 1000],
                 'fakebig': [5000, 100, 5000],
                 'netflix-small': [5000, 100, 2000],

                 'fake-normal-query-distribution': [5000, 100, 1000],
                 'fake-uniform-query-distribution': [5000, 100, 1000],
                 'fakebig-query-distribution': [5000, 100, 5000],
                 'netflix-small-query-distribution': [5000, 100, 2000]}
    n_data_item = dataset_m[dataset_name][0]
    n_user = dataset_m[dataset_name][2]
    sample_name_m = {
        'QueryRankSampleIntLR': 'OptimalPart',
        'QueryRankSampleLeastSquareIntLR': 'OptimalPart',
        'QueryRankSampleScoreDistribution': 'OptimalPart',
        'QueryRankSampleSearchAllRank': 'OptimalAll',
        'QueryRankSampleSearchKthRank': 'OptimalPart',
        'RankSample': 'Uniform',
    }
    sample_name = sample_name_m[method_name]

    os.system(
        f"cd /home/bianzheng/reverse-k-ranks/build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --sample_name {sample_name} --method_name {method_name} --n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )
    os.system(
        f"cd /home/bianzheng/reverse-k-ranks/build && ./bsi --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
    os.system(
        f"cd /home/bianzheng/reverse-k-ranks/build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --test_topk {'true'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )


if __name__ == '__main__':
    ds_m = {'fake-normal': 'fake-normal-query-distribution', 'fake-uniform': 'fake-uniform-query-distribution',
            'fakebig': 'fakebig-query-distribution', 'netflix-small': 'netflix-small-query-distribution'}
    basic_dir = '/home/bianzheng/Dataset/ReverseMIPS'
    # basic_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    n_sample_item = 150
    sample_topk = 10
    n_sample = 20
    index_dir = "/home/bianzheng/reverse-k-ranks/index"
    dataset_dir = "/home/bianzheng/Dataset/ReverseMIPS"

    for ds in ds_m.keys():
        os.system(
            "cd /home/bianzheng/reverse-k-ranks/build && ./qdi --index_dir {} --dataset_dir {} --dataset_name {} --n_sample_item {} --sample_topk {}".format(
                index_dir, dataset_dir, ds, n_sample_item, sample_topk
            ))

    os.system('cd /home/bianzheng/reverse-k-ranks/result/rank && rm *')
    os.system('cd /home/bianzheng/reverse-k-ranks/result/single_query_performance && rm *')
    os.system('cd /home/bianzheng/reverse-k-ranks/result/vis_performance && rm *')
    os.system('cd /home/bianzheng/reverse-k-ranks/index/memory_index && rm *')

    for from_ds in ds_m.keys():
        create_test_dataset.change_index(from_ds, ds_m[from_ds], basic_dir, n_sample_item, sample_topk)

    for ds in ds_m.values():
        os.system('cd /home/bianzheng/reverse-k-ranks/build && ./bst --dataset_name {}'.format(ds))
        run_sample_method('QueryRankSampleSearchKthRank', ds, n_sample, n_sample_item, sample_topk)
        run_sample_method('QueryRankSampleSearchAllRank', ds, n_sample, n_sample_item, sample_topk)
