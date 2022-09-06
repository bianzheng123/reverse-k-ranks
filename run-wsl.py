import os
import filecmp
import numpy as np


class CMDcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def delete_file_if_exist(dire):
    if os.path.exists(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


def cmp_file(file1, file2):
    status = filecmp.cmp(file1, file2)
    if status:
        return True
    else:
        return False


def cmp_file_all(method_name_l, type_arr, dataset_l, topk_l):
    flag = True
    suffix_m = {

        'CompressTopTIPBruteForce': 'n_sample_20-index_size_gb_50',
        'CompressTopTIPBruteForceBatchRun': 'n_sample_128-index_size_gb_256',
        'RSCompressTopTIPBruteForceBatchRun': 'n_sample_512-index_size_gb_256',
        'QRSSDTopTIPRefineOrder': 'n_sample_20-index_size_gb_50-n_sample_score_distribution_8',
        'QRSTopTIP': 'n_sample_20-index_size_gb_50',
        'QRSTopTIPRefineOrder': 'n_sample_20-index_size_gb_50',
        'RSTopTIP': 'n_sample_20-index_size_gb_50',
        'RSTopTIPRefineOrder': 'n_sample_20-index_size_gb_50',

        'QueryRankSample': 'n_sample_20',
        'QueryRankSampleScoreDistribution': 'n_sample_20-n_sample_score_distribution_8',
        'RankSample': 'n_sample_20',
        'RankSampleStoreID': 'n_sample_20',
    }
    for ds in dataset_l:
        for topk in topk_l:
            for _type in type_arr:
                for method_idx in range(1, len(method_name_l), 1):
                    # bfon = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'OnlineBruteForce', _type))
                    baseline_method = method_name_l[0]
                    cmp_method = method_name_l[method_idx]
                    if baseline_method in suffix_m:
                        base_method = os.path.join('result', 'rank',
                                                   '{}-{}-top{}-{}-{}.csv'.format(
                                                       ds, method_name_l[0], topk, suffix_m[baseline_method], _type))
                    else:
                        base_method = os.path.join('result', 'rank',
                                                   '{}-{}-top{}-{}.csv'.format(ds, method_name_l[0], topk, _type))

                    if cmp_method in suffix_m:
                        test_method = os.path.join('result', 'rank',
                                                   '{}-{}-top{}-{}-{}.csv'.format(
                                                       ds, method_name_l[method_idx], topk, suffix_m[cmp_method],
                                                       _type))
                    else:
                        test_method = os.path.join('result', 'rank',
                                                   '{}-{}-top{}-{}.csv'.format(
                                                       ds, method_name_l[method_idx], topk, _type))

                    flag = cmp_file(base_method, test_method)
                    if not flag:
                        print("file have diff {} {}".format(base_method, test_method))
                        exit(-1)
    if flag:
        print("no error, no bug")


def run():
    method_name_l = [
        'BatchDiskBruteForce',
        # 'RSCompressTopTIPBruteForceBatchRun',
        # 'DiskBruteForce',
        'MemoryBruteForce',
        # 'QRSSDTopTIPRefineOrder',
        # 'QRSTopTIP',
        # 'QRSTopTIPRefineOrder',
        # 'RSTopTIP',
        # 'RSTopTIPRefineOrder',

        # 'GridIndex',
        # 'QueryRankSample',
        # 'QueryRankSampleScoreDistribution',
        # 'RankSample',
        'RankSampleStoreID',
    ]

    # os.system('cd build && ./{} --dataset_name {}'.format('rb', ds))
    # os.system('cd build && ./{} {}'.format('bbfdi', ds))

    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    for ds in dataset_l:
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'BatchDiskBruteForce'))
        # os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'DiskBruteForce'))
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'MemoryBruteForce'))
        # os.system(
        #     'cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {} && ./rri --dataset_name {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
        #         ds, 150, 30,
        #         ds, 'QRSSDTopTIPRefineOrder', 150, 30))
        # os.system(
        #     'cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {} && ./rri --dataset_name {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
        #         ds, 150, 30,
        #         ds, 'QRSTopTIP', 150, 30))
        # os.system(
        #     'cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {} && ./rri --dataset_name {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
        #         ds, 150, 30,
        #         ds, 'QRSTopTIPRefineOrder', 150, 30))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'RSTopTIP'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'RSTopTIPRefineOrder'))

        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'GridIndex'))

        # os.system(
        #     'cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {} && ./rri --dataset_name {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
        #         ds, 150, 50,
        #         ds, 'QueryRankSample', 150, 50))
        # os.system(
        #     'cd build && ./rri --dataset_name {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
        #         ds, 'QueryRankSampleScoreDistribution', 150, 50))
        # os.system(
        #     'cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {} && ./rri --dataset_name {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
        #         ds, 150, 30,
        #         ds, 'QueryRankSampleScoreDistribution', 150, 30))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'RankSample'))
        os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'RankSampleStoreID'))

    type_arr = ['userID', 'IP', 'rank']
    # topk_l = [10, 20, 30, 40, 50]
    topk_l = [10, 20, 30]
    cmp_file_all(method_name_l, type_arr, dataset_l, topk_l)


if __name__ == '__main__':
    dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    # dataset_l = ['fake-uniform']
    # dataset_l = ['fake-normal-query-distribution', 'fake-uniform-query-distribution',
    #              'netflix-small-query-distribution', 'movielens-27m-small-query-distribution']

    run()
