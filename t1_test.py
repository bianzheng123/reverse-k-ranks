# run the result in different mode, then use the information
import numpy as np
import os


def cmp(set1, set2, result: str):
    nvecs1 = len(set1)
    nvecs2 = len(set2)
    assert nvecs2 == nvecs1
    for i, (ele1, ele2) in enumerate(zip(set1, set2), 0):
        if i == 0:
            print(ele1, ele2)
        assert np.abs(ele1 - ele2) < 0.01, f'{result} {ele1} {ele2} {i} {set1[i+1]} {set2[i+1]} {set1[i+2]} {set2[i+2]}'


if __name__ == '__main__':
    path = '/home/bianzheng/reverse-k-ranks/index/test_index/'

    cpu_batch = np.fromfile(os.path.join(path, 'cpu_batch/fake-normal.index'), dtype='float64').astype(np.float64)
    cpu_single = np.fromfile(os.path.join(path, 'cpu_single/fake-normal.index'), dtype='float64').astype(np.float64)
    gpu_batch = np.fromfile(os.path.join(path, 'gpu_batch/fake-normal.index'), dtype='float64').astype(np.float64)
    gpu_single = np.fromfile(os.path.join(path, 'gpu_single/fake-normal.index'), dtype='float64').astype(np.float64)
    # cmp(cpu_batch, cpu_single, 'cpu_batch cpu_single')
    # cmp(cpu_single, gpu_single, 'cpu_single gpu_single')
    cmp(cpu_single, gpu_batch, 'cpu_single gpu_batch')

