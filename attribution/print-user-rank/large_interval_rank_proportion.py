import numpy as np

if __name__ == '__main__':
    user_rank = np.loadtxt('/home/bianzheng/reverse-k-ranks/result/attribution/print-user-rank-movielens-27m.csv', delimiter=',')
    print(np.max(user_rank[:, 0]), np.argmax(user_rank[:, 0]))