import numpy as np
import torch, math
from torch import nn
import torch.nn.functional as F
from torch.nn.utils.rnn import pad_sequence, pack_padded_sequence, pad_packed_sequence, PackedSequence

class LSTM(nn.Module):
    
    def __init__(self, vocab, emb_len, hidden_size=256, n_layers=2,
                               dropout=0.2, bi=False, rnn='LSTM'):
        super().__init__()
        self.vocab = vocab
        self.dropout = dropout
        self.bi = bi
        self.n_layers = n_layers
        self.hidden_size = hidden_size
        self.emb_len = emb_len
        self.emb = nn.Embedding(len(vocab), emb_len)
        self.rnn = getattr(nn, rnn, 'LSTM')(emb_len, hidden_size, n_layers, 
                            dropout=dropout, batch_first=True, bidirectional=bi)
        self.output = nn.Linear(hidden_size * (bi+1), len(vocab))
        self.drop = nn.Dropout(p=dropout)
        
    def forward(self, x, mask = None, hidden = None):
        self.hidden = self.init_hidden(x.shape[0]) if hidden is None else hidden
             
        emb_x = self.drop(self.emb(x))
        lenghts = [x.shape[-1]]*x.shape[0] if mask is None else mask.long().sum(dim=-1).data.tolist()
        packed_x = pack_padded_sequence(
            input=emb_x,
            lengths=lenghts,
            batch_first=True,
            enforce_sorted=False,
        )
        
        hidden = self.hidden
        packed_out, hidden = self.rnn(packed_x, hidden)
        out, _ = pad_packed_sequence(packed_out, batch_first=True, padding_value=self.vocab.PAD)
        out = out.contiguous().view((-1, out.shape[-1]))

        pred = self.output(out)
        return pred.squeeze(-1), hidden
    
    def init_hidden(self, batch_size):
        weight = next(self.parameters())
        device = weight.device
        weight = weight.data
        n = self.n_layers * (self.bi + 1)
        k = np.sqrt(1/self.hidden_size)
        if isinstance(self.rnn, nn.LSTM):
            hidden = (weight.new(n, batch_size, self.hidden_size).uniform_(-k, k).to(device),
                    weight.new(n, batch_size, self.hidden_size).uniform_(-k, k).to(device))
        else:
            hidden = weight.new(n, batch_size, self.hidden_size).uniform_(-k, k).to(device)
        return hidden

class PositionalEncoding(nn.Module):

    def __init__(self, d_model, dropout=0.1, max_len=5000):
        super(PositionalEncoding, self).__init__()
        self.dropout = nn.Dropout(p=dropout)

        pe = torch.zeros(max_len, d_model)
        position = torch.arange(0, max_len, dtype=torch.float).unsqueeze(1)
        div_term = torch.exp(torch.arange(0, d_model, 2).float() * (-np.log(10000.0) / d_model))
        pe[:, 0::2] = torch.sin(position * div_term)
        pe[:, 1::2] = torch.cos(position * div_term)
        pe = pe.unsqueeze(0).transpose(0, 1)
        self.register_buffer('pe', pe)

    def forward(self, x):
        x = x + self.pe[:x.size(0), :]
        return self.dropout(x)

def generate_square_subsequent_mask(sz):
    mask = (torch.triu(torch.ones(sz, sz)) == 1).transpose(0, 1)
    mask = mask.float().masked_fill(mask == 0, float('-inf')).masked_fill(mask == 1, float(0.0))
    return mask
    
class Transformer(nn.Module):
    def __init__(self, vocab, emb_len, nhead=8, nlayers=6, nhid=512,
                               dropout=0.2):
        super().__init__()
        self.vocab = vocab
        self.dropout = dropout
        self.emb_len = emb_len
        self.nhead = nhead
        self.nhid = nhid
        self.nlayers = nlayers
        
        self.emb = nn.Embedding(len(vocab), emb_len)
        self.pos_encoder = PositionalEncoding(emb_len, dropout)
        encoder_layers = nn.TransformerEncoderLayer(emb_len, nhead, nhid, dropout)
        self.transformer_encoder = nn.TransformerEncoder(encoder_layers, nlayers)
  
        self.fc = nn.Linear(emb_len, len(vocab))
        self.init_weights()
        self.attn_mask = None
    
    def init_weights(self):
        initrange = 0.1
        nn.init.uniform_(self.emb.weight, -initrange, initrange)
        nn.init.zeros_(self.fc.weight)
        nn.init.uniform_(self.fc.weight, -initrange, initrange)

    def forward(self, x, mask = None):
        emb_x = self.emb(x) * np.sqrt(self.emb_len)
        pos_x = self.pos_encoder(emb_x)
        
        if mask is not None:
            mask = ~mask
            if self.attn_mask is None:
                self.attn_mask = generate_square_subsequent_mask(x.shape[1]).to(x.device)
        else:
            self.attn_mask = None

        pos_x = pos_x.permute(1, 0, 2)
        out = self.transformer_encoder(pos_x, 
                                       mask=self.attn_mask, 
                                       src_key_padding_mask=mask)
        out = out.permute(1, 0, 2)
        out = out.contiguous().view((-1, out.shape[-1]))
        output = self.fc(out)
        return output
