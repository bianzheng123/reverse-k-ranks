import functools
import numpy as np

a = [3, 8, 14, 6, 7]


def compare_personal(x, y):
    return x % 7 - y % 7


a.sort(key=functools.cmp_to_key(compare_personal))
b = sorted(a, key=functools.cmp_to_key(compare_personal))
print(a)
print(b)


tmp_a = np.array([1, 2, 3])
tmp_b = np.array([1, 2, 3])
print(np.dot(tmp_a, tmp_b))