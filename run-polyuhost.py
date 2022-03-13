import os


def run_attribution():
    dataset_l = ['fake', 'movielens-27m']
    for ds in dataset_l:
        os.system('cd build/attribution && ./bd {} /run/media/hdd/ReverseMIPS'.format(ds))
        os.system('cd attribution/bound-distribution && python3 plot.py -ds {}'.format(ds))


def run():
    dataset_l = ['movielens-27m']
    method_m = {
        'DiskBruteForce': 'bfdi',
        'MemoryBruteForce': 'bfmi',
        'BinarySearchCacheBound': 'bscb'
    }
    for ds in dataset_l:
        os.system('cd build && ./{} {} /run/media/hdd/ReverseMIPS'.format(method_m['DiskBruteForce'], ds))
        os.system('cd build && ./{} {} /run/media/hdd/ReverseMIPS'.format(method_m['BinarySearchCacheBound'], ds))

    type_arr = ['index', 'IP', 'rank']

    for ds in dataset_l:
        for _type in type_arr:
            bscb = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'BinarySearchCacheBound', _type))
            dbf = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'DiskBruteForce', _type))
            cmd = "diff {} {}".format(bscb, dbf)
            print(cmd)
            os.system(cmd)


if __name__ == '__main__':
    run_attribution()