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
    cache_bound_every_l = [512, 1024, 1536, 2048, 2560, 3072, 3584, 4096]
    for ds in dataset_l:
        for para in cache_bound_every_l:
            os.system(
                'cd build && ./rri --method_name {} --dataset_name {} --basic_dir {} --cache_bound_every {}'.format(
                    'BPlusTree', ds, basic_dir, para))
            # os.system(
            #     'cd build && ./rri --method_name {} --dataset_name {} --cache_bound_every {} --basic_dir {}'.format(
            #         'HashRankBound', ds, para, basic_dir))


def run_intervalrankbound_sample_rate():
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    n_interval_l = [16, 32, 64, 128, 256, 512, 1024, 2048]
    for ds in dataset_l:
        for n_interval in n_interval_l:
            os.system(
                'cd build && ./rri --method_name {} --dataset_name {} --basic_dir {} --cache_bound_every {} --n_interval {}'.format(
                    'HashRankBound', ds, basic_dir, 512, n_interval))


def run_compress_topt():
    dataset_l = ['netflix', 'yahoomusic-small', 'yelp-small']
    topt_perc_l = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
    for ds in dataset_l:
        for topt_perc in topt_perc_l:
            os.system(
                'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --topt_perc {}'.format(
                    ds, basic_dir, "CompressTopTIDIPBruteForce", topt_perc))
            os.system(
                'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --topt_perc {}'.format(
                    ds, basic_dir, "CompressTopTIPBruteForce", topt_perc))


def run_sample_rank_bound():
    # dataset_l = ['netflix', 'yahoomusic-small', 'yelp-small']
    n_sample_l = [1, 2, 4, 8, 16, 32]
    for ds in dataset_l:
        for n_sample in n_sample_l:
            os.system(
                'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {} --cache_bound_every {}'.format(
                    ds, basic_dir, "PartRankBound", n_sample, 512))


if __name__ == '__main__':
    basic_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix', 'yelp']
    dataset_l = ['movielens-27m', 'netflix', 'yahoomusic-small', 'yelp-small']
    # dataset_l = ['movielens-27m']
    # run_compress_topt()

    # run_rankbound_sample_rate()
    # run_intervalrankbound_sample_rate()
    run_sample_rank_bound()

    # for ds in dataset_l:
    #     os.system(
    #         'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {}'.format(
    #             ds, basic_dir, "HashRankBound"))
    #     os.system(
    #         'cd build && ./irb --dataset_name {} --basic_dir {} --cache_bound_every {} --n_interval {}'.format(
    #             ds, basic_dir, 512, 1024))
    #     os.system(
    #         'cd build && ./rb --dataset_name {} --cache_bound_every {} --basic_dir {}'.format(ds, 512,
    #                                                                                           basic_dir))
    #     os.system(
    #         'cd build && ./rb --dataset_name {} --cache_bound_every {} --basic_dir {}'.format(ds, 1024,
    #                                                                                           basic_dir))
    #     os.system('cd build && ./rb --dataset_name {} --basic_dir {}'.format(ds, basic_dir))
    #     os.system('cd build && ./irb --dataset_name {} --basic_dir {}'.format(ds, basic_dir))
    #     os.system('cd build && ./birb {} {}'.format(ds, basic_dir))
    #     os.system('cd build && ./bpt {} {}'.format(ds, basic_dir))
    #
    #     os.system('cd build && ./irbfdp {} {}'.format(ds, basic_dir))
    #     os.system('cd build && ./irbfip {} {}'.format(ds, basic_dir))
    #     os.system('cd build && ./irbfnp {} {}'.format(ds, basic_dir))
    #     os.system('cd build && ./irbpdpip {} {}'.format(ds, basic_dir))
    #     os.system('cd build && ./irbpdpnp {} {}'.format(ds, basic_dir))
    #     os.system('cd build && ./irbpipnp {} {}'.format(ds, basic_dir))
