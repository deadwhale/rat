import pandas as pd

SOS = "<start>"
EOS = "<end>"
PAD = "<pad>"

class Vocab:
    def __init__(self, df=None, vocab=None):
        if not df is None:
            self.vocab = {SOS, EOS, PAD}
            for comment in df.comment:
                self.vocab |= set(comment)
        elif not vocab is None:
            self.vocab = vocab
        else: return
        self.i2t = dict(enumerate(tuple(self.vocab)))
        self.t2i = {w: i for i, w in self.i2t.items()}
        self.SOS, self.EOS, self.PAD = (self.t2i[t] for t in (SOS, EOS, PAD))
    def __len__(self):
        return len(self.vocab)
    
    def __getitem__(self, key):
        if isinstance(key, int):
            return self.i2t.get(key, '')
        elif isinstance(key, str):
            return self.t2i.get(key, len(self.vocab))
