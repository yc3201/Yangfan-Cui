#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Dec 10 22:31:20 2017

@author: qiangji
"""
"""
##------ Back-testing Details --------
Assumptions:
    No transaction fee
    No brokerage commissions
    No short loan fee
    initial margin requirement =150%
    maintaining margin requrement =130%
    Long Market Value : Short Market Value = 1:a
    
Timing Order:
    Given k and n,
    When no earning releasing event occurs, we invest 100% in SPY;
    Once a stock has released earning, if it's "Beat", we long it (if "Miss", short it)
    on k days after the earning releasing day, and hold the position for n days;
    While the tatal MV of long and short postion should match 1:a,
    in both positions, asset will be allocated to make every stock's market value equals.
    
Worthy of memtioning:
    1. SPY is always in th long position, with proportion of 1/(stkNum+1)
        where stkNum is the number of stocks currently in long position.
    2. Due to the margin requrement for short sale, we should also spare
        some cash as collateral if we hold short position.
    3. For those stocks whoes earning released after-market, we can only trade it 
        at the open price on the day after the releasing day.
"""
import numpy as np
import pandas as pd
import time,datetime
from dateutil.parser import parse
from datetime import datetime
import time

a=1 #Short Market Value
k=0 #trade on k days after the releasing day
n=5 #hold the stock for n days
imar=1.5 #initial margin requirement
mmar=1.3 #maintaining margin requirement
uplimit=5 #defined as "Beat"
lowlimit=-5 #defined as "Miss"
#-----------/* specify the n days stock Rt */--------------
store=pd.HDFStore('S&P500_quotes.h5','r')
def speStkRt(bb_index_,k_,n_):
    inTrday=1
    try:
        stkcls_=store.select(bb_csv.loc[bb_index_,'Ticker'],columns=["Open","Close","Adj Close"])
    except:
        print("This  found in HDF, Skip!")
        return [0]
    stk_rt_=stkcls_["Adj Close"]/stkcls_["Adj Close"].shift(1) - 1
    try:
        time0=stk_rt_.index.get_loc(parse(bb_csv.loc[bb_index_,'Date']))
    except:
        newdate=stk_rt_.index[stk_rt_.index>parse(bb_csv.loc[bb_index_,'Date'])][0]
        time0=stk_rt_.index.get_loc(newdate)
        inTrday=0
        
    evstk_=stk_rt_.iloc[time0+1+k_:time0+1+k_+n_]
    evstk_.name = bb_csv.loc[bb_index_,'Ticker']
    if ((bb_csv.loc[bb_index_,'after-market']==1 and k_==0) or (inTrday==0)): 
        #for those stocks whoes earning released after-market, revise the first day Rt
        stktemp_=stkcls_.loc[evstk_.index[0]]
        adjOpen=stktemp_['Adj Close']*stktemp_['Open']/stktemp_['Close']
        adjRt=stktemp_['Adj Close']/adjOpen -1
        evstk_[0]=adjRt
        return evstk_
    else:
        return evstk_

#-----------/* retrieve market Rt form database */--------------
bb_csv=pd.read_csv('BTdata.csv')
mkt_cls=store.select("SPY")["Adj Close"]
mkt_rt=mkt_cls/mkt_cls.shift(1) - 1
mkt_rt=mkt_rt.loc["2014-01-02":"2017-11-15"]
#------/* Generate long position Matrix & short position Matrix */--------
longMat=pd.DataFrame({"SPY":mkt_rt})
shortMat=[]
for i in range(len(bb_csv)):
    surp=bb_csv.loc[i,'Surprise']
    evstk=speStkRt(i,k,n)
    if evstk[0]==0: continue
    if (surp>uplimit):
        
        if (bb_csv.loc[i,'Ticker'] not in longMat.columns):
            longMat=longMat.join(evstk)
        else:
            for j in evstk.index:
                longMat.loc[j,bb_csv.loc[i,'Ticker']]=evstk.loc[j]
        
    elif (surp<lowlimit):
        evstk=-evstk
        
        if (len(shortMat)==0):
            shortMat=pd.DataFrame(evstk,index=longMat.index)
        else:
            if (bb_csv.loc[i,'Ticker'] not in shortMat.columns):
                shortMat=shortMat.join(evstk)
            else:
                for j in evstk.index:
                    shortMat.loc[j,bb_csv.loc[i,'Ticker']]=evstk.loc[j]
                    
store.close()
longRt=longMat.mean(axis=1,skipna=True)
shortRt=shortMat.mean(axis=1,skipna=True).fillna(0)

#------/* Compute average Margin Requirement Ratio */--------
marginReq=shortMat.copy()
for i in marginReq.columns:
    for j in range(1,len(marginReq)):
        if (pd.isnull(marginReq[i][j-1]) and pd.notnull(marginReq[i][j])):
            marginReq[i][j]=1.5
        elif (pd.notnull(marginReq[i][j-1])&pd.notnull(marginReq[i][j])):
            marginReq[i][j]=1.3
            
ReqRate=marginReq.mean(axis=1,skipna=True).fillna(0)

#------/* Compute portfolio Rt */--------
longWeight=1/(ReqRate*a+1)
shortWeight=a/(ReqRate*a+1)

portfolioRt=longWeight*longRt+longWeight*shortRt

Result_rt=pd.DataFrame({"SPY":mkt_rt,"OnlyLong":longRt,"Long/Short":portfolioRt})
Result_NAV=(Result_rt+1).cumprod()
Result_NAV.plot(title="NAV of back-testing from 2014-01-02 to 2017-11-15")
#------/* Output Results*/-------

risk_free=0.026
AnRt=(Result_NAV.iloc[-1]/Result_NAV.iloc[0])**(365/(Result_NAV.index[-1]-Result_NAV.index[0]).days)-1
covmatrix=Result_rt.cov()["SPY"]

beta=covmatrix/covmatrix[-1]
alpha=(AnRt-risk_free)-beta*(AnRt["SPY"]-risk_free)
SharpRatio=(AnRt-risk_free)/(Result_rt.std()*np.sqrt(250))
infoRatio=(AnRt-AnRt["SPY"])/(Result_rt.std()*np.sqrt(250))

Maxdrawdown=(Result_NAV/np.maximum.accumulate(Result_NAV)-1).min()
Report=pd.DataFrame({"AnnualRt":AnRt,"MaxDrawDown":Maxdrawdown,"beta":beta,"alpha":alpha,"SharpRatio":SharpRatio,"infoRatio":infoRatio})
Report.to_excel("Backtesting_report.xls")