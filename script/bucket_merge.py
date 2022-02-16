from sklearn.cluster import KMeans
import numpy as np
import vecs_io

X, dim = vecs_io.dvecs_read('/run/media/hdd/ReverseMIPS/movielens-small/movielens-small_user.dvecs')

kmeans = KMeans(n_clusters=38, random_state=0).fit(X)
print(kmeans.labels_)
