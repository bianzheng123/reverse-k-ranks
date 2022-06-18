from scipy.stats import norm

n_user = 480189
n_item = 17770
k = 120
n_interval = 16
for i in range(0, n_interval + 1, 1):
    itv_n_user = n_user * (1 - norm.cdf(4.7 - 9.4 / n_interval * i))
    print(itv_n_user)
    if itv_n_user > k:
        pr = (n_user - itv_n_user) / n_user
        print("prune ratio", pr)
        break
# for i in range(0, n_interval + 1, 1):
#     itv_n_user = n_user / (n_item + 1) * n_interval * (i + 1) / (n_interval + 1)
#     print(itv_n_user)
#     if itv_n_user > k:
#         pr = (n_user - itv_n_user) / n_user
#         print("prune ratio", pr)
#         break

import scipy.stats
import numpy
alpha = numpy.pi / 8
minimum_expect = scipy.stats.norm.ppf((1 - alpha) / (n_user - 2 * alpha + 1))
maximum_expect = scipy.stats.norm.ppf((n_user - alpha) / (n_user - 2 * alpha + 1))
print(minimum_expect, maximum_expect)
