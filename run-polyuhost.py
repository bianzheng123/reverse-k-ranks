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


def run_interval_sample_rate():
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # n_interval_l = [16, 32, 64, 128, 256, 512, 1024, 2048]
    n_interval_l = [32, 64, 128, 256, 512]
    for ds in dataset_l:
        for n_interval in n_interval_l:
            os.system(
                'cd build && ./rri --method_name {} --dataset_name {} --basic_dir {} --n_interval {}'.format(
                    'IntervalBound', ds, basic_dir, n_interval))


def run_compress_topt():
    # dataset_l = ['netflix', 'yahoomusic-small', 'yelp-small']
    topt_perc_l = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
    for ds in dataset_l:
        for topt_perc in topt_perc_l:
            os.system(
                'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --topt_perc {}'.format(
                    ds, basic_dir, "HRBMergeRankBound", topt_perc))


def run_compute_all_scale():
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    scale_l = [25, 50, 100, 200, 400, 800, 1000]
    for ds in dataset_l:
        for scale in scale_l:
            os.system(
                'cd build && ./ca --bound_name {} --dataset_name {} --basic_dir {} --scale {}'.format(
                    'CAPartIntPartNorm', ds, basic_dir, scale))


def run_compute_all_n_codebook():
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    n_codebook_l = [2, 4, 8, 16]
    n_codeword = 128
    for ds in dataset_l:
        for n_codebook in n_codebook_l:
            os.system(
                'cd build && ./ca --bound_name {} --dataset_name {} --basic_dir {} --n_codebook {} --n_codeword {}'.format(
                    'CAUserItemPQ', ds, basic_dir, n_codebook, n_codeword))
            os.system(
                'cd build && ./ca --bound_name {} --dataset_name {} --basic_dir {} --n_codebook {} --n_codeword {}'.format(
                    'CAItemPQ', ds, basic_dir, n_codebook, n_codeword))


def run_compute_all_n_codeword():
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    n_codebook = 4
    n_codeword_l = [16, 32, 64, 128, 256, 512, 1024]
    for ds in dataset_l:
        for n_codeword in n_codeword_l:
            os.system(
                'cd build && ./ca --bound_name {} --dataset_name {} --basic_dir {} --n_codebook {} --n_codeword {}'.format(
                    'CAUserItemPQ', ds, basic_dir, n_codebook, n_codeword))
            os.system(
                'cd build && ./ca --bound_name {} --dataset_name {} --basic_dir {} --n_codebook {} --n_codeword {}'.format(
                    'CAItemPQ', ds, basic_dir, n_codebook, n_codeword))


if __name__ == '__main__':
    basic_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    # basic_dir = os.path.join('/home', 'bianzheng', 'Dataset', 'ReverseMIPS')
    dataset_l = ['movielens-27m-extreme', 'movielens-27m', 'netflix', 'yelp-small', 'yahoomusic-small']
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic-small', 'yelp-small']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    # run_compute_all_scale()
    # run_compute_all_n_codebook()
    # run_compute_all_n_codeword()

    n_data_item_m = {
        'movielens-27m-extreme': 53889,
        'movielens-27m': 53889,
        'netflix': 17770,
        'yelp-small': 50000,
        'yahoomusic-small': 50000
    }

    for ds in dataset_l:
        n_rankbound = 512
        rankbound_cmd = 'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --cache_bound_every {}'.format(
            ds, basic_dir, 'RankBound', n_rankbound
        )
        print(rankbound_cmd)
        os.system(rankbound_cmd)

        n_sample = int(n_data_item_m[ds] / n_rankbound)

        intervalbound_cmd = 'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {}'.format(
            ds, basic_dir, 'IntervalBound', n_sample
        )
        print(intervalbound_cmd)
        os.system(intervalbound_cmd)

        quadraticrankbound_cmd = 'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {}'.format(
            ds, basic_dir, 'QuadraticRankBound', n_sample
        )
        print(quadraticrankbound_cmd)
        os.system(quadraticrankbound_cmd)

        quadraticscorebound_cmd = 'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {}'.format(
            ds, basic_dir, 'QuadraticScoreBound', n_sample
        )
        print(quadraticscorebound_cmd)
        os.system(quadraticscorebound_cmd)
