import vecs_io
import os
import numpy as np


def delete_file_if_exist(dire):
    if os.path.exists(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


if __name__ == '__main__':
    from_ds = 'movielens-27m'
    to_ds = 'movielens-sample'
    basic_dir = '/home/bianzheng/Dataset/ReverseMIPS'
    # n_user = 10
    n_item = 100
    data_item, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_data_item.dvecs' % from_ds))
    # print("len ", data_item.shape)
    # data_item_idx = np.random.permutation(len(data_item))[:n_item]
    # data_item = data_item[data_item_idx]
    print(data_item.shape)

    query_item, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_query_item.dvecs' % from_ds))
    query_item = query_item[[0]]

    user, d = vecs_io.dvecs_read(os.path.join(basic_dir, from_ds, '%s_user.dvecs' % from_ds))
    # total_user_idx = []
    # for userID in range(len(user)):
    #     all0 = True
    #     for dim in range(len(user[userID])):
    #         if user[userID][dim] != 0:
    #             all0 = False
    #     if not all0:
    #         total_user_idx.append(userID)
    # total_user_idx = np.array(total_user_idx)
    print("len ", user.shape)
    # user_idx = np.array(np.random.permutation(len(total_user_idx))[:n_user])
    # user_idx = total_user_idx[user_idx]
    user_idx = np.array(
        [243879, 265144, 63109, 95318, 205229, 60450, 126763, 172768, 155074, 67418, 113673, 196865, 265966, 226396,
         129454, 274023, 73988, 88212, 197585, 135513])
    user = user[user_idx]
    # print(user.shape)

    delete_file_if_exist(os.path.join(basic_dir, to_ds))
    os.mkdir(os.path.join(basic_dir, to_ds))
    vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_data_item.dvecs' % to_ds), data_item)
    vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_query_item.dvecs' % to_ds), query_item)
    vecs_io.dvecs_write(os.path.join(basic_dir, to_ds, '%s_user.dvecs' % to_ds), user)
