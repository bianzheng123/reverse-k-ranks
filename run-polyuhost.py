import os
import filecmp
import numpy as np
from script.data_convert import vecs_io


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


def compute_n_sample(dataset_name, memory_capacity):
    dataset_m = {'movielens-27m': [52889, 1000, 283228],
                 'netflix': [16770, 1000, 480189],
                 'yahoomusic_big': [135736, 1000, 1823179],
                 'yahoomusic': [97213, 1000, 1948882],
                 'yelp': [159585, 1000, 2189457],
                 'goodreads': [2359650, 1000, 876145]}
    size_element = 8
    disk_page = 4096
    n_element_per_block = disk_page / size_element
    n_data_item = dataset_m[dataset_name][0]
    n_user = dataset_m[dataset_name][2]
    require_n_sample = np.ceil(n_data_item / n_element_per_block)
    require_memory_capacity = require_n_sample * n_user * size_element / 1024 / 1024 / 1024

    n_sample = require_n_sample
    if require_memory_capacity > memory_capacity:
        print(
            CMDcolors.WARNING + "Warning: required memory capacity {}GB > the required memory capacity {}GB, change n_sample to 32".format(
                require_memory_capacity, memory_capacity) + CMDcolors.ENDC)
        n_sample = memory_capacity * 1024 * 1024 * 1024 / size_element / n_user
    return int(n_sample)


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


def run_build_index():
    dataset_l = ['movielens-27m', 'yahoomusic_big', 'yelp', 'goodreads']
    dataset_l = ['goodreads']
    index_size_l = [1536]
    for index_size in index_size_l:
        for ds in dataset_l:
            n_sample = compute_n_sample(ds, 32)
            # if ds == 'movielens-27m':
            #     os.system(
            #         'cd build/attribution && ./rsbist --dataset_name {} --basic_dir {} --n_sample {} --index_size_gb {}'.format(
            #             ds, basic_dir, n_sample, index_size))
            os.system(
                'cd build/attribution && ./rsbimt --dataset_name {} --basic_dir {} --n_sample {} --index_size_gb {}'.format(
                    ds, basic_dir, n_sample, index_size))


def run():
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'goodreads']
    # dataset_l = ['movielens-27m', 'yahoomusic_big', 'yelp', 'goodreads']
    dataset_l = ['netflix', 'movielens-27m']
    # dataset_l = ['yahoomusic_big', 'yelp', 'goodreads']

    for ds in dataset_l:
        os.system('cd build && ./bst --dataset_dir {} --dataset_name {} --index_dir {}'.format(
            dataset_dir, ds, index_dir))
        n_sample = compute_n_sample(ds, 16)
        os.system(
            'cd build && ./dbt --dataset_dir {} --dataset_name {} --index_dir {} --n_sample_item {} --sample_topk {}'.format(
                dataset_dir, ds, index_dir, 5000, 600
            ))
        os.system(
            'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
                dataset_dir, ds, index_dir, 'QueryRankSampleSearchKthRank', n_sample, 5000, 600))
        # os.system(
        #     'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --simpfer_k_max {}'.format(
        #         ds, basic_dir, "Simpfer", 1025))
        # os.system(
        #     'cd build && ./rri --dataset_name {} --basic_dir {} --method_name {} --n_sample {} --index_size_gb {}'.format(
        #         ds, basic_dir, "RSTopTIPRefineOrder", n_sample, index_size))
        # os.system('cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {}'.format(
        #     ds, 5000, 500,
        # ))
        # os.system(
        #     'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --n_sample {}'.format(
        #         dataset_dir, ds, index_dir, 'RankSample', n_sample))

    for ds in dataset_l:
        n_sample = compute_n_sample(ds, 16)
        os.system(
            'cd build && ./rri --dataset_dir {} --dataset_name {} --index_dir {} --method_name {} --n_sample {} --n_sample_query {} --sample_topk {}'.format(
                dataset_dir, ds, index_dir, 'QueryRankSampleSearchAllRank', n_sample, 5000, 600))


if __name__ == '__main__':
    dataset_dir = os.path.join('/run', 'media', 'hdd', 'ReverseMIPS')
    # basic_dir = os.path.join('/home', 'bianzheng', 'Dataset', 'ReverseMIPS')
    index_dir = os.path.join('/home', 'bianzheng', 'CLionProjects', 'reverse-k-ranks', 'index')
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    # dataset_l = ['netflix-small', 'movielens-27m-small']

    run()

    # os.system(
    #     'cd build/attribution && ./qdur --dataset_name {} --basic_dir {} --n_sample_item {}'.format(
    #         'yahoomusic_big', basic_dir, 5000))
    # os.system(
    #     'cd build/attribution && ./bfcs --dataset_name {} --basic_dir {} --n_sample_item {} --n_sample_rank {} --topk {}'.format(
    #         'yahoomusic_big', basic_dir, 5000, 1500, 30))

    # TODO run
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big', 'yelp', 'goodreads', 'amazon']
    # for ds in dataset_l:
    #     os.system('cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {}'.format(ds, 1000, 10))
    #     os.system(
    #         'cd build/attribution && ./ppr --dataset_name {} --n_sample_query {} --sample_topk {}'.format(
    #             ds, 1000, 10))

    # os.system('cd build && ./dbt --dataset_name {} --n_sample_item {} --sample_topk {}'.format('amazon', 5000, 50))
    # os.system('cd build && ./brrstt --dataset_name {}'.format('amazon'))
