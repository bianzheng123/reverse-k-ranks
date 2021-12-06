from script import vecs_io
import numpy as np
import matplotlib.pylab as plt
import matplotlib

if __name__ == '__main__':
    dataset = 'netflixsmall'
    with open('/home/bianzheng/Dataset/Reverse-MIPS/%s/%s_gnd_all.txt' % (dataset, dataset), 'r') as f:
        text = f.readlines()[1:]
    rmips_gnd_l = []
    for tmp in text:
        if tmp == '\n':
            rmips_gnd_l.append([])
            continue
        txt_l = tmp.split(" ")
        int_l = list(map(int, txt_l))
        rmips_gnd_l.append(int_l)

    item_len_res_l = [len(_) for _ in rmips_gnd_l]
    print(np.sum([len(_) for _ in rmips_gnd_l]))
    print(len(item_len_res_l))
    item_l, d = vecs_io.fvecs_read('/home/bianzheng/Dataset/Reverse-MIPS/%s/%s_item.fvecs' % (dataset, dataset))
    item_norm_l = np.linalg.norm(item_l, axis=1)
    print(item_norm_l.shape)
    plt.scatter(item_norm_l, item_len_res_l)
    plt.savefig('%s-reverse-norm-bias.jpg' % dataset)
    plt.close()
