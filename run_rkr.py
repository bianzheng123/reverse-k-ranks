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
             'fake-normal-tiny': [50, 30, 200],
             'fake-uniform-tiny': [50, 30, 200],

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
            print(f"line {i}")
            return False
    return True


def cmp_file_all(baseline_method, compare_method_l, dataset_l, topk_l):
    flag = True
    suffix_m = {
        'QueryRankSampleComputeAll': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleDirectIntLR': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleGlobalIntLR': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleMinMaxIntLR': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleUniformIntLR': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleMinMaxIntLREstimate': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleScoreDistribution': 'n_sample_20-n_bit_8',
        'QueryRankSampleSearchAllRank': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleSearchBruteForce': 'n_sample_5',
        'QueryRankSampleSearchKthRank': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleSearchUniformRank': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleUniformIntLREstimate': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleSearchUniformRankMinMaxIntLR': 'n_sample_20-n_sample_query_150-sample_topk_40',
        'QueryRankSampleSearchUniformRankUniformIntLR': 'n_sample_20-n_sample_query_150-sample_topk_40',
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


def run_sample_method(method_name, dataset_name, n_sample, n_data_item, n_user, n_sample_item, sample_topk,
                      other_config=""):
    os.system(
        f"cd build && ./fsr --index_dir {index_dir} --dataset_name {dataset_name} --method_name {method_name} --n_sample {n_sample} --n_data_item {n_data_item} --n_user {n_user} --n_sample_query {n_sample_item} --sample_topk {sample_topk}"
    )
    os.system(
        f"cd build && ./bsibs --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")
    # os.system(
    #     f"cd build && ./bsibc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    if method_name == 'QueryRankSampleMinMaxIntLR' or \
            method_name == "QueryRankSampleDirectIntLR" or method_name == "QueryRankSampleGlobalIntLR" or \
            method_name == "QueryRankSampleUniformIntLR" or method_name == "QueryRankSampleSearchUniformRankMinMaxIntLR" or \
            method_name == "QueryRankSampleSearchUniformRankUniformIntLR":
        os.system(
            f"cd build && ./bilrbc --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk}")

    if method_name == "QueryRankSampleSearchBruteForce":
        os.system(
            f"cd build && ./progress --dataset_name {dataset_name} --index_dir {index_dir} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk} {other_config}"
        )
    else:
        os.system(
            f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {dataset_name} --index_dir {index_dir} --test_topk {'true'} --method_name {method_name} --n_sample {n_sample} --n_sample_query {n_sample_item} --sample_topk {sample_topk} {other_config}"
        )


def run():
    method_name_l = [
        'BatchDiskBruteForce',
        # 'DiskBruteForce',
        'MemoryBruteForce',

        # 'GridIndex',
        'QueryRankSampleComputeAll'

        # 'QueryRankSampleDirectIntLR',
        # 'QueryRankSampleGlobalIntLR',
        # 'QueryRankSampleMinMaxIntLR',
        # 'QueryRankSampleUniformIntLR',
        # 'QueryRankSampleMinMaxIntLREstimate',

        # 'QueryRankSampleScoreDistribution',
        # 'QueryRankSampleSearchAllRank',
        # 'QueryRankSampleSearchBruteForce',
        # 'QueryRankSampleSearchKthRank',
        # 'QueryRankSampleSearchUniformRank',

        # 'QueryRankSampleSearchUniformRankMinMaxIntLR',
        # 'QueryRankSampleSearchUniformRankUniformIntLR',
        # 'QueryRankSampleUniformIntLREstimate',

        # 'RankSample',
        # 'RTree',
    ]

    os.system('cd result/rank && rm *')
    os.system('cd result/single_query_performance && rm *')
    os.system('cd result/vis_performance && rm *')
    os.system('cd index/memory_index && rm *')

    # os.system('cd build && ./{} --dataset_name {}'.format('rb', ds))
    # os.system('cd build && ./{} {}'.format('bbfdi', ds))

    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    for ds in dataset_l:
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'BatchDiskBruteForce'))
        # os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'DiskBruteForce'))
        os.system('cd build && ./progress --dataset_name {} --method_name {}'.format(ds, 'MemoryBruteForce'))

        os.system('cd build && ./bst --dataset_name {}'.format(ds))

        # os.system('cd build && ./rri --dataset_name {} --test_topk {} --method_name {} --stop_time 36000'.format(ds, 'true', 'GridIndex'))

        n_sample_item = 150
        sample_topk = 40
        n_data_item = dataset_m[ds][0]
        n_user = dataset_m[ds][2]
        n_sample = 20
        os.system(
            "cd build && ./qdi --index_dir {} --dataset_dir {} --dataset_name {} --n_sample_item {} --sample_topk {}".format(
                index_dir, dataset_dir, ds, n_sample_item, sample_topk
            ))

        run_sample_method('QueryRankSampleComputeAll', ds, n_sample, n_data_item, n_user, n_sample_item,
                          sample_topk)

        # run_sample_method('QueryRankSampleDirectIntLR', ds, n_sample, n_data_item, n_user, n_sample_item,
        #                   sample_topk)
        # run_sample_method('QueryRankSampleGlobalIntLR', ds, n_sample, n_data_item, n_user, n_sample_item,
        #                   sample_topk)
        # run_sample_method('QueryRankSampleMinMaxIntLR', ds, n_sample, n_data_item, n_user, n_sample_item,
        #                   sample_topk)
        # os.system(
        #     f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_dir} --test_topk {'true'} "
        #     f"--method_name {'QueryRankSampleMinMaxIntLREstimate'} --n_sample {n_sample} "
        #     f"--n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        # )
        # run_sample_method('QueryRankSampleUniformIntLR', ds, n_sample, n_data_item, n_user, n_sample_item, sample_topk)
        # os.system(
        #     f"cd build && ./rri --dataset_dir {dataset_dir} --dataset_name {ds} --index_dir {index_dir} --test_topk {'true'} "
        #     f"--method_name {'QueryRankSampleUniformIntLREstimate'} --n_sample {n_sample} "
        #     f"--n_sample_query {n_sample_item} --sample_topk {sample_topk}"
        # )

        # run_sample_method('QueryRankSampleScoreDistribution', ds, n_sample, n_data_item, n_user, n_sample_item,
        #                   sample_topk)
        # run_sample_method('QueryRankSampleSearchAllRank', ds, n_sample, n_data_item, n_user, n_sample_item, sample_topk)
        # run_sample_method('QueryRankSampleSearchBruteForce', ds, n_sample, n_data_item, n_user, n_sample_item,
        #                   sample_topk)
        # run_sample_method('QueryRankSampleSearchKthRank', ds, n_sample, n_data_item, n_user, n_sample_item, sample_topk)
        # run_sample_method('QueryRankSampleSearchUniformRank', ds, n_sample, n_data_item, n_user, n_sample_item, sample_topk)

        # run_sample_method('QueryRankSampleSearchUniformRankMinMaxIntLR', ds, n_sample, n_data_item, n_user,
        #                   n_sample_item, sample_topk)
        # run_sample_method('QueryRankSampleSearchUniformRankUniformIntLR', ds, n_sample, n_data_item, n_user,
        #                   n_sample_item, sample_topk)

        # run_sample_method('RankSample', ds, n_sample, n_data_item, n_user, n_sample_item, sample_topk)
        # os.system(
        #     'cd build && ./rri --dataset_name {} --test_topk {} --method_name {} --stop_time 36000'.format(ds, 'true',
        #                                                                                                    'RTree'))

    # send_email.send('test complete')

    # topk_l = [10, 20, 30, 40, 50]
    topk_l = [10, 20, 30]
    cmp_file_all('BatchDiskBruteForce', method_name_l, dataset_l, topk_l)


def cmp_file_count(file1, file2):
    method1_result_l = np.loadtxt(file1, delimiter=',')
    method2_result_l = np.loadtxt(file2, delimiter=',')

    assert len(method1_result_l) == len(method2_result_l)
    count = 0

    for i in range(len(method1_result_l)):
        intersect = set(method1_result_l[i]).intersection(set(method2_result_l[i]))
        if len(intersect) != len(method1_result_l[i]):
            count += 1
    return count


if __name__ == '__main__':
    index_dir = "/home/bianzheng/reverse-k-ranks/index"
    dataset_dir = "/home/bianzheng/Dataset/ReverseMIPS"
    # dataset_l = ['fake-normal-tiny', 'fake-uniform-tiny']
    # dataset_l = ['fake-uniform-tiny']
    # dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    dataset_l = ['fake-normal', 'fake-uniform', 'fakebig', 'netflix-small']
    # dataset_l = ['fake-normal-query-distribution', 'fake-uniform-query-distribution',
    #              'netflix-small-query-distribution', 'movielens-27m-small-query-distribution']

    run()
