#!/bin/env python

import torch, sys, os
import torch.nn.functional as F
from models import LSTM, Transformer
from data import Vocab, SOS, EOS, PAD
import numpy as np
import sentencepiece as spm

def load(path, device):
    with open(path, 'rb') as f:
        checkpoint = torch.load(f, map_location=torch.device(device))
    vocab = checkpoint.pop('vocab')
    state_dict = checkpoint.pop('state_dict')
    MAX = checkpoint.pop('MAX')
    EMB_LEN = checkpoint.pop('EMB_LEN')
    try:
        model = LSTM(vocab, EMB_LEN, **checkpoint)
    except:
        model = Transformer(vocab, EMB_LEN, **checkpoint)
    model.load_state_dict(state_dict)
    model = model.to(device)
    return model, vocab

def predict(model, tokens, h, device, model_type='lstm', topk=100):
    if model_type == 'lstm':
        x = torch.tensor([[model.vocab[tokens]]], device=device)
        pred, h = model(x, hidden=h)
    elif model_type == 'trf':
        x = torch.tensor([[model.vocab[token] for token in tokens]], device=device)
        pred = model(x)[-1]
    pred = F.softmax(pred, dim=-1).data.cpu()
    p, top_ch = pred.topk(topk)
    top_ch = top_ch.numpy().squeeze()
    p = p.numpy().squeeze()
    if p.ndim == 2:
        top_ch = top_ch[0, :]
        p = p[0, :]
    token = np.random.choice(top_ch, p=p/p.sum())
    return model.vocab.i2t[token], h

def sample(model, size, start = '', device='cpu', topk=100, model_type='lstm', sp=None):
    model.eval()

    tokens = [SOS]
    tokens += list(start) if sp is None else sp.encode_as_pieces(start)
#     tokens += list(start[i:i+3] for i in range(0, len(start) - 2, 3))
    
    h = None
    if model_type == 'lstm':
        h = model.init_hidden(1)
        for t in tokens:   
            token, h = predict(model, t, h, device, model_type=model_type, topk=topk)
    else:
        token, h = predict(model, tokens, h, device, model_type=model_type, topk=topk)
    tokens.append(token)
    while len(tokens)-1 < size and tokens[-1] != EOS and tokens[-1] != PAD:
        token, h = predict(model, 
                           tokens[-1] if model_type == 'lstm' else tokens,
                           h, device, model_type=model_type, topk=topk)
        tokens.append(token)
    
    if tokens[-1] in {EOS, PAD}: tokens = tokens[:-1]
    return ''.join(tokens[1:])
#     return sp.decode_pieces(tokens[1:])

def mix():
    import warnings
    warnings.filterwarnings("ignore")
    device = 'cpu'
    
    # models_names = [
    #     'lite',
    #     'standart',
    #     'trip_lite',
    #     'trip_standart'
    # ]
    # models = []
    # for name in models_names:
    #     model, _ = load('checkpoints/char_lstm_' + name, device)
    #     model = model.to(device)
    #     models.append(model)
    
    # samples = []
    # for model in models:
    #     for _ in range(10):
    #         size = max(10, np.random.normal(200, 50))
    #         topk = 100
    #         samples.append(sample(model, size, '', device, topk))

    # models_names = os.listdir('./models/')

    # name = np.random.choice(models_names)
    name = 'piece_trf_fat'
    model_type = 'lstm' if 'lstm' in name else 'trf'
    sp = None
    print(name.startswith('piece'))
    if name.startswith('piece'):
        sp = spm.SentencePieceProcessor()
        sp.load('m.model')
    model, _ = load('models/' + name, device)
    model = model.to(device)
    samples = []
    for _ in range(10):
        size = max(10, np.random.normal(200, 50))
        topk = 10#0
        samples.append(sample(model, size, '', device, topk=topk, model_type=model_type, sp=sp))

    result = np.random.choice(samples)
    print(result.strip().rsplit(' ', 1)[0])

if __name__=='__main__':
    mix()
    # device = 'cpu'
    
    # size = 100 if len(sys.argv)==2 else int(sys.argv[2])
    # topk = 100 if len(sys.argv)<=3 else int(sys.argv[3])
    # model, vocab = load(sys.argv[1], device)
    # model = model.to(device)
    
    # while True:
    #     s = input()
    #     print(sample(model, size, s, device, topk))
    #     print(100*'=')
