import os


def run_attribution():
    # dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    dataset_l = ['movielens-27m']
    for ds in dataset_l:
        os.system('cd build/attribution && ./bd {}'.format(ds))
        os.system('cd attribution/bound-distribution && python3 plot.py -ds {}'.format(ds))


def run():
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    # dataset_l = ['movielens-small', 'movielens-1m']
    method_m = {
        'DiskBruteForce': 'bfdi',
        'MemoryBruteForce': 'bfmi',
        'BinarySearchCacheBound': 'bscb'
    }
    for ds in dataset_l:
        # os.system('cd build && ./bfon %s' % ds)
        os.system('cd build && ./{} {}'.format(method_m['DiskBruteForce'], ds))
        os.system('cd build && ./{} {}'.format(method_m['MemoryBruteForce'], ds))
        # os.system('cd build && ./bbfdi %s' % ds)
        os.system('cd build && ./{} {}'.format(method_m['BinarySearchCacheBound'], ds))

    type_arr = ['index', 'IP', 'rank']

    for ds in dataset_l:
        for _type in type_arr:
            bscb = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'BinarySearchCacheBound', _type))
            dbf = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'DiskBruteForce', _type))
            mbf = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'MemoryBruteForce', _type))
            cmd = "diff {} {}".format(bscb, dbf)
            print(cmd)
            os.system(cmd)

            cmd = "diff {} {}".format(bscb, mbf)
            print(cmd)
            os.system(cmd)


if __name__ == '__main__':
    run_attribution()
