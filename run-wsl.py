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
    flag = True
    for ds in dataset_l:
        for topk in topk_l:
            for _type in type_arr:
                for method_idx in range(1, len(method_name_l), 1):
                    # bfon = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'OnlineBruteForce', _type))
                    base_method = os.path.join('result', 'rank',
                                               '{}-{}-top{}-{}.csv'.format(ds, method_name_l[0], topk, _type))
                    test_method = os.path.join('result', 'rank',
                                               '{}-{}-top{}-{}.csv'.format(ds, method_name_l[method_idx], topk, _type))

                    flag = cmp_file(base_method, test_method)
                    if not flag:
                        print("file have diff {} {}".format(base_method, test_method))
                        exit(-1)
    if flag:
        print("no error, no bug")


def run():
    method_m = {
        'RankBound': 'rb',
        'BatchDiskBruteForce': 'bbfdi',
        # 'BPlusTree': 'bpt',
        # 'IntervalRankBound': 'irb',
        # 'BallIntervalRankBound': 'birb',

        # 'OnlineBruteForce': 'bfon',
        'MemoryBruteForce': 'bfmi',
        # 'DiskBruteForce': 'bfdi',

        # 'IRBFullDimPrune': 'irbfdp',
        # 'IRBFullIntPrune': 'irbfip',
        # 'IRBFullNormPrune': 'irbfnp',
        # 'IRBPartDimPartIntPrune': 'irbpdpip',
        # 'IRBPartDimPartNormPrune': 'irbpdpnp',
        # 'IRBPartIntPartNormPrune': 'irbpipnp',
        'IRBBallPrune': 'irbbp',
    }
    # os.system('cd build && ./{} --dataset_name {}'.format('rb', ds))
    # os.system('cd build && ./{} {}'.format('bbfdi', ds))

    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    for ds in dataset_l:
        os.system('cd build && ./{} --dataset_name {}'.format('rb', ds))
        # os.system('cd build && ./{} {}'.format('bbfdi', ds))
        # os.system('cd build && ./{} {}'.format('bpt', ds))
        os.system('cd build && ./{} --dataset_name {}'.format('irb', ds))
        # os.system('cd build && ./{} {}'.format('birb', ds))

        # os.system('cd build && ./{} {}'.format('bfon', ds))
        os.system('cd build && ./{} {}'.format('bfmi', ds))
        # os.system('cd build && ./{} {}'.format('bfdi', ds))

        # os.system('cd build && ./{} {}'.format('irbfdp', ds))
        # os.system('cd build && ./{} {}'.format('irbfip', ds))
        # os.system('cd build && ./{} {}'.format('irbfnp', ds))
        # os.system('cd build && ./{} {}'.format('irbpdpip', ds))
        # os.system('cd build && ./{} {}'.format('irbpdpnp', ds))
        # os.system('cd build && ./{} {}'.format('irbpipnp', ds))
        os.system('cd build && ./{} {}'.format('irbbp', ds))

    method_name_l = list(method_m.keys())
    type_arr = ['userID', 'IP', 'rank']
    topk_l = [10, 20, 30, 40, 50, 60, 70]
    cmp_file_all(method_name_l, type_arr, dataset_l, topk_l)


if __name__ == '__main__':
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # run(method_name='BPlusTree', program_name='bpt')
    run()
    # run(method_name='BallIntervalRankBound', program_name='birb')
    # run(method_name='IntervalRankBound', program_name='irb')
    # run(method_name='RankBound', program_name='rb')
