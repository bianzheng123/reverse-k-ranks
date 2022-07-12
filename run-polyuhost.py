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


def run_compress_topt():
    dataset_l = ['yahoomusic_big', 'yelp', 'amazon']
    index_size_l = [128, 256, 512, 1024]
    n_sample = 128
    for ds in dataset_l:
        # os.system(
        #     'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {}'.format(
        #         ds, basic_dir, "SSComputeAll", n_sample))
        # os.system(
        #     'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {}'.format(
        #         ds, basic_dir, "ScoreSample", n_sample))
        for index_size in index_size_l:
            compress_method_l = ['CompressTopTIDBruteForce', 'CompressTopTIDIPBruteForce', 'CompressTopTIPBruteForce',
                                 'SSMergeIntervalIDByBitmap', 'SSMergeIntervalIDByInterval',
                                 'SSMergeQuadraticRankBoundByBitmap',
                                 'SSMergeRankByBitmap', 'SSMergeRankByInterval']
            os.system(
                'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {} --index_size_gb {}'.format(
                    ds, basic_dir, "CompressTopTIDBruteForce", n_sample, index_size))
            os.system(
                'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {} --index_size_gb {}'.format(
                    ds, basic_dir, "CompressTopTIDIPBruteForce", n_sample, index_size))
            os.system(
                'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {} --index_size_gb {}'.format(
                    ds, basic_dir, "CompressTopTIPBruteForce", n_sample, index_size))
            os.system(
                'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {} --index_size_gb {}'.format(
                    ds, basic_dir, "SSMergeQuadraticRankBoundByBitmap", n_sample, index_size))
            os.system(
                'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {} --index_size_gb {}'.format(
                    ds, basic_dir, "SSMergeRankByInterval", n_sample, index_size))


if __name__ == '__main__':
    basic_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    # basic_dir = os.path.join('/home', 'bianzheng', 'Dataset', 'ReverseMIPS')
    # dataset_l = ['movielens-27m', 'netflix']
    # dataset_l = ['fake-normal']
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run_compress_topt()

    # for n_sample in [64, 128, 256, 512]:
    #     for ds in ['goodreads', 'amazon', 'book-crossing']:
    #         os.system(
    #             'cd build/attribution && ./mpr --dataset_name {} --n_sample {} --basic_dir {}'.format(
    #                 ds, n_sample, basic_dir))
    #
    # for ds in ['movielens-small', 'movielens-27m', 'netflix', 'yahoomusic_big', 'yelp']:
    #     for topk in [10, 50]:
    #         os.system(
    #             'cd build/attribution && ./udd {} {}'.format(ds, topk))
