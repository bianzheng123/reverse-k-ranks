import vecs_io
import numpy as np
import matplotlib.pyplot as plt
import os
import json
import sys
import time

if __name__ == '__main__':
    ds = sys.argv[1]

    start_time = time.time()
    base, dim = vecs_io.fvecs_read('data/%s/origin.fvecs' % ds)
    query, dim = vecs_io.fvecs_read('data/%s/origin_query.fvecs' % ds)
    # print([np.dot(q.T, base.T) for q in query])
    base = base.T
    u, sigma, v_prime = np.linalg.svd(base, full_matrices=False)
    # print(sigma)
    v = v_prime.T
    sigma = np.diag(sigma)

    # query_multiply = np.dot(np.diag(sigma), u.T)
    query = np.array([np.dot(np.dot(sigma, u.T), q) for q in query])

    # print([np.dot(q.T, base.T) for q in query])

    vecs_io.fvecs_write('data/%s/svd.fvecs' % ds, v)
    vecs_io.fvecs_write('data/%s/svd_query.fvecs' % ds, query)
    end_time = time.time()
    print("svd, time consumed", end_time - start_time)
