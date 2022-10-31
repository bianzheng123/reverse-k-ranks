import os
import numpy as np


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


'''first is n_item, second is n_query, thrid is n_user'''
dataset_m = {'fake-normal': [5000, 100, 1000],
             'fake-uniform': [5000, 100, 1000],
             'fakebig': [5000, 100, 5000],
             'netflix-small': [5000, 100, 2000],

             'movielens-27m': [52889, 1000, 283228],
             'netflix': [16770, 1000, 480189],
             'yahoomusic_big': [135736, 1000, 1823179],
             'yelp': [159585, 1000, 2189457],
             'amazon-home-kitchen': [409243, 1000, 2511610]}


def count_n_sample_given_memory_capacity():
    for ds in dataset_m.keys():
        n_item = dataset_m[ds][0]
        n_user = dataset_m[ds][2]
        memory_capacity_l = [2, 4, 8, 16, 32, 64]
        sample_l = []
        for memory_capacity in memory_capacity_l:
            n_sample = int(np.floor(memory_capacity * 1024 * 1024 * 1024 / 8 / n_user))
            sample_l.append(n_sample)
        print("ds: {}".format(ds))
        print(sample_l)


def cmp_file(file1, file2):
    method1_result_l = np.loadtxt(file1, delimiter=',')
    method2_result_l = np.loadtxt(file2, delimiter=',')

    assert len(method1_result_l) == len(method2_result_l)

    for i in range(len(method1_result_l)):
        intersect = set(method1_result_l[i]).intersection(set(method2_result_l[i]))
        if len(intersect) != len(method1_result_l[i]):
            return False
    return True


def cmp_file_all(baseline_method, compare_method_l, dataset_l, topk_l):
    flag = True
    suffix_m = {

        'QRSTopTIP': 'n_sample_20-index_size_gb_50',
        'RSTopTIP': 'n_sample_20-index_size_gb_50',

        'QueryRankSampleIntLR': 'n_sample_20',
        'QueryRankSampleMinMaxIntLR': 'n_sample_20',
        'QueryRankSampleScoreDistribution': 'n_sample_20',
        'QueryRankSampleSearchAllRank': 'n_sample_20',
        'QueryRankSampleSearchKthRank': 'n_sample_20',
        'RankSample': 'n_sample_20',
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

        # 'GridIndex',
        'LinearModel',
        'QueryRankSampleIntLR',
        'QueryRankSampleMinMaxIntLR',
        'QueryRankSampleScoreDistribution',
        'QueryRankSampleSearchAllRank',
        'QueryRankSampleSearchKthRank',
        'RankSample',
    ]

    os.system('cd result/rank && rm *')
    os.system('cd result/single_query_performance && rm *')
    os.system('cd result/vis_performance && rm *')

    # os.system('cd build && ./{} --dataset_name {}'.format('rb', ds))
    # os.system('cd build && ./{} {}'.format('bbfdi', ds))

    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    for ds in dataset_l:
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'BatchDiskBruteForce'))
        # os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'DiskBruteForce'))
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'MemoryBruteForce'))

        # os.system('cd build && ./bst --dataset_name {}'.format(ds))

        # os.system('cd build && ./rri --dataset_name {} --test_topk {} --method_name {}'.format(ds, 'true', 'GridIndex'))
        os.system('cd build && ./rri --dataset_name {} --test_topk {} --method_name {}'.format(ds, 'true', 'LinearModel'))

        n_sample_item = 150
        sample_topk = 60
        n_data_item = dataset_m[ds][0]
        n_user = dataset_m[ds][2]
        n_sample = 20
        os.system(
            "cd build && ./qdi --dataset_name {} --n_sample_item {} --sample_topk {}".format(
                ds, n_sample_item, sample_topk
            ))
        os.system(
            "cd build && ./fsr --dataset_name {} --n_data_item {} --n_user {} --n_sample {} --n_sample_query {} --sample_topk {}".format(
                ds, n_data_item, n_user, n_sample, n_sample_item, sample_topk
            ))
        os.system(
            "cd build && ./bqrsi --dataset_name {} --n_sample {} --n_sample_query {} --sample_topk {}".format(
                ds, n_sample, n_sample_item, sample_topk
            ))
        os.system(
            "cd build && ./rri --dataset_name {} --test_topk {} --method_name {} --n_sample_query {} --sample_topk {}".format(
                ds, 'true', 'QueryRankSampleSearchKthRank', n_sample_item, sample_topk
            ))
        os.system(
            "cd build && ./rri --dataset_name {} --test_topk {} --method_name {} --n_sample_query {} --sample_topk {}".format(
                ds, 'true', 'QueryRankSampleSearchAllRank', n_sample_item, sample_topk
            ))

        os.system(
            'cd build && ./rri --dataset_name {} --test_topk {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
                ds, 'true', 'QueryRankSampleIntLR', n_sample_item, sample_topk))
        os.system(
            'cd build && ./rri --dataset_name {} --test_topk {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
                ds, 'true', 'QueryRankSampleMinMaxIntLR', n_sample_item, sample_topk))
        os.system(
            'cd build && ./rri --dataset_name {} --test_topk {} --method_name {} --n_sample_query {} --sample_topk {}'.format(
                ds, 'true', 'QueryRankSampleScoreDistribution', 150, 60))

        os.system('cd build && ./brsi --dataset_name {} --n_sample {}'.format(ds, 20))
        os.system(
            'cd build && ./rri --dataset_name {} --test_topk {} --method_name {}'.format(ds, 'true', 'RankSample'))
    # send_email.send('test complete')

    # topk_l = [10, 20, 30, 40, 50]
    topk_l = [10, 20, 30]
    cmp_file_all('BatchDiskBruteForce', method_name_l, dataset_l, topk_l)


if __name__ == '__main__':
    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    # dataset_l = ['fake-normal', 'fake-uniform']
    dataset_l = ['fake-normal']
    # dataset_l = ['fake-normal-query-distribution', 'fake-uniform-query-distribution',
    #              'netflix-small-query-distribution', 'movielens-27m-small-query-distribution']

    # for ds in dataset_l:
    #     for bound_name in ['Grid', 'FullDim', 'FullNorm', 'FullInt', 'PartDimPartInt', 'PartDimPartNorm',
    #                        'PartIntPartNorm']:
    #         os.system(
    #             'cd build/attribution && ./biipb --dataset_name {} --bound_name {}'.format(ds, bound_name))

    run()
