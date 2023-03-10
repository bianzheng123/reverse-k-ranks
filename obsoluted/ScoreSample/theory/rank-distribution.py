import numpy as np
import matplotlib.pyplot as plt


def show_freqency_hist(data, name):
    # plt.xlim((-3, 3))
    # plt.ylim((0, 0.5))
    '''
    plt.hist():
    参数bins: 划分间隔，可以采用 整数来指定间隔的数量，然后由程序由间隔的数量来确定每一个间隔的范围，也可以通过列表来直接指定间隔的范围
    density的类型是 bool型，若为True,则为频率直方图，反之，频数直方图:
        频率直方图，的在统计出每一个间隔中的频数后，将频数除以总的观测数，
        就得到了每一个间隔中的频率，然后将频率除以组距（每一个间隔的宽度），
        即用纵轴来表示 频率/组距 的大小；之所以要除以组距的目的是为了让频率直方图的阶梯形折线将逼近于概率密度曲线。
        也就是说，当观测数据充分大时，频率直方图近似地反映了概率密度曲线的大致形状，在统计推断中常常由此提出对总体分布形式的假设。
        频数直方图，就是给定桶的范围，查看有多少个落到这个桶的范围中
    stacked:如果为``True''，则多个数据相互堆叠
    '''
    plt.hist(data, bins=50, density=False, stacked=False)
    plt.savefig('rank_distribution-{}.jpg'.format(name))
    plt.close()


if __name__ == '__main__':
    df = np.loadtxt('../../result/rank/fake-test-rank-uniform-ScoreSample-top6000-n_sample_20-rank.csv', delimiter=',')
    df = df.reshape(-1)
    show_freqency_hist(df, "all-uniform")
    df = np.loadtxt('../../result/rank/fake-test-rank-normal-ScoreSample-top6000-n_sample_20-rank.csv', delimiter=',')
    df = df.reshape(-1)
    show_freqency_hist(df, "all-normal")
    # for i in range(100):
    #     show_freqency_hist(df[i], i)
