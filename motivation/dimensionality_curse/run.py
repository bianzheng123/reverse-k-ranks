import argparse
import json

from recbole.quick_start import run_recbole

if __name__ == '__main__':
    model = 'ENMF'
    dataset = 'ml-1m'
    ebd_l = [1024]
    # ebd_l = [2]
    res_l = {}
    for ebd in ebd_l:
        # config_dict = {'embedding_size': 64, 'epochs': 100, 'neg_sampling': None}
        # config_dict = {'embedding_size': 64, 'epochs': 100, 'topk': [10], 'valid_metric': 'MRR@10',
        #                'metrics': ['Recall', 'NDCG', 'Hit', 'Precision']}
        config_dict = {'embedding_size': ebd, 'epochs': 200, 'topk': [50, 100, 200], 'neg_sampling': None, 'use_gpu': False,
                       'train_batch_size': 512, 'learning_rate': 0.05, 'weight_decay': 0.5, 'learner': 'adagrad',
                       'eval_step': 20,
                       'valid_metric': 'hit@200',
                       'metrics': ['Recall', 'NDCG', 'Hit', 'Precision'],
                       'eval_args': {'split': {'LS': 'valid_and_test'}, 'group_by': 'user', 'order': 'RO',
                                     'mode': 'full'}}
        # config_dict=None

        res = run_recbole(model=model, dataset=dataset, config_dict=config_dict)

        with open('result/hitting_rate-%d-new.json' % (ebd), 'w') as f:
            json.dump(res, f)
        res_l[ebd] = res
    print(res_l)
