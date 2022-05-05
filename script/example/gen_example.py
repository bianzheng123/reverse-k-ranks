import numpy as np
import faiss


def ip_gnd(base, query, k):
    base_dim = base.shape[1]
    index = faiss.IndexFlatIP(base_dim)
    index.add(base)
    gnd_distance, gnd_idx = index.search(query, k)
    return gnd_idx, gnd_distance


if __name__ == '__main__':
    for i in range(1000):
        d = 2
        n_item = 5
        n_user = 4
        item_l = np.random.rand(n_item * d).reshape(-1, d).astype(np.float32) * 4
        user_l = np.random.rand(n_user * d).reshape(-1, d).astype(np.float32) * 4
        gnd_idx_l, gnd_dist_l = ip_gnd(item_l, user_l, len(item_l))
        gnd_idx_l = gnd_idx_l + 1
        arr_end_idx = []
        for gnd_idx in gnd_idx_l:
            for _ in range(len(gnd_idx)):
                if gnd_idx[_] == 5:
                    arr_end_idx.append(_)
        arr_end_idx = np.array(arr_end_idx)
        arr_end_idx = np.unique(arr_end_idx)
        if len(arr_end_idx) > 3:
            print(item_l)
            print(user_l)
            print(gnd_idx_l)
            print(gnd_dist_l)
            break

    # query_item = np.array([6.4, 6.3, 8.3], dtype=np.float32)
    # for i in range(len(user_l)):
    #     string = ""
    #     for j in range(len(query_item)):
    #         string += " %8.3f" % (query_item[j] * user_l[i][j])
    #     print(string)
