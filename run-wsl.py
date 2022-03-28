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


def gen_data(n_user, n_data_item, n_query_item, vec_dim, dataset):
    query_item_l = np.random.normal(scale=0.1, size=(n_query_item, vec_dim))
    data_item_l = np.random.normal(scale=0.1, size=(n_data_item, vec_dim))
    user_l = np.random.normal(scale=0.1, size=(n_user, vec_dim))

    output_dir = '/home/bianzheng/Dataset/ReverseMIPS/%s' % dataset
    delete_file_if_exist(output_dir)
    os.mkdir(output_dir)

    vecs_io.dvecs_write("%s/%s_query_item.dvecs" % (output_dir, dataset), query_item_l)
    vecs_io.dvecs_write("%s/%s_data_item.dvecs" % (output_dir, dataset), data_item_l)
    vecs_io.dvecs_write("%s/%s_user.dvecs" % (output_dir, dataset), user_l)


def debug_run():
    while True:
        n_user = 10000
        n_data_item = 1000
        n_query_item = 100
        vec_dim = 50
        dataset = 'fakebig'
        gen_data(n_user, n_data_item, n_query_item, vec_dim, dataset)
        cache_bound_every_l = [20, 80, 320, 800, 900]
        for n_bound in cache_bound_every_l:
            method = 'rb'
            os.system(
                'cd build && ./{} --dataset_name {} --cache_bound_every {} > {}-{}.log'.format(method, dataset, n_bound,
                                                                                               method, n_bound))

        method = 'bbfdi'
        os.system('cd build && ./{} {} > {}.log'.format(method, dataset, method))

        topk_l = [10, 20, 30, 40, 50]
        type_arr = ['index', 'IP', 'rank']
        flag = True
        for n_bound in cache_bound_every_l:
            for topk in topk_l:
                for _type in type_arr:
                    # bfon = os.path.join('result', 'rank', '{}-{}-top10-{}.csv'.format(ds, 'OnlineBruteForce', _type))
                    base_method = os.path.join('result', 'rank',
                                               '{}-{}-top{}-{}.csv'.format(dataset, 'BatchDiskBruteForce', topk, _type))
                    test_method = os.path.join('result', 'rank',
                                               '{}-RankBound-top{}-cache_bound_every_{}-{}.csv'.format(dataset, topk,
                                                                                                       n_bound, _type))

                    flag = cmp_file(base_method, test_method)
                    flag = False
                    if not flag:
                        print("file diff {} {}".format(base_method, test_method))
                        exit(-1)
        if flag:
            print("no error, no bug")


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
            os.system('cd build && ./{} --dataset_name {}'.format(program_name, ds))
        else:
            os.system('cd build && ./{} {}'.format(program_name, ds))

        for method in method_m:
            os.system('cd build && ./{} {}'.format(method_m[method], ds))

    type_arr = ['index', 'IP', 'rank']
    topk_l = [10, 20, 30, 40, 50, 60, 70]
    # topk_l = [283228]

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
    cache_bound_every_l = [20, 40, 60, 80]
    for ds in dataset_l:
        for para in cache_bound_every_l:
            os.system('cd build && ./rb --dataset_name {} --cache_bound_every {}'.format(ds, para))

    flag = True
    for ds in dataset_l:
        for topk in [10, 20, 30, 40, 50]:
            for i in range(len(cache_bound_every_l)):
                if i != 0:
                    file1 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-index.csv".format(ds, topk,
                                                                                                   cache_bound_every_l[
                                                                                                       0])
                    file2 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-index.csv".format(ds, topk,
                                                                                                   cache_bound_every_l[
                                                                                                       i])
                    flag = cmp_file(file1, file2)
                    if not flag:
                        print("file diff {} {}".format(file1, file2))

                    file1 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-IP.csv".format(ds, topk,
                                                                                                cache_bound_every_l[0])
                    file2 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-IP.csv".format(ds, topk,
                                                                                                cache_bound_every_l[i])
                    flag = cmp_file(file1, file2)
                    if not flag:
                        print("file diff {} {}".format(file1, file2))

                    file1 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-rank.csv".format(ds, topk,
                                                                                                  cache_bound_every_l[
                                                                                                      0])
                    file2 = "result/rank/{}-RankBound-top{}-cache_bound_every_{}-rank.csv".format(ds, topk,
                                                                                                  cache_bound_every_l[
                                                                                                      i])
                    flag = cmp_file(file1, file2)
                    if not flag:
                        print("file diff {} {}".format(file1, file2))
    if flag:
        print("no error, no bug")


if __name__ == '__main__':
    dataset_l = ['fake', 'fakebig', 'movielens-small', 'movielens-1m']
    run(method_name='BallIntervalRankBound', program_name='birb')
    # run(method_name='IntervalRankBound', program_name='irb')
    # run(method_name='RankBound', program_name='rb')


    # run(method_name='IRBFullDimPrune', program_name='irbfdp')
    # run(method_name='IRBFullIntPrune', program_name='irbfip')
    # run(method_name='IRBFullNormPrune', program_name='irbfnp')
    # run(method_name='IRBPartDimPartIntPrune', program_name='irbpdpip')
    # run(method_name='IRBPartDimPartNormPrune', program_name='irbpdpnp')
    # run(method_name='IRBPartIntPartNormPrune', program_name='irbpipnp')

    # debug_run()
    # run_rankbound_sample_rate()
    # run(method_name='IntervalRankBound', program_name='irb')
