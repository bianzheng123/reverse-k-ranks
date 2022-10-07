import os
import numpy as np
import filecmp


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
    return filecmp.cmp(file1, file2)


def cmp_file_all(baseline_method, compare_method_l, dataset_l, topk_l):
    flag = True
    suffix_m = {

        'Grid': 'codeword_64',
        'FullInt': 'scale_1000',
        'PartDimPartInt': 'scale_1000',
        'PartIntPartNorm': 'scale_1000',
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

        # 'Grid',
        # 'FullDim',
        # 'FullNorm',
        # 'FullInt',
        # 'PartDimPartInt',
        # 'PartDimPartNorm',
        # 'PartIntPartNorm',

    ]

    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    for ds in dataset_l:
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'BatchDiskBruteForce'))
        # os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'DiskBruteForce'))
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'MemoryBruteForce'))

        # os.system('cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, 'Grid'))
        # os.system('cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, 'FullDim'))
        # os.system('cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, 'FullNorm'))
        # os.system('cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, 'FullInt'))
        # os.system('cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, 'PartDimPartInt'))
        # os.system('cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, 'PartDimPartNorm'))
        # os.system('cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, 'PartIntPartNorm'))

    # topk_l = [10, 20, 30, 40, 50]
    topk_l = [10, 20, 30]
    cmp_file_all('BatchDiskBruteForce', method_name_l, dataset_l, topk_l)


if __name__ == '__main__':
    # dataset_l = ['fake-small', 'fake-normal', 'fake-uniform', 'netflix-small']
    dataset_l = ['fake-small']
    # dataset_l = ['fake-normal-query-distribution', 'fake-uniform-query-distribution',
    #              'netflix-small-query-distribution', 'movielens-27m-small-query-distribution']

    # for ds in dataset_l:
    #     for bound_name in ['Grid', 'FullDim', 'FullNorm', 'FullInt', 'PartDimPartInt', 'PartDimPartNorm',
    #                        'PartIntPartNorm']:
    #         os.system(
    #             'cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, bound_name))

    run()
