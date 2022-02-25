import os

method_arr = ['BatchDiskBruteForce', 'DiskBruteForce', 'MemoryBruteForce']
dataset_arr = ['movielens-1m', 'movielens-small']
# dataset_arr = ['movielens-small']
type_arr = ['index', 'IP', 'rank']

length = len(method_arr)
for ds in dataset_arr:
    for _type in type_arr:
        for i in range(length):
            for j in range(i + 1, length, 1):
                first = os.path.join('/home', 'bianzheng', 'reverse-k-ranks', 'result', 'rank',
                                     '{}-{}-top10-{}.csv'.format(ds, method_arr[i], _type))
                second = os.path.join('/home', 'bianzheng', 'reverse-k-ranks', 'result', 'rank',
                                      '{}-{}-top10-{}.csv'.format(ds, method_arr[j], _type))
                os.system("diff {} {}".format(first, second))
