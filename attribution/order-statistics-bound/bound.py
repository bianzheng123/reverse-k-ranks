import numpy as np

if __name__ == '__main__':
    arr = np.random.normal(size=100000)
    max_val = np.max(arr)
    min_val = np.min(arr)
    print(max_val, min_val)
