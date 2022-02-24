import os

if __name__ == '__main__':
    # dataset_l = ['fake', 'fake-big', 'movielens-small']
    dataset_l = ['movielens-1m']
    for ds in dataset_l:
        # os.system('cd build && ./bfon %s' % ds)
        os.system('cd build && ./bfmi %s' % ds)
        os.system('cd build && ./bfdi %s' % ds)
        # os.system('cd build && ./rkbkt %s' % ds)
        # os.system('cd build && ./itvvec %s' % ds)
        os.system('cd build && ./bscb %s' % ds)
