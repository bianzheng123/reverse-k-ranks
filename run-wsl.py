import os


def run_attribution():
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['movielens-27m']
    for ds in dataset_l:
        os.system('cd build/attribution && ./bd {}'.format(ds))
        os.system('cd attribution/bound-distribution && python3 plot.py -ds {}'.format(ds))


def run(method_name='IntervalRankBound', program_name='irb'):
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['fake']
    dataset_l = ['movielens-1m']
    # dataset_l = ['movielens-small', 'movielens-1m']
    method_m = {
        # 'OnlineBruteForce': 'bfon',
        'MemoryBruteForce': 'bfmi',
        'DiskBruteForce': 'bfdi',
        'BatchDiskBruteForce': 'bbfdi',
        # 'IntervalRankBound': 'irb'
    }
    for ds in dataset_l:
        os.system('cd build && ./{} {}'.format(program_name, ds))
        # for method in method_m:
        #     os.system('cd build && ./bfon %s' % ds)
        # os.system('cd build && ./{} {}'.format(method_m[method], ds))

    type_arr = ['index', 'IP', 'rank']

    for ds in dataset_l:
        for method in method_m:
            for _type in type_arr:
                # bfon = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'OnlineBruteForce', _type))
                base_method = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, method, _type))
                test_method = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, method_name, _type))

                cmd = "diff {} {}".format(base_method, test_method)
                print(cmd)
                os.system(cmd)


def run_bound_selection():
    arr = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    for ds in arr:
        os.system("cd build/attribution && ./bs %s" % ds)


def run_rankbound_sample_rate():
    dataset_name_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    cache_bound_every_l = [10, 20, 40, 80, 160]
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
    test_method_name_l = ['IRBFullDim', 'IRBFullInt', 'IRBFullNorm', 'IRBPartDimPartInt', 'IRBPartDimPartNorm',
                          'IRBPartIntPartNorm']
    test_program_name_l = ['irbfd', 'irbfi', 'irbfn', 'irbpdpi', 'irbpdpn', 'irbpipn']
    assert len(test_method_name_l) == len(test_program_name_l)
    for i in range(len(test_method_name_l)):
        run(method_name=test_method_name_l[i], program_name=test_program_name_l[i])
    # run_rankbound_sample_rate()
