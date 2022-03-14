import os


def run_attribution():
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['movielens-27m']
    for ds in dataset_l:
        os.system('cd build/attribution && ./bd {}'.format(ds))
        os.system('cd attribution/bound-distribution && python3 plot.py -ds {}'.format(ds))


def run():
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['fake', 'fakebig', 'movielens-small']
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['movielens-small', 'movielens-1m']
    method_m = {
        # 'OnlineBruteForce': 'bfon',
        'MemoryBruteForce': 'bfmi',
        'DiskBruteForce': 'bfdi',
        'RankBound': 'rb',
        'BatchDiskBruteForce': 'bbfdi',
        # 'IntervalRankBound': 'irb'
    }
    for ds in dataset_l:
        for method in method_m:
            # os.system('cd build && ./bfon %s' % ds)
            os.system('cd build && ./{} {}'.format(method_m[method], ds))

    type_arr = ['index', 'IP', 'rank']
    type_arr = ['index', 'rank']

    for ds in dataset_l:
        for _type in type_arr:
            # bfon = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'OnlineBruteForce', _type))
            bfmi = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'MemoryBruteForce', _type))
            bfdi = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'DiskBruteForce', _type))
            rb = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'RankBound', _type))
            bbfdi = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'BatchDiskBruteForce', _type))
            # cmd = "diff {} {}".format(bfon, bfmi)
            # print(cmd)
            # os.system(cmd)

            cmd = "diff {} {}".format(bfmi, bfdi)
            print(cmd)
            os.system(cmd)

            cmd = "diff {} {}".format(bfmi, rb)
            print(cmd)
            os.system(cmd)

            cmd = "diff {} {}".format(bfmi, bbfdi)
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
    run_bound_selection()
    # run_rankbound_sample_rate()
