import os


def run_attribution():
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['movielens-27m']
    for ds in dataset_l:
        os.system('cd build/attribution && ./ipc {}'.format(ds))

    os.system('cd build/attribution && ./pi')

    os.system('cd build/attribution && ./pp')

    for ds in dataset_l:
        os.system('cd build/attribution && ./dvd {}'.format(ds))
        os.system('cd attribution/svd-compare && python3 plot_curve.py -ds {}'.format(ds))

        os.system('cd build/attribution && ./svdcmp {}'.format(ds))


def run(method_name='IntervalRankBound', program_name='irb'):
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['fake']
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['movielens-small', 'movielens-1m']
    method_m = {
        # 'OnlineBruteForce': 'bfon',
        'MemoryBruteForce': 'bfmi',
        # 'DiskBruteForce': 'bfdi',
        'BatchDiskBruteForce': 'bbfdi',
        # 'IntervalRankBound': 'irb'
    }
    for ds in dataset_l:
        os.system('cd build && ./{} --dataset_name {}'.format(program_name, ds))
        for method in method_m:
            os.system('cd build && ./{} {}'.format(method_m[method], ds))

    type_arr = ['index', 'IP', 'rank']
    topk_l = [10, 20, 30, 40, 50]

    for ds in dataset_l:
        for topk in topk_l:
            for method in method_m:
                for _type in type_arr:
                    # bfon = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'OnlineBruteForce', _type))
                    base_method = os.path.join('result', 'rank', '{}-{}-top{}-{}.csv'.format(ds, method, topk, _type))
                    test_method = os.path.join('result', 'rank',
                                               '{}-{}-top{}-{}.csv'.format(ds, method_name, topk, _type))

                    cmd = "diff {} {}".format(base_method, test_method)
                    print(cmd)
                    os.system(cmd)


def run_check_baseline(compare_method=['IntervalRankBound'], compare_program=['irb']):
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['movielens-small', 'movielens-1m']
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    method_m = {
        # 'OnlineBruteForce': 'bfon',
        'MemoryBruteForce': 'bfmi',
        'DiskBruteForce': 'bfdi',
        'BatchDiskBruteForce': 'bbfdi',
        # 'IntervalRankBound': 'irb'
    }

    for ds in dataset_l:
        for i in range(len(compare_method)):
            os.system('cd build && ./{} {}'.format(compare_program[i], ds))

    type_arr = ['index', 'IP', 'rank']
    topk_l = [10, 20, 30, 40, 50]

    for ds in dataset_l:
        for topk in topk_l:
            for _type in type_arr:
                for i in range(len(compare_method)):
                    # bfon = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'OnlineBruteForce', _type))
                    base_method = os.path.join('result', 'rank',
                                               '{}-{}-top{}-{}.csv'.format(ds, 'BatchDiskBruteForce', topk, _type))
                    test_method = os.path.join('result', 'rank',
                                               '{}-{}-top{}-{}.csv'.format(ds, compare_method[i], topk, _type))

                    cmd = "diff {} {}".format(base_method, test_method)
                    print(cmd)
                    os.system(cmd)


def run_rankbound_sample_rate():
    dataset_name_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    cache_bound_every_l = [20]
    for ds in dataset_name_l:
        for para in cache_bound_every_l:
            os.system('cd build && ./rb --dataset_name {} --cache_bound_every {}'.format(ds, para))

    for ds in dataset_name_l:
        for topk in [10, 20, 30, 40, 50]:
            for i in range(len(cache_bound_every_l)):
                if i != 0:
                    file1 = "{}-RankBound-top{}-cache_bound_every_{}-index.csv".format(ds, topk, cache_bound_every_l[0])
                    file2 = "{}-RankBound-top{}-cache_bound_every_{}-index.csv".format(ds, topk, cache_bound_every_l[i])
                    cmd = "diff {} {}".format(file1, file2)
                    print(cmd)
                    os.system("cd result/rank && {}".format(cmd))

                    file1 = "{}-RankBound-top{}-cache_bound_every_{}-IP.csv".format(ds, topk, cache_bound_every_l[0])
                    file2 = "{}-RankBound-top{}-cache_bound_every_{}-IP.csv".format(ds, topk, cache_bound_every_l[i])
                    cmd = "diff {} {}".format(file1, file2)
                    print(cmd)
                    os.system("cd result/rank && {}".format(cmd))

                    file1 = "{}-RankBound-top{}-cache_bound_every_{}-rank.csv".format(ds, topk, cache_bound_every_l[0])
                    file2 = "{}-RankBound-top{}-cache_bound_every_{}-rank.csv".format(ds, topk, cache_bound_every_l[i])
                    cmd = "diff {} {}".format(file1, file2)
                    print(cmd)
                    os.system("cd result/rank && {}".format(cmd))


if __name__ == '__main__':
    run(method_name='RankBound', program_name='rb')
    # run(method_name='IntervalRankBound', program_name='irb')
    # run_check_baseline()
    # run_check_baseline(
    #     compare_method=['IRBFullDim', 'IRBFullInt', 'IRBFullNorm', 'IRBPartDimPartInt', 'IRBPartDimPartNorm',
    #                     'IRBPartIntPartNorm'],
    #     compare_program=['irbfd', 'irbfi', 'irbfn', 'irbpdpi', 'irbpdpn', 'irbpipn'])
    # run_rankbound_sample_rate()
