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
        'CompressTopTIDBruteForce': 'n_sample_128-index_size_gb_128',
        'CompressTopTIPBruteForce': 'n_sample_128-index_size_gb_128',
        # 'CompressTopTIDBruteForce': 'n_sample_20-index_size_gb_50',
        # 'CompressTopTIPBruteForce': 'n_sample_20-index_size_gb_50',

        'QuadraticBound': 'n_sample_20',
        'QuadraticScoreBound': 'n_sample_20',
        'RankSample': 'cache_bound_every_512',
        'ScoreSample': 'n_sample_20',
        'SSComputeAll': 'n_sample_20',
        'GPUScoreSample': 'n_sample_20',

        'SSMergeIntervalIDByBitmap': 'n_sample_20-index_size_gb_50',
        'SSMergeIntervalIDByInterval': 'n_sample_20-index_size_gb_50',
        'SSMergeQuadraticRankBoundByBitmap': 'n_sample_20-index_size_gb_50',
        'SSMergeRankByBitmap': 'n_sample_20-index_size_gb_50',
        'SSMergeRankByInterval': 'n_sample_20-index_size_gb_50',

        'CAGrid': 'codeword_32',
        'CAFullInt': 'scale_100',
        'CAPartDimPartInt': 'scale_100',
        'CAPartIntPartNorm': 'scale_100',
        'CAUserItemPQ': 'n_codebook_8-n_codeword_32',
        'CAItemPQ': 'n_codebook_8-n_codeword_32'
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
        'CompressTopTIDBruteForce',
        'CompressTopTIPBruteForce',
        # 'DiskBruteForce',
        'MemoryBruteForce',

        # 'GridIndex',
        # 'RankSample',
        # 'ScoreSample',
        # 'SSComputeAll',

        # 'SSMergeQuadraticRankBoundByBitmap',
        # 'SSMergeRankByInterval',

        # 'CAGrid',
        # 'CAFullDim',
        # 'CAFullNorm',
        # 'CAFullInt',
        # 'CAPartDimPartInt',
        # 'CAPartDimPartNorm',
        # 'CAPartIntPartNorm',
        # 'CAUserItemPQ',
        # 'CAItemPQ',
    ]

    # os.system('cd build && ./{} --dataset_name {}'.format('rb', ds))
    # os.system('cd build && ./{} {}'.format('bbfdi', ds))

    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    # dataset_l = ['fake-small', 'fake']
    for ds in dataset_l:
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'BatchDiskBruteForce'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'CompressTopTIDBruteForce'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'CompressTopTIPBruteForce'))
        os.system('cd build && ./brtt --dataset_name {}'.format(ds))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'DiskBruteForce'))
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'MemoryBruteForce'))

        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'GridIndex'))

        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'RankSample'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'ScoreSample'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'SSComputeAll'))

        # os.system(
        #     'cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'SSMergeQuadraticRankBoundByBitmap'))
        # os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'SSMergeRankByInterval'))

        # os.system('cd build && ./ca --dataset_name {} --bound_name {}'.format(ds, 'CAGrid'))
        # os.system('cd build && ./ca --dataset_name {} --bound_name {}'.format(ds, 'CAFullDim'))
        # os.system('cd build && ./ca --dataset_name {} --bound_name {}'.format(ds, 'CAFullNorm'))
        # os.system('cd build && ./ca --dataset_name {} --bound_name {}'.format(ds, 'CAFullInt'))
        # os.system('cd build && ./ca --dataset_name {} --bound_name {}'.format(ds, 'CAPartDimPartInt'))
        # os.system('cd build && ./ca --dataset_name {} --bound_name {}'.format(ds, 'CAPartDimPartNorm'))
        # os.system('cd build && ./ca --dataset_name {} --bound_name {}'.format(ds, 'CAPartIntPartNorm'))
        # os.system('cd build && ./ca --dataset_name {} --bound_name {}'.format(ds, 'CAUserItemPQ'))
        # os.system('cd build && ./ca --dataset_name {} --bound_name {}'.format(ds, 'CAItemPQ'))

    type_arr = ['userID', 'IP', 'rank']
    topk_l = [10, 20, 30, 40, 50, 60, 70]
    # topk_l = [10, 20]
    cmp_file_all(method_name_l, type_arr, dataset_l, topk_l)


if __name__ == '__main__':
    dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    # dataset_l = ['fake-uniform']
    # dataset_l = ['netflix-small']
    # dataset_l = ['fake-small', 'fake']
    # dataset_l = ['fake']
    run()
