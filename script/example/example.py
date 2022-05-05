import numpy as np
import gen_example


def normalization(user_l):
    norm_user_l = user_l.copy()
    for i in range(len(user_l)):
        norm = np.linalg.norm(user_l[i])
        norm_user_l[i] = norm_user_l[i] / norm
    return norm_user_l


def user_item_norm():
    item_l = np.array(
        [[0.7585935, 1.6275389],
         [1.7348094, 2.3759346],
         [3.1377952, 0.08928549],
         [1.8950846, 3.26434],
         [3.1293406, 1.0796206]], dtype=np.float64
    )

    user_l = np.array(
        [[0.13834432, 2.2017198],
         [2.9890387, 2.9058983],
         [0.6941855, 0.22097017],
         [1.6679517, 1.9731998]],
        dtype=np.float64)

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
    item_l = np.array(
        [[0.8, 1.6],
         [1.7, 2.4],
         [3.1, 0.1],
         [1.9, 3.3],
         [3.1, 1.1]], dtype=np.float32
    )

    user_l = np.array(
        [[0.1, 2.2],
         [3.0, 2.9],
         [0.7, 0.2],
         [1.7, 2.0]],
        dtype=np.float32)
    gnd_id, gnd_dist = gen_example.ip_gnd(item_l, user_l, len(item_l))
    gnd_id = gnd_id + 1
    print(gnd_id)
    print(gnd_dist)
