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


def run_program():
    test_method_name_l = ['IRBFullDim', 'IRBFullInt', 'IRBFullNorm', 'IRBPartDimPartInt', 'IRBPartDimPartNorm',
                          'IRBPartIntPartNorm']
    test_program_name_l = ['irbfd', 'irbfi', 'irbfn', 'irbpdpi', 'irbpdpn', 'irbpipn']
    dataset_l = ['movielens-27m']
    for ds in dataset_l:
        for i in range(len(test_method_name_l)):
            os.system('cd build && ./{} {} /run/media/hdd/ReverseMIPS'.format(test_program_name_l[i], ds))


def run_bound_selection():
    arr = ['movielens-small', 'movielens-27m']
    for ds in arr:
        os.system("cd build/attribution && ./bs %s /run/media/hdd/ReverseMIPS" % ds)


def run_rankbound_sample_rate():
    dataset_name_l = ['movielens-27m']
    cache_bound_every_l = [32, 64, 128, 256, 512, 1024, 2048, 4096, 8192]
    for ds in dataset_name_l:
        for para in cache_bound_every_l:
            os.system('cd build && ./rb --dataset_name {} --cache_bound_every {}'.format(ds, para))


if __name__ == '__main__':
    run_program()
