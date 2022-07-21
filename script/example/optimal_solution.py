if __name__ == '__main__':
    # np.random.seed(0)
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