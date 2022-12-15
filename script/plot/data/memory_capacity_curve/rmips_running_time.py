import math
import re
import numpy as np
import pandas as pd


def get_rmips_query_time_simpfer(*, dataset_name: str, k_max: int):
    file = open('./rmips/{}-SimpferOnly-simpfer_k_max_{}-config.txt'.format(dataset_name, k_max))
    lines = file.read().split("\n")
    query_info = []
    for line in lines:
        match_obj = re.match(
            r'\ttotal IP cost (.*), total time (.*)s',
            line)
        if match_obj:
            ip_cost = int(match_obj.group(1))
            total_time = float(match_obj.group(2))
            query_info.append(ip_cost)
            query_info.append(total_time)
    assert len(query_info) == 2

    print("No.total query 100, total IP {}, total time {}s".format(query_info[0], query_info[1]))
    total_query_time = query_info[1] / query_info[0] * 1000
    total_ip_cost = query_info[0] / 100 * 1000
    return total_query_time, total_ip_cost


def get_rmips_terminate_topk(*, dataset_name: str, n_query: int):
    file = open('./rmips/find_terminate_topk_{}.log'.format(dataset_name))
    lines = file.read().split("\n")
    query_terminate_topk_l = []
    for line in lines:
        match_obj = re.match(
            r'\[.*\] \[info\] queryID (.*), result_size (.*), rtk_topk (.*), ip_cost (.*), '
            r'query_time (.*)s, total_ip_cost (.*), total_retrieval_time (.*)s',
            line)
        if match_obj:
            rtk_topk = int(match_obj.group(3))

            query_terminate_topk_l.append(rtk_topk)
    assert len(query_terminate_topk_l) == n_query
    return query_terminate_topk_l


def get_rmips_fexipro_ipcost(*, rtopk_ip_cost_map: dict, k_max: int, terminate_topk_l: list,
                             n_test_query: int):
    ip_cost_l = []
    for terminate_topk in terminate_topk_l:
        larger_k_max_l = []
        for rtopk in rtopk_ip_cost_map.keys():
            if k_max < rtopk and rtopk <= terminate_topk:
                larger_k_max_l.append(rtopk)
        ip_cost = np.sum([rtopk_ip_cost_map[topk] for topk in larger_k_max_l])
        ip_cost_l.append(ip_cost)
    predict_ip_cost = np.sum(ip_cost_l) / n_test_query * 1000
    return predict_ip_cost


if __name__ == '__main__':
    # by default, first is yahoomusic, second is yelp
    dataset_name_l = ['yahoomusic_big', 'yelp']
    # running time / IP cost
    fexipro_host = [18091 / 8857173406, 4638 / 1282375896]
    k_max_l = [[36, 73, 147, 294, 588],
               [30, 61, 122, 245, 490]]
    memory_capacity_l = [2, 4, 8, 16, 32]
    rtopk_ip_cost_m = [{8192: 15210782397, 4096: 7743041213, 2048: 4009170621,
                        1024: 2142235325, 512: 1208767677, 256: 742033853,
                        128: 508666941, 64: 391983485, 32: 0, 16: 0, 8: 0, 4: 0, 2: 0, 1: 0},
                       {4096: 9298623879, 2048: 4814615943, 1024: 2572611975,
                        512: 1451609991, 256: 891108999, 128: 610858503, 64: 470733255,
                        32: 400670631, 16: 0, 8: 0, 4: 0, 2: 0, 1: 0}]
    for ds, k_max_ds_l, fexipro_time_div_ip_cost, n_test_query, rtopk_ip_cost_map in zip(dataset_name_l,
                                                                                         k_max_l, fexipro_host,
                                                                                         [12, 16],
                                                                                         rtopk_ip_cost_m):
        terminate_topk_l = get_rmips_terminate_topk(dataset_name=ds, n_query=n_test_query)
        method_info_l = []
        for k_max, memory_capacity in zip(k_max_ds_l, memory_capacity_l):
            query_time, ip_cost = get_rmips_query_time_simpfer(dataset_name=ds, k_max=k_max)
            fexipro_ip_cost = get_rmips_fexipro_ipcost(rtopk_ip_cost_map=rtopk_ip_cost_map,
                                                       terminate_topk_l=terminate_topk_l,
                                                       k_max=k_max, n_test_query=n_test_query)
            total_ip_cost = ip_cost + fexipro_ip_cost
            total_time = query_time + fexipro_ip_cost * fexipro_time_div_ip_cost
            method_info_l.append([total_ip_cost, total_time])
        print(ds, method_info_l)
