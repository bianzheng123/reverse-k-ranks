import os

if __name__ == '__main__':
    dataset_l = ['movielens-27m']
    for ds in dataset_l:
        os.system('cd build && ./bfdi %s /home/zhengbian/Dataset/ReverseMIPS' % ds)
        os.system('cd build && ./bscb %s /home/zhengbian/Dataset/ReverseMIPS' % ds)
