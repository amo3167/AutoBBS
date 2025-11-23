import datetime, csv, os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import matplotlib.dates as mdates
import matplotlib.cbook as cbook
import sys
sys.path.insert(0, './include')
from asirikuy import *
from pylab import *

def datestr2num(dateStr):
    return datetime.datetime.strptime(dateStr, "%d/%m/%Y %H:%M")
    return mdates.datestr2num(dateStr)

def in_unix(input):
  start = datetime.datetime(year=1970,month=1,day=1)
  diff = input - start
  return diff.total_seconds()

def plotTestResult(tradesFile,  regressionType = LINEAR_REGRESSION, show = False,  resolution = {"x": 800, 'y': 600}, pdf = False):
    dates = []
    balances = []
    volumes = []
    swaps = []
    epochDates = []
    lineFit = []
    balancesForFit = []
    fig = plt.figure(figsize=(resolution['x']/100, resolution['y']/100))
    gs = gridspec.GridSpec(2, 1,height_ratios=[4,1])
    ax1 = fig.add_subplot(gs[0])
    plt.title('Test Result')

    with open(tradesFile, 'r') as f:
        reader = csv.reader(f)
        i = 0
        for row in reader:
            if i > 0:
                dates.append(datetime.datetime.strptime(row[3], "%d/%m/%Y %H:%M"))
                balances.append(float(row[10]))
                volumes.append(float(row[6]))
                swaps.append(float(row[13]))
            i = i + 1

    for j in range(0,i-1):
        if regressionType == LINEAR_REGRESSION:
            balancesForFit.append(balances[j]-balances[0])
        if regressionType == EXPONENTIAL_REGRESSION:
            balancesForFit.append(log(balances[j])-log(balances[0]))
        epochDates.append(in_unix(dates[j])-in_unix(dates[0]))

    x = np.array(epochDates)
    y = np.array(balancesForFit)

    x = x[:,np.newaxis]
    a, _, _, _ = np.linalg.lstsq(x, y)


    if regressionType == LINEAR_REGRESSION:
        
        for j in range(0,i-1):
            lineFit.append(a*epochDates[j]+balances[0])
            
        ax1.plot_date(dates,lineFit, 'r--', linewidth = 2.0, color = '#ACFA58')
        ax1.plot_date(dates, balances, ',-', linewidth = 2.0, color = 'green')
        ax1.set_ylabel('Balance')
            
    if regressionType == EXPONENTIAL_REGRESSION:
        
        for j in range(0,i-1):    
            lineFit.append(exp(a*epochDates[j]+log(balances[0])))
            
        ax1.plot_date(dates,log(lineFit), 'r--', linewidth = 2.0, color = '#ACFA58')
        ax1.plot_date(dates,log(balances), ',-', linewidth = 2.0, color = 'green')
        ax1.set_ylabel('log(Balance)')

    plt.gcf().autofmt_xdate()
    plt.grid(True)

    
    ax1.set_title('Test Result')

    ax2 = ax1.twinx()
    ax2.bar(dates, volumes, width=0.1, edgecolor = '#08298A', color = '#08298A',)
    ax2.set_ylabel('Volume')
    try:
        ax2.set_ylim(0, max(volumes)*2.5)
    except:
        ax2.set_ylim(0,0)

    ax1.set_zorder(ax2.get_zorder()+1)
    ax1.patch.set_visible(False)

    ax3 = fig.add_subplot(gs[1])
    ax3.bar(dates, swaps, width=0.1, edgecolor = '#08298A', color = '#08298A',)
    ax3.set_xlabel('Date')
    ax3.set_ylabel('Swap P/L')
    matplotlib.rcParams.update({'font.size': 8})

    fileName, fileExtension = os.path.splitext(tradesFile)

    try:
        plt.savefig(fileName+'.png', bbox_inches=0, dpi=100)
    except IOError as e:
        print(e)

    if pdf:
        try:
            plt.savefig(fileName+'.pdf', bbox_inches=0, dpi=100)
        except IOError as e:
            print(e)

    if show:
        plt.show()

    plt.close()
    del dates,  balances

def plotMultipleTestResults(pairs,  show = False,  resolution = {"x": 800, 'y': 600}):

    fig = plt.figure(figsize=(resolution['x']/100, resolution['y']/100))
    gs = gridspec.GridSpec(2, 1,height_ratios=[4,1])
    ax1 = fig.add_subplot(gs[0])
    ax3 = fig.add_subplot(gs[1])

    for item in pairs:
        dates = []
        balances = []
        volumes = []
        swaps = []

        if os.path.isfile(item+"_results.txt"):
            with open(item+"_results.txt", 'r') as f:
                reader = csv.reader(f)
                i = 0
                for row in reader:
                    if i > 0:
                        dates.append(datetime.datetime.strptime(row[3], "%d/%m/%Y %H:%M"))
                        balances.append(float(row[10]))
                        volumes.append(float(row[6]))
                        swaps.append(float(row[13]))
                    i = i + 1
            import random
            colorToUse = [random.random(), random.random(), random.random()]
            ax1.plot_date(dates, balances, ',-', linewidth = 2.0, color = colorToUse, label=item)
            ax2 = ax1.twinx()
            ax2.bar(dates, volumes, width=0.1, edgecolor = colorToUse, color = colorToUse)
            ax3.bar(dates, swaps, width=0.1, edgecolor = colorToUse, color = colorToUse)

            try:
                ax2.set_ylim(0, max(volumes)*8)
            except:
                ax2.set_ylim(0,0)

    plt.gcf().autofmt_xdate()
    plt.grid(True)
    handles, labels = ax1.get_legend_handles_labels()
    legendBox = legend(handles, labels, loc=2)
    ax1.set_ylabel('Balance')
    ax2.set_ylabel('Volume')
    ax1.set_title('Test Result')
    ax3.set_xlabel('Date')
    ax3.set_ylabel('Swap P/L')
    matplotlib.rcParams.update({'font.size': 8})

    try:
        plt.savefig('multipleResults.png', bbox_inches=0, dpi=100)
    except IOError as e:
        print(e)

    if show:
        plt.show()

    plt.close()
    del dates,  balances

def plotPortfolioTestResult(tradesFile,  show = False,  resolution = {"x": 800, 'y': 600}, systemNames = None, pdf = False):
    dates = []
    balances = []
    systems = []
    idPerPosition = []
    profits = []
    volumes = []
    swaps = []
    fig = plt.figure(figsize=(resolution['x']/100, resolution['y']/100))
    gs = gridspec.GridSpec(2, 1,height_ratios=[4,1])
    ax1 = fig.add_subplot(gs[0])

    with open(tradesFile, 'r') as f:
        reader = csv.reader(f)
        i = 0
        for row in reader:
            if i > 0:
                dates.append(datetime.datetime.strptime(row[3], "%d/%m/%Y %H:%M"))
                balances.append(float(row[10]))
                idPerPosition.append(float(row[11]))
                profits.append(float(row[7]))
                volumes.append(float(row[6]))
                swaps.append(float(row[13]))
                if float(row[11]) not in systems:
                    systems.append(float(row[11]))
            i = i + 1
            if i == 2:
                initialBalance = float(row[10])-float(row[7])

    systems.sort()

    for j in range(0, len(systems)):
        newSystemProfits = []
        newSystemBalance = []
        newSystemDates   = []
        floatingBalance = initialBalance
        for i in range(0, len(idPerPosition)):
            if systems[j] == idPerPosition[i]:
                newSystemDates.append(dates[i])
                newSystemProfits.append(profits[i])
        for n in range(0, len(newSystemProfits)):
            floatingBalance += newSystemProfits[n]
            newSystemBalance.append(floatingBalance)
        import random
        if systemNames == None:
            ax1.plot_date(newSystemDates, newSystemBalance, ',-', linewidth = 1.0, color = [random.random(), random.random(), random.random()], zorder=systems[j]+2, label="System "+str(systems[j]))
        else:
            ax1.plot_date(newSystemDates, newSystemBalance, ',-', linewidth = 1.0, color = [random.random(), random.random(), random.random()], zorder=systems[j]+2, label=systemNames[j])


    ax1.plot_date(dates, balances, ',-', linewidth = 2.0, color = 'green', zorder=1, label="Portfolio")

    #handles, labels = ax1.get_legend_handles_labels()
    #legendBox = legend(handles, labels, loc=2)

    ax2 = ax1.twinx()
    ax1.set_zorder(ax2.get_zorder()+1)
    ax1.patch.set_visible(False)
    ax2.bar(dates, volumes, width=0.1, edgecolor = '#08298A', color = '#08298A', label="Portfolio Traded Volume")
    ax2.set_ylabel('Volume')
    ax2.set_ylim(0, max(volumes)*3)
    ax2.set_axisbelow(True)

    ax1.set_zorder(ax2.get_zorder()+1)
    ax2.patch.set_visible(False)

    ax1.set_title('Portfolio Test Result')

    ax3 = fig.add_subplot(gs[1])
    ax3.bar(dates, swaps, width=0.1, edgecolor = '#08298A', color = '#08298A',)
    ax3.set_xlabel('Date')
    ax3.set_ylabel('Swap P/L')

    plt.gcf().autofmt_xdate()
    plt.grid(True)
    ax1.set_ylabel('Balance')

    matplotlib.rcParams.update({'font.size': 8})

    fileName, fileExtension = os.path.splitext(tradesFile)

    try:
        plt.savefig(fileName+'.png', bbox_inches=0, dpi=100)
    except IOError as e:
        print(e)

    if pdf:
        try:
            plt.savefig(fileName+'.pdf', bbox_inches=0, dpi=100)
        except IOError as e:
            print(e)

    if show:
        plt.show()

    plt.close()
    del dates,  balances

def plotOptimizationResult(resultsFile, optimizationGoal = 0, show = False, resolution = {"x": 800, 'y': 600}, pdf = False):
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
                scores.append(float(row[optimizationGoal + 3]))
            i = i + 1

    plt.scatter(iterations, scores, s=15, color = 'green',)
    plt.gcf().autofmt_xdate()
    plt.grid(True)
    plt.xlabel('Iteration')
    plt.ylabel(goalsDesc[optimizationGoal])
    plt.title('Optimization Result')

    fileName, fileExtension = os.path.splitext(resultsFile)

    try:
        plt.savefig(fileName+'.png', bbox_inches=0, dpi=100)
    except IOError as e:
        print(e)

    if pdf:
        try:
            plt.savefig(fileName+'.pdf', bbox_inches=0, dpi=100)
        except IOError as e:
            print(e)

    if show:
        plt.show()

    plt.close()
    del iterations,  scores
