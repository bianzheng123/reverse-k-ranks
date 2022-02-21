import os

if __name__ == '__main__':
    dataset_l = ['fake', 'movielens-1m', 'movielens-small']
    for ds in dataset_l:
        os.system('cd build && ./bfon %s' % ds)
        os.system('cd build && ./bfmi %s' % ds)
        os.system('cd build && ./bfdi %s' % ds)
        os.system('cd build && ./rkbkt %s' % ds)
        os.system('cd build && ./itvvec %s' % ds)
