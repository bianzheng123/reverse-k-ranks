import faiss
import numpy as np
import vecs_io
import multiprocessing
import time


def parallel_reverse_mips_gnd(gnd_idx, query_idx_l):
    start_time = time.time()

    share_score_table = multiprocessing.Manager().list()
    for _ in range(len(query_idx_l)):
        share_score_table.append(0)

    manager = multiprocessing.managers.BaseManager()
    manager.register('RMIPSGnd', RMIPSGnd)
    manager.start()
    parallel_obj = manager.RMIPSGnd(gnd_idx, query_idx_l, multiprocessing.cpu_count())
    res_l = []
    pool = multiprocessing.Pool(multiprocessing.cpu_count())
    for i in range(multiprocessing.cpu_count()):
        res = pool.apply_async(r_mips_parallel, args=(parallel_obj, share_score_table, i))
        res_l.append(res)
    pool.close()
    pool.join()

    end_time = time.time()
    print("get reverse MIPS time: %.2f" % (end_time - start_time))
    return share_score_table


class RMIPSGnd:
    def __init__(self, gnd_idx, query_idx_l, total_process):
        self.gnd_idx = gnd_idx
        self.query_idx_l = query_idx_l
        self.total_process = total_process

    def get_share_data(self):
        return self.gnd_idx, self.query_idx_l, self.total_process


def r_mips_parallel(obj, share_score_table, start_idx):
    gnd_idx, query_idx_l, total_process = obj.get_share_data()
    # iteration for every query
    query_len = len(query_idx_l)
    for i in range(start_idx, query_len, total_process):
        if i % 50 == 0:
            print("get reverse mips result " + str(i))
        # tmp_score_table = np.zeros(shape=n_item, dtype=np.float32)
        r_mips_gnd = []
        idx = query_idx_l[i]
        for j, mips_gnd in enumerate(gnd_idx, 0):
            if idx in mips_gnd:
                r_mips_gnd.append(j)
        share_score_table[i] = r_mips_gnd
    print("finish parallel")


def ip_gnd(base, query, k):
    base_dim = base.shape[1]
    index = faiss.IndexFlatIP(base_dim)
    index.add(base)
    gnd_distance, gnd_idx = index.search(query, k)
    return gnd_idx, gnd_distance


def l2_gnd(base, query, k):
    base_dim = base.shape[1]
    index = faiss.IndexFlatL2(base_dim)
    index.add(base)
    gnd_distance, gnd_idx = index.search(query, k)
    print("search")
    return gnd_idx, gnd_distance


def reverse_mips_gnd(gnd_idx, query_idx_l):
    reverse_mips_gnd_l = []
    for idx in query_idx_l:
        r_mips_gnd = []
        for i, mips_gnd in enumerate(gnd_idx, 0):
            if idx in mips_gnd:
                r_mips_gnd.append(i)

        reverse_mips_gnd_l.append(r_mips_gnd)
    return reverse_mips_gnd_l


def save_rmips_gnd(gnd_l, filename):
    with open(filename, 'w') as f:
        f.write(str(len(gnd_l)) + '\n')
        for arr in gnd_l:
            arr = list(map(str, arr))
            f.write(' '.join(arr) + '\n')


def get_reverse_mips_gnd(topk, dataset, n_query=-1):
    item_l, d = vecs_io.fvecs_read('/home/bianzheng/Dataset/Reverse-MIPS/%s/%s_item.fvecs' % (dataset, dataset))
    user_l, d = vecs_io.fvecs_read('/home/bianzheng/Dataset/Reverse-MIPS/%s/%s_user.fvecs' % (dataset, dataset))

    gnd_idx_, gnd_distance = ip_gnd(item_l, user_l, topk)
    print(gnd_idx_.shape)
    permutation_l = np.arange(len(item_l))
    if n_query != -1:
        permutation_l = np.random.permutation(len(item_l))
        permutation_l = permutation_l[:n_query]

    # rmips_result = reverse_mips_gnd(gnd_idx_, np.arange(n_query))
    rmips_result = parallel_reverse_mips_gnd(gnd_idx_, permutation_l)
    print(np.sum([len(_) for _ in rmips_result]))

    query_l = np.array([permutation_l])
    if n_query != -1:
        save_rmips_gnd(rmips_result, '/home/bianzheng/Dataset/Reverse-MIPS/%s/%s_gnd.txt' % (dataset, dataset))
        vecs_io.ivecs_write('/home/bianzheng/Dataset/Reverse-MIPS/%s/%s_item_query_idx.ivecs' % (dataset, dataset), query_l)
    else:
        save_rmips_gnd(rmips_result, '/home/bianzheng/Dataset/Reverse-MIPS/%s/%s_gnd_all.txt' % (dataset, dataset))
        vecs_io.ivecs_write('/home/bianzheng/Dataset/Reverse-MIPS/%s/%s_item_query_idx_all.ivecs' % (dataset, dataset),
                            query_l)


if __name__ == '__main__':
    topk = 10
    # n_query = 100
    dataset = 'netflix'
    for n_query in [1000, -1]:
        get_reverse_mips_gnd(topk, dataset, n_query)
