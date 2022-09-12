import os
import filecmp
import numpy as np


def cmp_rtk(baseline_method, rtk_method, dataset_l, topk_l):
    suffix_m = {
        'Simpfer': 'simpfer_k_max_25',
    }
    have_bug = False
    for ds in dataset_l:
        for topk in topk_l:
            if baseline_method in suffix_m:
                baseline_dir = os.path.join('result', 'rank',
                                            '{}-{}-top{}-{}-{}.csv'.format(
                                                ds, baseline_method, topk, suffix_m[baseline_method], 'userID'))
            else:
                baseline_dir = os.path.join('result', 'rank',
                                            '{}-{}-top{}-{}.csv'.format(ds, baseline_method, topk, 'userID'))

            if rtk_method in suffix_m:
                rtk_dir = os.path.join('result', 'rank',
                                       '{}-{}-top{}-{}-{}.csv'.format(
                                           ds, rtk_method, topk, suffix_m[rtk_method],
                                           'userID'))
            else:
                rtk_dir = os.path.join('result', 'rank',
                                       '{}-{}-top{}-{}.csv'.format(
                                           ds, rtk_method, topk, 'userID'))

            with open(rtk_dir, 'r') as f:
                rtk_result_l = []
                for line in f:
                    res = set(map(int, line.split(",")))
                    rtk_result_l.append(res)

            baseline_result_l = np.loadtxt(baseline_dir, delimiter=',')

            assert len(baseline_result_l) == len(rtk_result_l)

            for i in range(len(baseline_result_l)):
                intersect = set(baseline_result_l[i]).intersection(rtk_result_l[i])
                if len(intersect) != len(baseline_result_l[i]):
                    print("have bug dataset {}, topk {}, queryID {}".format(ds, topk, i))
                    have_bug = True
                    exit(-1)
    if not have_bug:
        print("no error, no bug")


def run():
    # for ds in dataset_l:
    #     os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'BatchDiskBruteForce'))
    #     # os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'DiskBruteForce'))
    #     os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'MemoryBruteForce'))
    #     os.system('cd build && ./rri --dataset_name {} --method_name {}'.format(ds, 'Simpfer'))

    # topk_l = [10, 20, 30, 40, 50]
    topk_l = [10, 20, 30]
    cmp_rtk('MemoryBruteForce', 'Simpfer', dataset_l, topk_l)


if __name__ == '__main__':
    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    dataset_l = ['fake-normal']
    # dataset_l = ['fake-normal-query-distribution', 'fake-uniform-query-distribution',
    #              'netflix-small-query-distribution', 'movielens-27m-small-query-distribution']

    run()
