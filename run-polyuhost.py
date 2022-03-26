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


def run_attribution():
    for ds in dataset_l:
        os.system('cd build/attribution && ./ipc {}'.format(ds))

    os.system('cd build/attribution && ./pi')

    os.system('cd build/attribution && ./pp')

    for ds in dataset_l:
        os.system('cd build/attribution && ./dvd {}'.format(ds))
        os.system('cd attribution/svd-compare && python3 plot_curve.py -ds {}'.format(ds))

        os.system('cd build/attribution && ./svdcmp {}'.format(ds))


def run(method_name='IntervalRankBound', program_name='irb'):
    method_m = {
        # 'OnlineBruteForce': 'bfon',
        # 'MemoryBruteForce': 'bfmi',
        # 'DiskBruteForce': 'bfdi',
        'BatchDiskBruteForce': 'bbfdi',
        # 'IntervalRankBound': 'irb'
    }
    for ds in dataset_l:
        if program_name == 'rb':
            os.system('cd build && ./{} --dataset_name {} --basic_dir {}'.format(program_name, ds, basic_dir))
        else:
            os.system('cd build && ./{} {} {}'.format(program_name, ds, basic_dir))

        for method in method_m:
            os.system('cd build && ./{} {} {}'.format(method_m[method], ds, basic_dir))

    type_arr = ['index', 'IP', 'rank']
    topk_l = [10, 20, 30, 40, 50]

    flag = True
    for ds in dataset_l:
        for topk in topk_l:
            for method in method_m:
                for _type in type_arr:
                    # bfon = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'OnlineBruteForce', _type))
                    base_method = os.path.join('result', 'rank', '{}-{}-top{}-{}.csv'.format(ds, method, topk, _type))
                    test_method = os.path.join('result', 'rank',
                                               '{}-{}-top{}-{}.csv'.format(ds, method_name, topk, _type))

                    flag = cmp_file(base_method, test_method)
                    if not flag:
                        print("file diff {} {}".format(base_method, test_method))
    if flag:
        print("no error, no bug")


def run_rankbound_sample_rate():
    # dataset_name_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    cache_bound_every_l = [20, 40, 60, 80]
    for ds in dataset_l:
        for para in cache_bound_every_l:
            os.system('cd build && ./rb --dataset_name {} --cache_bound_every {}'.format(ds, para))

    flag = True
    for ds in dataset_l:
        for topk in [10, 20, 30, 40, 50]:
            for i in range(len(cache_bound_every_l)):
                if i != 0:
                    file1 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-index.csv".format(ds, topk,
                                                                                                   cache_bound_every_l[
                                                                                                       0])
                    file2 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-index.csv".format(ds, topk,
                                                                                                   cache_bound_every_l[
                                                                                                       i])
                    flag = cmp_file(file1, file2)
                    if not flag:
                        print("file diff {} {}".format(file1, file2))

                    file1 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-IP.csv".format(ds, topk,
                                                                                                cache_bound_every_l[0])
                    file2 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-IP.csv".format(ds, topk,
                                                                                                cache_bound_every_l[i])
                    flag = cmp_file(file1, file2)
                    if not flag:
                        print("file diff {} {}".format(file1, file2))

                    file1 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-rank.csv".format(ds, topk,
                                                                                                  cache_bound_every_l[
                                                                                                      0])
                    file2 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-rank.csv".format(ds, topk,
                                                                                                  cache_bound_every_l[
                                                                                                      i])
                    flag = cmp_file(file1, file2)
                    if not flag:
                        print("file diff {} {}".format(file1, file2))
    if flag:
        print("no error, no bug")


if __name__ == '__main__':
    basic_dir = os.path.join('run', 'media', 'hdd', 'ReverseMIPS')
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']

    os.system('cd build && ./rb --dataset_name {} --basic_dir {}'.format('movielens-27m', basic_dir))
    os.system('cd build && ./irb {} {}'.format('movielens-27m', basic_dir))

    os.system('cd build && ./irbfdp {} {}'.format('movielens-27m', basic_dir))
    os.system('cd build && ./irbfip {} {}'.format('movielens-27m', basic_dir))
    os.system('cd build && ./irbfnp {} {}'.format('movielens-27m', basic_dir))
    os.system('cd build && ./irbpdpip {} {}'.format('movielens-27m', basic_dir))
    os.system('cd build && ./irbpdpnp {} {}'.format('movielens-27m', basic_dir))
    os.system('cd build && ./irbpipnp {} {}'.format('movielens-27m', basic_dir))
