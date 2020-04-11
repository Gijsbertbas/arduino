#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Jan 20 09:56:08 2019

test plotting arduino logs

@author: gijs
"""

from datetime import datetime
import pandas as pd
import matplotlib.pyplot as plt

logfile = '20190121_Voorburg_mega_test_stabiel.log'

with open('logfiles/'+logfile) as f:
    f.readline()
    log = f.readlines()

data=[]
for line in log:
    s = line.strip().split(' ')
    if s[0] != '9999':
        d = datetime.strptime(s[0]+" "+s[1],'%Y-%m-%d %H:%M:%S')
        if s[2] != 'nan':
            data.append([d,float(s[2]),float(s[3])])
    
df = pd.DataFrame(data=data, columns=['timestamp','temp','hum'])
df = df.set_index('timestamp')

# correction for a 2 hrs timeshift present in the landal log
if logfile == '20190119_landal_buiten.LOG':
    df.index = df.index.shift(2, freq='H')

# smooth result:
df_sm = df.rolling(15).mean()
# plot temperature
df_sm.plot(x=None, y='temp')

'''
Simpler loading script if I change the sketch...
'''

df = pd.read_csv('logfiles/'+logfile, delimiter=' ', index_col=0, parse_dates=True)
df.drop(labels='9999', inplace=True, errors='ignore')
df_sm = df.rolling(15).mean()
df_sm.plot(x=None, y='temperature')

'''
plot temperature graphs for 2 days in 1 axis
first, export from a Django shell the database readings using:
    df1 = dhtpostsbydate('day1')
    df1.to_csv('day1.csv')
    etc
then import into dataframes here:
    df1 = pd.read_csv('day1.csv')
    etc
'''
plt.figure()
ax = df1.plot(x='timestamp', y='temperature', label='4 feb')
df2.plot(x='timestamp', y='temperature', label='5 feb', ax=ax)
plt.savefig('test.png')
