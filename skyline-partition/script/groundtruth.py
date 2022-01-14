import faiss
import numpy as np
import vecs_io


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


def get_gnd_dataset(dataset_name, topk):
    base, dim = vecs_io.fvecs_read('/home/bianzheng/Dataset/NIPS/%s/%s_base.fvecs' % (dataset_name, dataset_name))
    query, dim = vecs_io.fvecs_read('/home/bianzheng/Dataset/NIPS/%s/%s_query.fvecs' % (dataset_name, dataset_name))
    print(dataset_name, base.shape, query.shape)

    idx_, distance_ = ip_gnd(base, query, topk)
    vecs_io.ivecs_write('/home/bianzheng/Dataset/NIPS/%s/gnd-top%s.ivecs' % (dataset_name, topk), idx_)


if __name__ == '__main__':
    get_gnd_dataset('word2vec', 10)
    dataset_l = ['word2vec']
    for ds in dataset_l:
        gnd, topk = vecs_io.ivecs_read('/home/bianzheng/Dataset/NIPS/%s/gnd-top10.ivecs' % (ds))
        print(gnd.shape, topk)
