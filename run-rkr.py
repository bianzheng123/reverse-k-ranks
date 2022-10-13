import os
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
    method1_result_l = np.loadtxt(file1, delimiter=',')
    method2_result_l = np.loadtxt(file2, delimiter=',')

    assert len(method1_result_l) == len(method2_result_l)

    for i in range(len(method1_result_l)):
        intersect = set(method1_result_l[i]).intersection(set(method2_result_l[i]))
        if len(intersect) != len(method1_result_l[i]):
            return False
    return True


def cmp_file_all(baseline_method, compare_method_l, dataset_l, topk_l):
    flag = True
    suffix_m = {

        'QRSTopTIP': 'n_sample_20-index_size_gb_50',
        'RSTopTIP': 'n_sample_20-index_size_gb_50',

        'QueryRankSampleSearchAllRank': 'n_sample_20',
        'QueryRankSampleSearchKthRank': 'n_sample_20',
        'RankSample': 'n_sample_20',
        'RankSampleApprByGridIPBound': 'n_sample_20',
        'RankSampleApprByNormIPBound': 'n_sample_20',
        'RankSampleApprByUserIPBound': 'n_sample_20',
        'RankSampleIntIPBound': 'n_sample_20',
        'RankSampleIntPGM': 'n_sample_20',
    }
    for ds in dataset_l:
        for topk in topk_l:
            for method_idx in range(len(compare_method_l)):
                cmp_method = compare_method_l[method_idx]
                if baseline_method in suffix_m:
                    baseline_dir = os.path.join('result', 'rank',
                                                '{}-{}-top{}-{}-userID.csv'.format(
                                                    ds, baseline_method, topk, suffix_m[baseline_method]))
                else:
                    baseline_dir = os.path.join('result', 'rank',
                                                '{}-{}-top{}-userID.csv'.format(
                                                    ds, baseline_method, topk))

                if cmp_method in suffix_m:
                    cmp_dir = os.path.join('result', 'rank',
                                           '{}-{}-top{}-{}-userID.csv'.format(
                                               ds, cmp_method, topk, suffix_m[cmp_method]))
                else:
                    cmp_dir = os.path.join('result', 'rank',
                                           '{}-{}-top{}-userID.csv'.format(
                                               ds, cmp_method, topk))

                flag = cmp_file(baseline_dir, cmp_dir)
                if not flag:
                    print("file have diff {} {}".format(baseline_dir, cmp_dir))
                    exit(-1)
    if flag:
        print("no error, no bug")


def run():
    method_name_l = [
        'BatchDiskBruteForce',
        # 'DiskBruteForce',
        'MemoryBruteForce',

        # 'GridIndex',
        'LinearModel',
        # 'QueryRankSampleSearchAllRank',
        # 'QueryRankSampleSearchKthRank',
        # 'RankSample',
        # 'RankSampleIntPGM',
    ]

    # os.system('cd build && ./{} --dataset_name {}'.format('rb', ds))
    # os.system('cd build && ./{} {}'.format('bbfdi', ds))

    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    for ds in dataset_l:
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'BatchDiskBruteForce'))
        # os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'DiskBruteForce'))
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'MemoryBruteForce'))

        # os.system('cd build && ./bst --dataset_name {}'.format(ds))

        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'GridIndex'))
        os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'LinearModel'))
        # os.system(
        #     'cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {} && ./rri --dataset_name {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
        #         ds, 150, 10,
        #         ds, 'QueryRankSampleSearchAllRank', 150, 10))
        # os.system(
        #     'cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {} && ./rri --dataset_name {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
        #         ds, 150, 30,
        #         ds, 'QueryRankSampleSearchKthRank', 150, 30))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'RankSample'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'RankSampleIntPGM'))

    # topk_l = [10, 20, 30, 40, 50]
    topk_l = [10, 20, 30]
    cmp_file_all('BatchDiskBruteForce', method_name_l, dataset_l, topk_l)


if __name__ == '__main__':
    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    # dataset_l = ['fake-normal', 'fake-uniform']
    dataset_l = ['fake-normal']
    # dataset_l = ['fake-normal-query-distribution', 'fake-uniform-query-distribution',
    #              'netflix-small-query-distribution', 'movielens-27m-small-query-distribution']

    # for ds in dataset_l:
    #     for bound_name in ['Grid', 'FullDim', 'FullNorm', 'FullInt', 'PartDimPartInt', 'PartDimPartNorm',
    #                        'PartIntPartNorm']:
    #         os.system(
    #             'cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, bound_name))

    run()
