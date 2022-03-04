import numpy as np


def normalization(user_l):
    norm_user_l = user_l.copy()
    for i in range(len(user_l)):
        norm = np.linalg.norm(user_l[i])
        norm_user_l[i] = norm_user_l[i] / norm
    return norm_user_l


def user_item_norm():
    user_l = np.array([
        [2.7, 2.1, 8.6],
        [7.5, 5.0, 3.4],
        [8.2, 2.5, 3.1],
        [4.4, 3.9, 4.6],
        [4.0, 0.6, 2.8],
        [8.3, 9.4, 8.2],
    ], dtype=np.float64)
    item_l = np.array([
        [3.6, 3.5, 3.9],
        [5.0, 2.8, 4.2],
        [9.1, 7.4, 1.8],
        [9.0, 5.9, 1.3],
        [9.2, 4.3, 3.5],
        [3.0, 9.8, 5.7],
    ], dtype=np.float64)

    IP_l = np.dot(user_l, item_l.T)
    before_ip_l = np.array([np.sort(-_) for _ in IP_l])
    before_ip_l = -before_ip_l
    print("before")
    print(before_ip_l.T)
    norm_user_l = normalization(user_l)
    IP_l = np.dot(norm_user_l, item_l.T)
    after_ip_l = np.array([np.sort(-_) for _ in IP_l])
    after_ip_l = -after_ip_l
    print("after")
    print(after_ip_l.T)


def q_norm():
    q = np.array([0.0, 8.2, 8.8])
    norm_q = np.linalg.norm(q)
    print(norm_q)


if __name__ == '__main__':
    bound_l = np.array([
        [8.39, 5.17],
        [11.56, 5.99],
        [10.82, 5.52],
        [10.40, 6.35],
        [9.99, 5.57],
        [10.92, 6.32]
    ], dtype=np.float64)
    norm = 12.02
    percent_l = np.array([(norm - _[1]) / (_[0] - _[1]) for _ in bound_l])
    print(percent_l)
