import os
import filecmp
import numpy as np
from script.data_convert import vecs_io


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
    suffix_m = {
        'RankBound': 'cache_bound_every_512',
        'IntervalRankBound': 'cache_bound_every_512-n_interval_1024',

        'CompressTopTIDIPBruteForce': 'cache_bound_every_512-n_interval_1024-topt_200',
        'CompressTopTIPBruteForce': 'cache_bound_every_512-n_interval_1024-topt_200',

        'IRBFullDimPrune': 'cache_bound_every_512-n_interval_1024',
        'IRBFullIntPrune': 'cache_bound_every_512-n_interval_1024',
        'IRBFullNormPrune': 'cache_bound_every_512-n_interval_1024',
        'IRBPartDimPartIntPrune': 'cache_bound_every_512-n_interval_1024',
        'IRBPartDimPartNormPrune': 'cache_bound_every_512-n_interval_1024',
        'IRBPartIntPartNormPrune': 'cache_bound_every_512-n_interval_1024',
        'IRBBallPrune': 'cache_bound_every_512-n_interval_1024',
    }
    flag = True
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
        # 'BPlusTree',
        # 'DiskBruteForce',
        # 'IntervalRankBound',
        'MemoryBruteForce',
        # 'OnlineBruteForce',
        # 'RankBound',

        # 'IRBMergeRankBound',
        'CompressTopTIDIPBruteForce',
        'CompressTopTIPBruteForce',

        # 'IRBFullDimPrune',
        # 'IRBFullIntPrune',
        # 'IRBFullNormPrune',
        # 'IRBPartDimPartIntPrune',
        # 'IRBPartDimPartNormPrune',
        # 'IRBPartIntPartNormPrune',
        # 'IRBBallPrune',
    ]

    # os.system('cd build && ./{} --dataset_name {}'.format('rb', ds))
    # os.system('cd build && ./{} {}'.format('bbfdi', ds))

    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    dataset_l = ['fake-normal', 'fake-uniform', 'fakebig']
    # dataset_l = ['fake-normal', 'fake-uniform']
    for ds in dataset_l:
        os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'BatchDiskBruteForce'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'BPlusTree'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'DiskBruteForce'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'IntervalRankBound'))
        os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'MemoryBruteForce'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'OnlineBruteForce'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'RankBound'))

        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'CompressTopTIDIPBruteForce'))
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'CompressTopTIPBruteForce'))

        # os.system('cd build && ./isbs --dataset_name {} --method_name {}'.format(ds, 'IRBFullDimPrune'))
        # os.system('cd build && ./isbs --dataset_name {} --method_name {}'.format(ds, 'IRBFullIntPrune'))
        # os.system('cd build && ./isbs --dataset_name {} --method_name {}'.format(ds, 'IRBFullNormPrune'))
        # os.system('cd build && ./isbs --dataset_name {} --method_name {}'.format(ds, 'IRBPartDimPartIntPrune'))
        # os.system('cd build && ./isbs --dataset_name {} --method_name {}'.format(ds, 'IRBPartDimPartNormPrune'))
        # os.system('cd build && ./isbs --dataset_name {} --method_name {}'.format(ds, 'IRBPartIntPartNormPrune'))
        # os.system('cd build && ./isbs --dataset_name {} --method_name {}'.format(ds, 'IRBBallPrune'))

        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'IntervalRankBoundCompress'))

        # os.system('cd build && ./{} {}'.format('irbfdp', ds))
        # os.system('cd build && ./{} {}'.format('irbfip', ds))
        # os.system('cd build && ./{} {}'.format('irbfnp', ds))
        # os.system('cd build && ./{} {}'.format('irbpdpip', ds))
        # os.system('cd build && ./{} {}'.format('irbpdpnp', ds))
        # os.system('cd build && ./{} {}'.format('irbpipnp', ds))
        # os.system('cd build && ./{} {}'.format('irbbp', ds))

        # os.system('cd build && ./{} --dataset_name {}'.format('rbnet', ds))

    type_arr = ['userID', 'IP', 'rank']
    topk_l = [10, 20, 30, 40, 50, 60, 70]
    cmp_file_all(method_name_l, type_arr, dataset_l, topk_l)


if __name__ == '__main__':
    dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    # run(method_name='BPlusTree', program_name='bpt')
    run()
    # run(method_name='BallIntervalRankBound', program_name='birb')
    # run(method_name='IntervalRankBound', program_name='irb')
    # run(method_name='RankBound', program_name='rb')
