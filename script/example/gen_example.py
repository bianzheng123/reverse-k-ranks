import numpy as np
import faiss


def ip_gnd(base, query, k):
    base_dim = base.shape[1]
    index = faiss.IndexFlatIP(base_dim)
    index.add(base)
    gnd_distance, gnd_idx = index.search(query, k)
    return gnd_idx, gnd_distance


# def compute_rank(user, item, query):
#     score_table = np.dot(user, item.T)  # 行是user * item_l
#     queryIP_l = np.dot(query, user.T)
#     query_rank_l = [(queryIP_l[0][userID] <= np.array(score_table[userID])).sum() + 1 for userID in range(len(user_l))]
#     return query_rank_l, queryIP_l[0], score_table


def compute_rank_IP(score_table, queryIP_l):
    query_rank_l = [(queryIP_l[userID] <= np.array(score_table[userID])).sum() + 1 for userID in
                    range(len(score_table))]
    return query_rank_l, queryIP_l, score_table


def rank_appear(query_rank_l, k):
    k_rank = np.sort(query_rank_l)[k - 1]
    count = 0
    for rank in query_rank_l:
        if k_rank == rank:
            count += 1
    n_appear = len(np.unique(query_rank_l))
    return n_appear > 3 and count == 1


def rank_sample(query_rank_l, n_sample, topt, n_refine, k):
    # 经过rank sample之后的refinement ratio要尽可能地高
    sample_every = np.ceil(topt / n_sample)
    query_rb_l = np.zeros(dtype=np.int32, shape=len(query_rank_l))
    for i in range(len(query_rank_l)):
        if query_rank_l[i] == 1:
            query_rb_l[i] = 0
        elif query_rank_l[i] > topt:
            query_rb_l[i] = n_sample + 1
        else:
            query_rb_l[i] = (query_rank_l[i] - 2) // (sample_every - 1) + 1
    sort_query_rb_l = np.sort(query_rb_l)
    n_actual_refine = (sort_query_rb_l[k - 1] >= sort_query_rb_l).sum()
    return n_actual_refine == n_refine


def larger_topt(query_rank_l, topt):
    n_large_topt = np.array([rank > topt for rank in query_rank_l]).sum()
    return n_large_topt == 1


def score_sample(score_table, queryIP_l, query_rank_l, n_sample, n_refine, k):
    # 经过score sample之后refinement ratio要很低
    high_score_l = np.array([np.max(score_list) for score_list in score_table], dtype=np.float32)
    low_score_l = np.array([np.min(score_list) for score_list in score_table], dtype=np.float32)
    n_user = len(score_table)
    n_item = len(score_table[0])
    dist_l = np.array([(high_score_l[i] - low_score_l[i]) / (n_sample + 1) for i in range(n_user)])
    interval_table = []
    for userID in range(n_user):
        score_list = np.array(score_table[userID])
        high_score = high_score_l[userID]
        dist = dist_l[userID]
        interval_l = np.zeros(shape=n_sample + 2, dtype=np.int32)
        for score in score_list:
            itvID = int(np.floor((high_score - score) / dist))
            interval_l[itvID] += 1
        interval_l = np.cumsum(interval_l)
        interval_table.append(interval_l)
    interval_table = np.array(interval_table, dtype=np.int32)

    query_rank_lb_l = np.zeros(shape=n_user, dtype=np.int32)
    query_rank_ub_l = np.zeros(shape=n_user, dtype=np.int32)
    for userID in range(n_user):
        high_score = high_score_l[userID]
        dist = dist_l[userID]
        queryIP = queryIP_l[userID]
        if dist == 0:
            return False
        itvID = int(np.floor((high_score - queryIP) / dist))
        # print("high_score {}, dist {}, queryIP {}, itvID {}".format(high_score, dist, queryIP, itvID))
        if queryIP > np.max(score_table[userID]):
            query_rank_lb_l[userID] = 0
            query_rank_ub_l[userID] = 0
        elif itvID >= n_sample + 2:
            query_rank_lb_l[userID] = n_item
            query_rank_ub_l[userID] = n_item
        else:
            if itvID == 0:
                query_rank_ub_l[userID] = int(1)
            else:
                query_rank_ub_l[userID] = int(interval_table[userID][itvID - 1])
            query_rank_lb_l[userID] = int(interval_table[userID][itvID])
    query_rank_lb_l += 1
    query_rank_ub_l += 1
    for userID in range(n_user):
        assert query_rank_ub_l[userID] <= query_rank_l[userID] <= query_rank_lb_l[userID]
    lbtop = np.sort(query_rank_lb_l)[k - 1]
    n_actual_refine = (lbtop >= query_rank_ub_l).sum()
    for userID in range(n_user):
        if lbtop >= query_rank_ub_l[userID]:
            if query_rank_l[userID] == query_rank_lb_l[userID] or query_rank_l[userID] == query_rank_ub_l[userID]:
                return False
    assert n_actual_refine >= k
    print(lbtop, query_rank_lb_l, query_rank_ub_l)
    return n_actual_refine == n_refine


if __name__ == '__main__':
    # np.random.seed(0)
    d = 2
    n_item = 11  # 规定最后一个ID是query
    n_user = 5
    k = 2
    topt = 8
    n_sample = 3
    n_rank_sample_refine = 2
    n_score_sample_refine = 4
    for i in range(10000000):
        # query = (np.random.rand(d).reshape(-1, d) * 10).astype(np.int32) / 10 * 4
        # item_l = (np.random.rand(n_item * d).reshape(-1, d) * 10).astype(np.int32) / 10 * 4
        # user_l = (np.random.rand(n_user * d).reshape(-1, d) * 10).astype(np.int32)
        # print("query\n", query)
        # print("user_l\n", user_l)
        # print("item_l\n", item_l)
        # query_rank_l, queryIP_l, score_table = compute_rank(user_l, item_l, query)

        score_table = ((np.random.rand(n_user * n_item).reshape(n_user, n_item) * 50) * 10).astype(np.int32).astype(
            np.float32) / 10
        queryIP_l = ((np.random.rand(n_user) * 50) * 10).astype(np.int32).astype(np.float32) / 10

        query_rank_l, queryIP_l, score_table = compute_rank_IP(score_table, queryIP_l)
        continue_flag = False
        for i, score_list in enumerate(score_table, 0):
            if len(np.unique(score_list)) != len(score_list) and queryIP_l[i] in score_list:
                continue_flag = True
                break
        if continue_flag:
            continue
        if rank_appear(query_rank_l, k) and \
                rank_sample(query_rank_l, n_sample, topt, n_rank_sample_refine, k) and \
                larger_topt(query_rank_l, topt) and \
                score_sample(score_table, queryIP_l, query_rank_l, n_sample, n_score_sample_refine, k):
            # print("query\n", query)
            # print("item_l\n", item_l)
            # print("user_l\n", user_l)
            print("score_table\n", score_table)
            print("queryIP_l\n", queryIP_l)
            print("query_rank_l\n", query_rank_l)
            sort_list = np.array([np.sort(score_list) for score_list in score_table])
            print("sort_score_table\n", sort_list)
            break
