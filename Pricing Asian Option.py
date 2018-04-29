#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Mar 28 19:03:01 2018

@author: Yangfan Cui
"""

import numpy as np 
from math import sqrt,exp,log
from scipy.stats import norm
from scipy.stats import gmean

## Method 1
n_simulation = 1000000
S0 = 110
r = 0.01
sigma=0.2
K=100
T=1
m=12
dt = T/m
C = np.zeros(n_simulation)
np.random.seed(10)
for i in range(0,n_simulation):
    S = S0
    sum_S = 0
    for j in range(0,m):
        e = np.random.normal()
        S *= exp((r-0.5*sigma**2)*dt+sigma*sqrt(dt)*e)
        sum_S+=S
        S_average = sum_S/m
    C[i] = exp(-r*T)*max(S_average-K,0)
    call_price = C.mean()
print('Asian Option Price is: ',round(call_price,3))
error = sqrt(((C-call_price)**2).sum()/(len(C)-1))/sqrt(len(C))

## Method 2
np.random.seed(10)
geo_price = np.zeros(n_simulation)
for i in range(0,n_simulation):
    S = S0
    geo_product = 1
    for j in range(0,m):
        e = np.random.normal()
        S *= exp((r-0.5*sigma**2)*dt+sigma*sqrt(dt)*e)
        geo_product *= S
        geo_average = geo_product**(1/m)
    geo_price[i] = exp(-r*T)*max(geo_average-K,0)   

b_star = np.corrcoef(C,geo_price)[0][1]
T_tem = np.zeros(m)
for i in range(0,m):
    T_tem[i] = i*dt
T_adj = T_tem.mean()
def sigma_generate(n_step):
    T_reverse = np.zeros(n_step)
    for i in range(0,n_step):
        T_reverse[i] = (2*i-1)*(m-i)*dt
    sum_T = T_reverse.sum()
    sigma_adj = sqrt(pow(sigma,2)*sum_T/((m**2)*T_adj))
    return sigma_adj
sigma_adj = sigma_generate(m)

delta = 0.5*(sigma**2-sigma_adj**2)    
d = (log(S0/K)+(r-delta+0.5*sigma_adj**2)*T_adj)/(sigma_adj*sqrt(T_adj))
Geo_price = exp(-delta*T_adj)*exp(-r*(T-T_adj))*S0*norm.cdf(d) - exp(-r*T)*K*norm.cdf(d-sigma_adj*sqrt(T_adj))      
Arith_Asian = (C-b_star*(geo_price-Geo_price)).mean()     
error_2 = sqrt(((C-b_star*(geo_price-Geo_price)-Arith_Asian)**2).sum()/(len(C)-1))/sqrt(len(C))      
        
        
        
        