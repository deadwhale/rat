FROM rocm/pytorch:rocm3.7_ubuntu18.04_py3.6_pytorch

RUN sed -i 's/debian/3.7/' /etc/apt/sources.list.d/rocm.list

# nodejs
RUN apt-get install -y curl 
RUN curl -sL https://deb.nodesource.com/setup_current.x | bash 
RUN apt install -y nodejs 

RUN python3.6 -m pip install --upgrade pip
RUN pip3 install -U pip setuptools wheel tqdm pandas nltk seaborn sentencepiece
# jupyter
RUN pip3 install jupyterlab ipywidgets
RUN jupyter labextension install @jupyter-widgets/jupyterlab-manager
RUN jupyter serverextension enable --py jupyterlab

# spacy
# RUN pip3 install -U spacy
# RUN python3.6 -m spacy download ru_core_news_md

RUN apt-get -y install htop

COPY bg_jupyter.sh /usr/local/bin
RUN chmod +x /usr/local/bin/bg_jupyter.sh
CMD ["bg_jupyter.sh"]
