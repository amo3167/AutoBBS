import datetime, csv, os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.cbook as cbook
import sys
sys.path.insert(0, './include')
from asirikuy import *
from pylab import *

def datestr2num(dateStr):
    return datetime.datetime.strptime(dateStr, "%d/%m/%Y %H:%M")
    return mdates.datestr2num(dateStr)  
    
def plotTestResult(tradesFile,  show = False,  resolution = {"x": 800, 'y': 600}):
    dates = []
    balances = []
    figure(figsize=(resolution['x']/100, resolution['y']/100))
    with open(tradesFile, 'r') as f:
        reader = csv.reader(f)
        i = 0
        for row in reader:
            if i > 0:
                dates.append(datetime.datetime.strptime(row[3], "%d/%m/%Y %H:%M"))
                balances.append(float(row[10]))
            i = i + 1
    plt.plot_date(dates, balances, ',-', linewidth = 2.0, color = 'green',)
    plt.gcf().autofmt_xdate()
    plt.grid(True)
    plt.xlabel('Date')
    plt.ylabel('Balance')
    plt.title('Test Result')
    
    fileName, fileExtension = os.path.splitext(tradesFile)
    
    plt.savefig(fileName+'.png', bbox_inches=0, dpi=100)
    
    if show:
        plt.show()
    
    plt.close()
    del dates,  balances

def plotOptimizationResult(resultsFile, optimizationGoal = 0, show = False, resolution = {"x": 800, 'y': 600}):
    iterations = []
    scores = []
    figure(figsize=(resolution['x']/100, resolution['y']/100))
    global goalsDesc
    with open(resultsFile, 'r') as f:
        reader = csv.reader(f)
        i = 0
        for row in reader:
            if i > 0:
                iterations.append(int(row[0]))
                scores.append(float(row[optimizationGoal + 2]))
            i = i + 1

    plt.scatter(iterations, scores, s=15, marker = 'o', color = 'green',)
    plt.gcf().autofmt_xdate()
    plt.grid(True)
    plt.xlabel('Iteration')
    plt.ylabel(goalsDesc[optimizationGoal])
    plt.title('Optimization Result')
    
    fileName, fileExtension = os.path.splitext(resultsFile)
    
    plt.savefig(fileName+'.png', bbox_inches=0, dpi=100) 
    
    if show:
        plt.show()
    
    plt.close()
    del iterations,  scores
