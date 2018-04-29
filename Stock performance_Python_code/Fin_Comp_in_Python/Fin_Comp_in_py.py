#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Nov 20 23:49:53 2017

@author: qiangji
"""

import numpy as np
import pandas as pd
import time,datetime
from dateutil.parser import parse

store=pd.HDFStore('S&P500_quotes.h5','r')

N=30
limit=5
sam_times=4
sam_num=40
mp=store.select('S&P500',columns=['Adj Close'])
mr=mp/mp.shift(1)-1
## import csv data form bloomberg, transform to dict
data_bloom=pd.read_csv('YeahTest.csv',usecols=['Ticker','Date','surprise'])
## modify ticker string
data_bloom=data_bloom.set_index('Ticker')
data=data_bloom.T.to_dict()
data.pop('IQV')#Yahoo does not have quotes for it

## divide by “Surprise”
beatSet={}
meetSet={}
missSet={}
for t,d in data.items():
    if d['surprise']<-limit:
        missSet[t]=d
    elif d['surprise']>limit:
        beatSet[t]=d
    else:
        meetSet[t]=d
        
setdict={'beat':beatSet,'miss':missSet,'meet':meetSet}

AR_set={}
AAR_set={}
CAAR_set={}
for k in range(sam_times):
    for s in setdict.keys():
        boots=np.random.randint(0,len(setdict[s])-1,sam_num)
        tlist=list(setdict[s].keys())
        AR_mtr={}
        for i in boots:
            sample=setdict[s][tlist[i]]
            t0=parse(sample['Date'])
            q=store.select(key=tlist[i],columns=['Adj Close'])
            t0_q=q.index.tolist().index(t0)
            q=q[t0_q-N-1:t0_q+N+1]
            r=q/q.shift(1)-1
            r.drop(r.index[0],inplace=True)
            
            t0_m=mr.index.tolist().index(t0)
            mr_w=mr[t0_m-N-1:t0_m+N+1]
            mr_w.drop(mr_w.index[0],inplace=True)
            AR=r-mr_w
            AR_mtr[tlist[i]]=list(AR.values.flatten())
        
        AR_df=pd.DataFrame(AR_mtr)
        AR_df['t']=range(-N,N+1)
        AR_set[s]=AR_df.set_index('t')
        AAR_set[s]=AR_set[s].mean(axis=1,skipna=True)
        CAAR_set[s]=AAR_set[s].cumsum()

    comb_AAR=pd.DataFrame(AAR_set)
    comb_CAAR=pd.DataFrame(CAAR_set)
    if k == 0:
        sa_AAR=comb_AAR
        sa_CAAR=comb_CAAR
    else:
        sa_AAR=(sa_AAR*k+comb_AAR)/(k+1)
        sa_CAAR=(sa_CAAR*k+comb_CAAR)/(k+1)
        
sa_AAR.plot(title="Course Project Result in Python_AAR")
sa_CAAR.plot(title="Course Project Result in Python_CAAR")