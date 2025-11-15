from __future__ import division
from mt import *
from asirikuy import *
import json, httplib, math, calendar, datetime, requests , jpype
from pytz import timezone
from jpype import *
import dateutil
from urllib2 import HTTPError
from colorama import Fore
import rfc3339
import time
import platform
from shutil import copyfile
from colorama import init, Fore, Back, Style
import copy
from tinydb import TinyDB, Query

BID_ASK_EXPIRATION_IN_SECONDS   =   5.0


class Account:
    def __init__(self, depositSymbol, initialBalance, name=None, brokerName=None, login=None, password=None, useOrderWrapper=None, hasStaticIP=None, isDemo=True, allBidAskSymbols=None, allBidAskUpdateTimes=None, allBidAskLoaded=None):

        #Load Asirikuy Framework

        system = platform.system()
        if (system == "Windows"):
            self.asfdll = loadLibrary('AsirikuyFrameworkAPI')
        elif (system == "Linux"):
            self.asfdll = loadLibrary('libAsirikuyFrameworkAPI.so')
        elif (system == "Darwin"):
            self.asfdll = loadLibrary('libAsirikuyFrameworkAPI.dylib')
        else:
            print "No shared library loading support for OS %s" % (system)

        self.name = name
        self.initialBalance = initialBalance
        self.depositSymbol = depositSymbol
        self.brokerName = brokerName
        self.refBrokerName = ''
        self.login = login
        self.password = password
        self.useOrderWrapper = useOrderWrapper
        self.hasStaticIP = hasStaticIP
        self.openTradesCount = 0
        self.openTrades = None
        self.virtualOpenTradesCount = 0
        self.totalHistoryTradesCount = 0
        self.totalOpenTradesCount = 0
        self.totalVirtualOpenTradesCount = 0
        self.currentBrokerTime = 0
        self.virtualEquity = 0
        self.virtualBalance = 0
        
        if name == None or brokerName == None or login == None or password == None or useOrderWrapper == None or hasStaticIP == None: raise ValueError(
            'Values not valid to initialize Account')

        #Account info values
        self.accountInfo = accountInfoType()
        try:
            self.accountInfo[IDX_ACCOUNT_NUMBER] = float(login)
        except:
            self.accountInfo[IDX_ACCOUNT_NUMBER] = 111111 #Parche dukascopy. Arreglar!!!!
        self.accountInfo[IDX_CONTRACT_SIZE] = 100000
        self.accountInfo[IDX_TOTAL_OPEN_TRADE_RISK_PERCENT] = 0 #UI Values. Ignore
        self.accountInfo[IDX_LARGEST_DRAWDOWN_PERCENT] = 0 #UI Values. Ignore
        self.getAccountInfo(allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)

    def runStrategies(self, allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded, symbol=None):
        self.getAccountInfo(allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)    
        for strategy in self.strategies:
            if (strategy.lastRunningTime-datetime.datetime.now()).seconds > 300:
                strategy.isRunning = False
            if (symbol == None or symbol == DukascopySymbol[strategy.pair]) and not strategy.isRunning:
                strategy.isRunning = True
                strategy.run(allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)

    def displayBalance(self):
        print '-' * 91
        print "Balance: %.2f (USD)  Equity: %.2f (USD)" % (self.accountInfo[IDX_BALANCE], self.accountInfo[IDX_EQUITY])
        if self.virtualBalance != 0:
            print "Virtual Balance: %.2f (USD)  Virtual Equity: %.2f (USD)" % (self.virtualBalance, self.virtualEquity)    

    def displaySystemUI(self, instanceID):
        try:
            if os.path.exists('./tmp/' + str(instanceID) + ".ui"):
                with open('./tmp/' + str(instanceID) + ".ui", 'rb') as f:
                    reader = csv.reader(f)
                    for index, row in enumerate(reader):
                        if row[0] != " ":
                            print "%-20s%-10s" % (row[0], row[1])
        except Exception:
            e = Exception
            print sys.exc_info()[1]
    def displayVirtualTrades(self):
        print '%-10s%-20s%-5s%-21s%-8s%-8s%9s%9s%9s%15s' % ('Trade ID', 'System', 'Type', 'Open Time', 'Lots', 'Symbol', 'Price', 'SL', 'TP', 'Profit (USD)')
        trades = self.getVirtualTrades()
        for trade in trades:
            profitStyle = Fore.RED if trade['profit'] < 0 else Fore.GREEN
            if "JPY" not in trade['symbol']:
                print '%-10s%-20s%-5s%-21s%-8.5f%-8s%9.5f%9.5f%9.5f%s%11.2f' % (
                    int(trade['id']),
                    trade['strategy_name'],
                    trade['direction'],
                    trade['open'],
                    trade['lots'],
                    trade['symbol'],
                    trade['price'],
                    trade['sl'],
                    trade['tp'],
                    profitStyle,
                    trade['profit']
                )
            else:
                print '%-10s%-20s%-5s%-21s%-8.5f%-8s%9.2f%9.2f%9.2f%s%11.2f' % (
                    int(trade['id']),
                    trade['strategy_name'],
                    trade['direction'],
                    trade['open'],
                    trade['lots'],
                    trade['symbol'],
                    trade['price'],
                    trade['sl'],
                    trade['tp'],
                    profitStyle,
                    trade['profit']
                )
            #trade.id, trade.strategyName, trade.direction, trade.open, trade.lots, trade.symbol,
            #                                                                trade.price, trade.sl, trade.tp, trade.profit)


    def getVirtualTrades(self):
        trades = []
        try:
            db = TinyDB('./data/' + str(self.accountID) + "_openOrders.json")
            all_trades = db.all()
            
            for saved_trade in all_trades:
                trade = type('', (), {})
                orderType = saved_trade['Type']
                if orderType == SELL:
                    direction = 'SELL'
                if orderType == SELLSTOP:
                    direction = 'SELLSTOP'
                if orderType == SELLLIMIT:
                    direction = 'SELLLIMIT'
                if orderType == BUY:
                    direction = 'BUY'
                if orderType == BUYSTOP:
                    direction = 'BUYSTOP'
                if orderType == BUYLIMIT:
                    direction = 'BUYLIMIT'

                for strategy in self.strategies:
                    if strategy.instanceID == float(saved_trade['InstanceID']):
                        strategyName = strategy.name

                profit = float(saved_trade['Profit'])
                symbol = saved_trade['Symbol']
                symbolList=list(symbol)
                baseName=symbolList[0]+symbolList[1]+symbolList[2]
                termName=symbolList[3]+symbolList[4]+symbolList[5]

                trade = {'id': float(saved_trade['Ticket']), 'strategy_name': strategyName, 'strategy_id': int(float(saved_trade['InstanceID'])), 'direction': direction, 'open': datetime.datetime.fromtimestamp(float(saved_trade['OpenTime'])).strftime('%Y-%m-%d %H:%M:%S'),
                                     'lots': float(saved_trade['Volume']),'symbol': saved_trade['Symbol'], 'price': float(saved_trade['OpenPrice']), 'sl': float(saved_trade['StopLoss']), 'tp': float(saved_trade['TakeProfit']), 'profit': profit}

                trades.append(trade)


        except Exception:
            e = Exception
            print sys.exc_info()[1]

        return trades

    def getVirtualHistoryTrades(self):
        
        historyTrades = []
        
        try:
            db = TinyDB('./data/' + str(self.accountID) + "_orderHistory.json")
            all_trades = db.all()

            #print 'testing history all trades....................'
            #print all_trades

            for trade in all_trades:
                direction = 'SELL' if float(trade['Type']) == 1 else 'BUY'

                strategyName = trade['InstanceID']
                for strategy in self.strategies:
                    if strategy.instanceID == float(trade['InstanceID']):
                        strategyName = strategy.name
                                    
                symbol = trade['Symbol']
                symbolList=list(symbol)
                baseName=symbolList[0]+symbolList[1]+symbolList[2]
                termName=symbolList[3]+symbolList[4]+symbolList[5]

                historyTrade = {'id': float(trade['Ticket']), 'strategy_name': strategyName, 'strategy_id': int(float(trade['InstanceID'])), 'direction': direction, 'openTime': datetime.datetime.fromtimestamp(float(trade['OpenTime'])).strftime('%Y-%m-%d %H:%M:%S')
                                     , 'closeTime': datetime.datetime.fromtimestamp(float(trade['CloseTime'])).strftime('%Y-%m-%d %H:%M:%S'), 'lots': float(trade['Volume']),'symbol': trade['Symbol'], 'closePrice': float(trade['ClosePrice']), 'openPrice': float(trade['OpenPrice']), 'sl': float(trade['StopLoss']), 'tp': float(trade['TakeProfit']), 'profit':  float(trade['Profit']), 'profitPL':  float(trade['ProfitPL'])}

                #print 'testing .............................................................'
                #print historyTrade
                historyTrades.append(historyTrade)


        except Exception:
            e = Exception
            print sys.exc_info()[1]

        return historyTrades

    def getVirtualHistoryAnalysis(self):
        
        strategyAnalysis = []
        print "VIRTUAL ANALYSIS START"

        try:
            for strategy in self.strategies:
          
                singleStrategyAnalysis = type('', (), {})
                totalPL = 0
                instancePL = 0
                instancePL_percentage = 0
                maxBalance = 100
                instanceBalance = 100
                drawdown = 0
                maxDrawdown = 0
                symbol = strategy.pair
                symbolList=list(symbol)
                baseName=symbolList[0]+symbolList[1]+symbolList[2]
                termName=symbolList[3]+symbolList[4]+symbolList[5]

                db = TinyDB('./data/' + str(self.accountID) + "_orderHistory.json")
                all_trades = db.all()

                for trade in all_trades:

                    totalPL += float(trade['Profit'])
                                                             
                    if strategy.instanceID == float(trade['InstanceID']):

                        instancePL += float(trade['Profit'])
                        instancePL_percentage += float(trade['ProfitPL'])*100
                        instanceBalance += float(trade['ProfitPL'])*instanceBalance
                                    
                        if instanceBalance > maxBalance:
                            maxBalance = instanceBalance
                        if instanceBalance < maxBalance:
                            drawdown = 100-100*(instanceBalance/maxBalance)
                        if drawdown > maxDrawdown:
                            maxDrawdown = drawdown

                singleStrategyAnalysis = {'strategyName': strategy.name, 'strategy_id': strategy.instanceID, 'instancePL': instancePL, 'instancePL_percentage': instancePL_percentage, 'maxDrawdown': maxDrawdown}
                strategyAnalysis.append(singleStrategyAnalysis)
                   
        except Exception:
            e = Exception
            print sys.exc_info()[1]

        return strategyAnalysis
        

    def displayVirtualHistoryAnalysis(self):
        try:
            print '%-20s%-20s%-20s%-10s' % ('System', 'Absolute P/L (USD)', '%P/L', 'MaxDrawdown')
            for strategy in self.strategies:
                
                totalPL = 0
                instancePL = 0
                instancePL_percentage = 0
                maxBalance = 100
                instanceBalance = 100
                drawdown = 0
                maxDrawdown = 0
                symbol = strategy.pair
                symbolList=list(symbol)
                baseName=symbolList[0]+symbolList[1]+symbolList[2]
                termName=symbolList[3]+symbolList[4]+symbolList[5]

                db = TinyDB('./data/' + str(self.accountID) + "_orderHistory.json")
                all_trades = db.all()

                for trade in all_trades:
                    totalPL += float(trade['Profit'])
                                                             
                    if strategy.instanceID == float(trade['InstanceID']):

                        instancePL += float(trade['Profit'])
                        instancePL_percentage += 100*float(trade['ProfitPL'])
                        instanceBalance += float(trade['ProfitPL'])*instanceBalance
                                    
                        if instanceBalance > maxBalance:
                            maxBalance = instanceBalance
                        if instanceBalance < maxBalance:
                            drawdown = 100-100*(instanceBalance/maxBalance)
                        if drawdown > maxDrawdown:
                            maxDrawdown = drawdown

                absolutProfitStyle = Fore.RED if instancePL < 0 else Fore.GREEN
                profitStyle = Fore.RED if instancePL_percentage < 0 else Fore.GREEN
                print '%-20s%s%-20f%s%-20f%s%-15f' % (strategy.name, absolutProfitStyle, instancePL, profitStyle, instancePL_percentage, Fore.WHITE, maxDrawdown)

        except Exception:
            e = Exception
            print sys.exc_info()[1]

class OandaAccount(Account):
    def __init__(self, depositSymbol, initialBalance, name=None, brokerName=None, login=None, password=None, useOrderWrapper=None, hasStaticIP=None, isDemo=True, accountID=None, logger=None):

        logger.info('Starting account init process...')
        
        self.isDemo = isDemo
        self.login = login
        self.password = password
        self.accountID = accountID
        self.error = None
        self.logger = logger
        token =  "Bearer %s" % (self.password)
        
        if self.isDemo:
            self.server = "api-fxpractice.oanda.com"
        else:
            self.server = "api-fxtrade.oanda.com"
            
        self.headers = {'Authorization': token, 'Connection': 'Keep-Alive',  'Accept-Encoding': 'gzip, deflate', 'Content-type': 'application/x-www-form-urlencoded'}
            
        Account.__init__(self, depositSymbol, initialBalance, name=name, brokerName=brokerName, login=login, password=password,
                         useOrderWrapper=useOrderWrapper, hasStaticIP=hasStaticIP, isDemo=isDemo)

        logger.info('Account init process done...')
        
    def getAccountInfo(self, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
        self.logger.info('Requesting account info...')
        url = "https://" + self.server + "/v1/accounts/%s" % (self.accountID)
        
        try:
            req = requests.get(url, headers = self.headers)
            resp = req.json()
            if req.status_code != 200:
                self.logger.error(resp)
        except Exception:
            e = Exception
            print sys.exc_info()[1]
            self.error = e
            return

        self.openTradesCount = resp['openTrades']
        self.accountCurrency = resp['accountCurrency']
        self.accountInfo[IDX_STOPOUT_PERCENT] = float(resp['marginRate'])
        self.accountInfo[IDX_MARGIN] = float(resp['marginAvail'])
        self.accountInfo[IDX_BALANCE] = float(resp['balance'])
        self.accountInfo[IDX_EQUITY] = float(resp['unrealizedPl'])+float(resp['balance'])

        if self.accountCurrency != "USD":
	    if self.accountCurrency == "JPY":
		BidAskQueryQuote = self.getCustomBidAsk("USDJPY", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)  
		BidAskQuote = BidAskQueryQuote['BidAsk']   
		self.accountInfo[IDX_BALANCE] /= BidAskQuote[0]
		self.accountInfo[IDX_EQUITY] /= BidAskQuote[0]
	    if self.accountCurrency == "CHF":
		BidAskQueryQuote = self.getCustomBidAsk("USDCHF", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded) 
		BidAskQuote = BidAskQueryQuote['BidAsk']    
		self.accountInfo[IDX_BALANCE] /= BidAskQuote[0]
		self.accountInfo[IDX_EQUITY] /= BidAskQuote[0]
	    if self.accountCurrency == "EUR":
		BidAskQueryQuote = self.getCustomBidAsk("EURUSD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)    
		BidAskQuote = BidAskQueryQuote['BidAsk'] 
		self.accountInfo[IDX_BALANCE] *= BidAskQuote[0]
		self.accountInfo[IDX_EQUITY] *= BidAskQuote[0]
	    if self.accountCurrency == "GBP":
		BidAskQueryQuote = self.getCustomBidAsk("GBPUSD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded) 
		BidAskQuote = BidAskQueryQuote['BidAsk']    
		self.accountInfo[IDX_BALANCE] *= BidAskQuote[0]
		self.accountInfo[IDX_EQUITY] *= BidAskQuote[0]
            if self.accountCurrency == "CAD":
		BidAskQueryQuote = self.getCustomBidAsk("USDCAD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)     
		BidAskQuote = BidAskQueryQuote['BidAsk']
		self.accountInfo[IDX_BALANCE] /= BidAskQuote[0]
		self.accountInfo[IDX_EQUITY] /= BidAskQuote[0]
	    if self.accountCurrency == "AUD":
		BidAskQueryQuote = self.getCustomBidAsk("AUDUSD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)   
		BidAskQuote = BidAskQueryQuote['BidAsk']  
		self.accountInfo[IDX_BALANCE] *= BidAskQuote[0]
		self.accountInfo[IDX_EQUITY] *= BidAskQuote[0]

        
        self.accountInfo[IDX_LEVERAGE] = 50
        self.logger.info('Finished getting account info.')

    def getCandles(self, numCandles, pair, timeframe, allSymbolsLoaded, allRatesLoaded, isPlotter=False):
        global OandaSymbol
        savedRateIndex = -1
        currentTime = datetime.datetime.now(dateutil.tz.gettz('Europe/Madrid'))        

        if pair+str(timeframe) in allSymbolsLoaded:
            count = 10
            savedRateIndex = allSymbolsLoaded.index(pair+str(timeframe))
            if (currentTime.hour == 16) and (currentTime.minute == 0):
                count = 500
        else:
            count = 500
        
        self.logger.info('Getting {0} bars of trading history for {1} in timeframe {2}'.format(numCandles, pair, timeframe))
        self.logger.debug("Symbol = %s, bars needed = %s, base TF = %s", OandaSymbol[pair], str(numCandles),str(OandaTF[timeframe]))
        url = "https://" + self.server + "/v1/candles?instrument=" + OandaSymbol[pair] + "&count="+str(count)+"&granularity=" + OandaTF[timeframe]

        try:
            req = requests.get(url, headers = self.headers)
            resp = req.json()
            if req.status_code != 200:
                self.logger.error(resp)
        except Exception:
            e = Exception
            self.logger.critical('Error getting Rates from URL %s: %s', type(e), sys.exc_info()[1])
            quit()
            return

        try:
            candles = resp['candles']
    
            #put rates into a proper rates array
            RatesType = Rate * numCandles
            rates = RatesType()

            for i in range(0, numCandles-1):
                rates[i].time = 0
        
            i = 0
            for candle in reversed(candles):
                timestamp = int(rfc3339.FromTimestamp(candle['time']))

                if (datetime.datetime.utcfromtimestamp(timestamp).isoweekday() < 6) and (i < numCandles):
                    rates[numCandles-1-i].time = timestamp
                    rates[numCandles-1-i].open = float(candle['openBid'])
                    rates[numCandles-1-i].high = float(candle['highBid'])
                    rates[numCandles-1-i].low = float(candle['lowBid'])
                    rates[numCandles-1-i].close = float(candle['closeBid'])
                    rates[numCandles-1-i].volume = 4
                    i = i + 1

            totalCandlesLoaded = i
            neededSupplementaryCandles = numCandles-totalCandlesLoaded
            matchingCandleIndex = -1
        
            if neededSupplementaryCandles > 0:
                if savedRateIndex == -1 or ((currentTime.hour == 16) and (currentTime.minute == 0)):
                    result = loadRates('./history/' + pair + '_' + str(timeframe) + '.csv')
                    ratesTest = result['rates']
                else:
                    ratesTest = copy.copy(allRatesLoaded[savedRateIndex])
                

                for i in range(len(ratesTest)-1, 0, -1):
                    if ratesTest[i].time <= rates[numCandles-1-totalCandlesLoaded+1].time:
                        matchingCandleIndex = i
			break

                if matchingCandleIndex == -1:
                    self.logger.critical('Error, there is no matching index between broker data and historical data. Check that your historical data csv is up to date. If your data is not up to date use data from the DATA_NST zip file available in the Asirikuy backtesting data section (compatible from v0.37).')
                    print 'Error, there is no matching index between broker data and historical data. Check that your historical data csv is up to date. If your data is not up to date use data from the DATA_NST zip file available in the Asirikuy backtesting data section (compatible from v0.37)'
                    self.error = 1001
                    sleep(10)
                    return

                for i in range(0, neededSupplementaryCandles):
                    if rates[i].time == 0:
                       rates[i] = ratesTest[matchingCandleIndex-neededSupplementaryCandles+i]

                
                if (matchingCandleIndex-neededSupplementaryCandles) > 30000:
                    with open('./history/' + pair + '_' + str(timeframe) + '.csv', 'wb') as f:
                        spamwriter  = csv.writer(f, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)
                        for i in range((matchingCandleIndex-neededSupplementaryCandles)-30000, matchingCandleIndex-neededSupplementaryCandles):

			    timeToAdd = ratesTest[i].time
			    correctedTimeStamp = timezone('UTC').localize(datetime.datetime.utcfromtimestamp(timeToAdd))
            		    correctedTimeStamp = correctedTimeStamp.astimezone(timezone('Europe/Madrid'))
	    		    timeToAdd = int(calendar.timegm(correctedTimeStamp.timetuple()))

                            spamwriter.writerow([str(time.strftime("%d/%m/%y %H:%M", time.gmtime(timeToAdd))), str(ratesTest[i].open), str(ratesTest[i].high), str(ratesTest[i].low), str(ratesTest[i].close), str(ratesTest[i].volume)])

                        for i in range(0, numCandles-1):

			    timeToAdd = rates[i].time
			    correctedTimeStamp = timezone('UTC').localize(datetime.datetime.utcfromtimestamp(timeToAdd))
            		    correctedTimeStamp = correctedTimeStamp.astimezone(timezone('Europe/Madrid'))
	    		    timeToAdd = int(calendar.timegm(correctedTimeStamp.timetuple()))

                            spamwriter.writerow([str(time.strftime("%d/%m/%y %H:%M", time.gmtime(timeToAdd))), str(rates[i].open), str(rates[i].high), str(rates[i].low), str(rates[i].close), str(rates[i].volume)])

        except Exception:
            e = Exception
            self.logger.critical('Error processing Rates %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            self.error = e
            return

        if isPlotter == False:
            if savedRateIndex == -1:
                allSymbolsLoaded.append(pair+str(timeframe))
                allRatesLoaded.append(rates)
            else:
                allRatesLoaded[savedRateIndex]=rates
        
        self.logger.info('Finished getting trading history')
        return rates

    def displayTrades(self):
        print '%-10s%-5s%-21s%-8s%-8s%9s%9s%9s' % (
        'Trade ID', 'Type', 'Open Time', 'Lots', 'Symbol', 'Price', 'SL', 'TP')
        for trade in self.openTrades:
            ticket = 1
            print '%-10s%-5s%-21s%-8.5f%-8s%9.5f%9.5f%9.5f' % (
            ticket, trade['side'].upper(), 0,
            float(int(trade['units']) / 100000), trade['instrument'],
            float(trade['avgPrice']), 0,0)
            ticket += 1


    def getTrades(self, maxOrderArraySize, instanceID):

        self.logger.info('Getting trades for instance ID {0}'.format(instanceID))
        
        url = "https://" + self.server + "/v1/accounts/%s/positions" % (self.accountID)

        try:
            req = requests.get(url, headers = self.headers)
            resp = req.json()
            if req.status_code != 200:
                self.logger.error(resp)
        except Exception:
            e = Exception
            print sys.exc_info()[1]
            self.error = e
            return
        
        self.openTrades = resp['positions']
        self.openOrderSymbols = []

        self.openTradesCount = 0
        self.totalOpenTradesCount = len(self.openTrades)

        OrderInfoType = OrderInfo * int(maxOrderArraySize)
        self.openOrderInfo = OrderInfoType()
        
        ticket = 1

        for index, trade in enumerate(self.openTrades):
            if index < int(maxOrderArraySize):
                self.openTradesCount += 1
                self.openOrderSymbols.append(OandaSymbolReverse[str(trade['instrument'])])
                self.openOrderInfo[index].ticket = float(ticket)
                ticket += 1
                self.openOrderInfo[index].instanceId = 0
                self.openOrderInfo[index].type = -1
                if trade['side'] == "buy": self.openOrderInfo[index].type = BUY
                if trade['side'] == "sell": self.openOrderInfo[index].type = SELL
                self.openOrderInfo[index].openTime = 0
                self.openOrderInfo[index].closeTime = 0
                self.openOrderInfo[index].stopLoss = 0
                self.openOrderInfo[index].takeProfit = 0
                self.openOrderInfo[index].expiration = 0
                self.openOrderInfo[index].openPrice = float(trade['avgPrice'])
                self.openOrderInfo[index].closePrice = 0
                self.openOrderInfo[index].lots = (trade['units'] / 100000)
                self.openOrderInfo[index].profit = 0
                self.openOrderInfo[index].commission = 0
                self.openOrderInfo[index].swap = 0
                self.openOrderInfo[index].isOpen = 1
                self.logger.debug("Order ticket = %s, orderType = %s, openPrice = %s",
                                  str(self.openOrderInfo[index].ticket), str(self.openOrderInfo[index].type),
                                  str(self.openOrderInfo[index].openPrice))

        self.virtualOpenTradesCount = 0
        self.totalVirtualOpenTradesCount = 0
        VirtualOrderInfoType = OrderInfo * int(maxOrderArraySize)
        self.virtualOpenOrderInfo = VirtualOrderInfoType()

        for i in range(0, int(maxOrderArraySize)):
            self.virtualOpenOrderInfo[i].ticket = 0
            self.virtualOpenOrderInfo[i].instanceId = 0
            self.virtualOpenOrderInfo[i].type = 0
            self.virtualOpenOrderInfo[i].openTime = 0
            self.virtualOpenOrderInfo[i].openPrice = 0
            self.virtualOpenOrderInfo[i].stopLoss = 0
            self.virtualOpenOrderInfo[i].takeProfit = 0

            self.virtualOpenOrderInfo[i].lots = 0
            self.virtualOpenOrderInfo[i].isOpen = 0
            self.virtualOpenOrderInfo[i].profit = 0
            self.virtualOpenOrderInfo[i].commission = 0
            self.virtualOpenOrderInfo[i].swap = 0
            self.virtualOpenOrderInfo[i].closeTime = 0
            self.virtualOpenOrderInfo[i].expiration = 0
            self.virtualOpenOrderInfo[i].closePrice = 0

        db = TinyDB('./data/' + str(self.accountID) + "_openOrders.json")
        all_trades = db.all()
        numTrades = len(all_trades)
        self.totalVirtualOpenTradesCount = numTrades
        i = 0
    
        if len(all_trades) > 0:
            for trade in all_trades:
                if trade['InstanceID'] == instanceID and i < int(maxOrderArraySize):
                    self.virtualOpenOrderInfo[i].ticket = trade['Ticket']
                    self.virtualOpenOrderInfo[i].instanceId = trade['InstanceID']
                    self.virtualOpenOrderInfo[i].type = trade['Type']
                    self.virtualOpenOrderInfo[i].openTime = trade['OpenTime']
                    self.virtualOpenOrderInfo[i].openPrice = trade['OpenPrice']
                    self.virtualOpenOrderInfo[i].stopLoss = trade['StopLoss']
                    self.virtualOpenOrderInfo[i].takeProfit = trade['TakeProfit']
                    self.virtualOpenOrderInfo[i].lots = trade['Volume']
                    self.virtualOpenOrderInfo[i].isOpen = True
                    self.virtualOpenOrderInfo[i].profit = 0
                    self.virtualOpenOrderInfo[i].commission = 0
                    self.virtualOpenOrderInfo[i].swap = 0
                    self.virtualOpenOrderInfo[i].closeTime = 0
                    self.virtualOpenOrderInfo[i].expiration = 0
                    self.virtualOpenOrderInfo[i].closePrice = 0
                    self.logger.debug("Order ticket = %s, orderType = %s, openPrice = %s",
                                      str(self.virtualOpenOrderInfo[i].ticket),
                                      str(self.virtualOpenOrderInfo[i].type),
                                      str(self.virtualOpenOrderInfo[i].openPrice))
                    self.virtualOpenTradesCount += 1
                    i = i + 1

        db = TinyDB('./data/' + str(self.accountID) + "_orderHistory.json")
        all_trades = db.all()
        numTrades = len(all_trades)
        self.totalHistoryTradesCount = numTrades
    
        if len(all_trades) > 0:
            for trade in all_trades:
                if trade['InstanceID'] == instanceID and i < int(maxOrderArraySize):
                    self.virtualOpenOrderInfo[i].ticket = trade['Ticket']
                    self.virtualOpenOrderInfo[i].instanceId = trade['InstanceID']
                    self.virtualOpenOrderInfo[i].type = trade['Type']
                    self.virtualOpenOrderInfo[i].openTime = trade['OpenTime']
                    self.virtualOpenOrderInfo[i].openPrice = trade['OpenPrice']
                    self.virtualOpenOrderInfo[i].stopLoss = trade['StopLoss']
                    self.virtualOpenOrderInfo[i].takeProfit = trade['TakeProfit']
                    self.virtualOpenOrderInfo[i].lots = trade['Volume']
                    self.virtualOpenOrderInfo[i].isOpen = False
                    self.virtualOpenOrderInfo[i].profit = trade['Profit']
                    self.virtualOpenOrderInfo[i].commission = 0
                    self.virtualOpenOrderInfo[i].swap = 0
                    self.virtualOpenOrderInfo[i].closeTime = trade['CloseTime']
                    self.virtualOpenOrderInfo[i].expiration = 0
                    self.virtualOpenOrderInfo[i].closePrice = trade['ClosePrice']
                    self.logger.debug("(History) Order ticket = %s, orderType = %s, openPrice = %s",
                                      str(self.virtualOpenOrderInfo[i].ticket),
                                      str(self.virtualOpenOrderInfo[i].type),
                                      str(self.virtualOpenOrderInfo[i].openPrice))
                    i = i + 1
                    
        self.logger.info('Finished getting trades')
                
    def getBidAsk(self, pair, basePair, quotePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
    
        pairIndex = -1
        bidAskExpired = False
    
        if pair in allBidAskSymbols:
            pairIndex = allBidAskSymbols.index(pair)
            if (datetime.datetime.now()-allBidAskUpdateTimes[pairIndex]).total_seconds() > BID_ASK_EXPIRATION_IN_SECONDS:
                bidAskExpired = True
    
        if pairIndex == -1 or bidAskExpired == True:
                                 
            self.logger.info('Getting BidAsk for main pair {0}'.format(pair))
            url = "https://" + self.server + "/v1/prices?instruments=" + OandaSymbol[pair]

            try:
                req = requests.get(url, headers = self.headers)
                resp = req.json()
            except Exception:
                e = Exception
                print sys.exc_info()[1]
                self.error = e
                return               
          
            prices = resp['prices']
                            
        else:
            prices = allBidAskLoaded[pairIndex]
            
        self.BidAsk = BidAskType()

        for price in prices:
            self.currentBrokerTime = int(rfc3339.FromTimestamp(price['time']))
            self.BidAsk[0] = price['bid']
            self.BidAsk[1] = price['ask']

            self.BidAsk[IDX_QUOTE_CONVERSION_BID] = 0
            self.BidAsk[IDX_QUOTE_CONVERSION_ASK] = 0
            self.BidAsk[IDX_BASE_CONVERSION_BID]  = 0
            self.BidAsk[IDX_BASE_CONVERSION_ASK]  = 0
            
            if basePair != "":
                BidAskQueryBase = self.getCustomBidAsk(basePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)     
                BidAskBase = BidAskQueryBase['BidAsk']
                self.BidAsk[IDX_BASE_CONVERSION_BID] = BidAskBase[0]
                self.BidAsk[IDX_BASE_CONVERSION_ASK] = BidAskBase[1]
            else:
                self.BidAsk[IDX_BASE_CONVERSION_BID] = 1
                self.BidAsk[IDX_BASE_CONVERSION_ASK] = 1

            if quotePair != "":
                BidAskQueryQuote = self.getCustomBidAsk(quotePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)     
                BidAskQuote = BidAskQueryQuote['BidAsk']
                self.BidAsk[IDX_QUOTE_CONVERSION_BID]  = BidAskQuote[0]
                self.BidAsk[IDX_QUOTE_CONVERSION_ASK]  = BidAskQuote[1]
            else:
                self.BidAsk[IDX_QUOTE_CONVERSION_BID]  = 1
                self.BidAsk[IDX_QUOTE_CONVERSION_ASK]  = 1

            if self.BidAsk[IDX_QUOTE_CONVERSION_BID] == 0 or self.BidAsk[IDX_QUOTE_CONVERSION_ASK] == 0 or self.BidAsk[IDX_BASE_CONVERSION_BID]  == 0 or self.BidAsk[IDX_BASE_CONVERSION_ASK]  ==  0:
                raise Exception("A Bid/ask values is zero")
                
        if pair in allBidAskSymbols:
            pairIndex = allBidAskSymbols.index(pair)
                
        if pairIndex == -1:
            allBidAskSymbols.append(pair)
            allBidAskLoaded.append(prices)
            allBidAskUpdateTimes.append(datetime.datetime.now())
                
        if bidAskExpired == True:
            allBidAskLoaded[pairIndex] = prices
            allBidAskUpdateTimes[pairIndex] = datetime.datetime.now()
            

        self.logger.debug("Current bid = %s, current ask = %s, broker time %s", str(self.BidAsk[0]),
                          str(self.BidAsk[1]), str(self.currentBrokerTime))
        
        self.logger.info('Finished getting BidAsk')

    def getCustomBidAsk(self, pair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
    
        pairIndex = -1
        bidAskExpired = False
    
        if allBidAskSymbols != None:
            if pair in allBidAskSymbols:
                pairIndex = allBidAskSymbols.index(pair)
                if (datetime.datetime.now()-allBidAskUpdateTimes[pairIndex]).total_seconds() > BID_ASK_EXPIRATION_IN_SECONDS:
                    bidAskExpired = True
    
        if pairIndex == -1 or bidAskExpired == True:
        
            self.logger.info('Getting BidAsk for pair {0}'.format(pair))
            url = "https://" + self.server + "/v1/prices?instruments=" + OandaSymbol[pair]
            self.error = None
        
            try:
                req = requests.get(url, headers = self.headers)
                resp = req.json()
                if req.status_code != 200:
                    self.logger.error(resp)
            except Exception:
                e = Exception
                print sys.exc_info()[1]
                self.error = e
                return {'error': self.error}
        
            prices = resp['prices']
                
        else:
            prices = allBidAskLoaded[pairIndex]         
           
        BidAsk = BidAskType()
        
        for price in prices:
            currentBrokerTime = int(rfc3339.FromTimestamp(price['time']))
            BidAsk[0] = price['bid']
            BidAsk[1] = price['ask']
            BidAsk[2] = 1
            BidAsk[3] = 1
            BidAsk[4] = 1
            BidAsk[5] = 1          
            
        if pairIndex == -1 and allBidAskSymbols != None:
            allBidAskSymbols.append(pair)
            allBidAskLoaded.append(prices)
            allBidAskUpdateTimes.append(datetime.datetime.now())
                
        if bidAskExpired == True:
            allBidAskLoaded[pairIndex] = prices
            allBidAskUpdateTimes[pairIndex] = datetime.datetime.now()

        self.logger.info('Finished getting BidAsk')
        return {'BidAsk': BidAsk, 'error': self.error}

    def openTrade(self, lots, direction, pair, stopLoss, takeProfit, instanceID):

        if direction == BUY:
            directionString = "buy"
        if direction == SELL:
            directionString = "sell"

        if (int(lots * 100000)) > 0:
            url = "https://" + self.server + "/v1/accounts/" + str(self.accountID) + "/orders"
            data="instrument=" + OandaSymbol[pair] + "&units=" + str(int(lots * 100000)) + '&side=' + directionString + '&type=market'
        
            try:
                req = requests.post(url, headers = self.headers, data = data)
                resp = req.json()
                if req.status_code != 200:
                    self.logger.error(resp)
            except Exception:
                e = Exception
                print sys.exc_info()[1]
                self.error = e
                return

    def closeTrade(self, ticket, instanceID, orderType):
        self.logger.info("Closing order %s", str(ticket))
        url = "https://" + self.server + "/v1/accounts/" + str(self.accountID) + "/trades/" + str(ticket)

        try:
            req = requests.delete(url, headers = self.headers)
            resp = req.json()
            if req.status_code != 200:
                self.logger.error(resp)
        except Exception:
            e = Exception
            print sys.exc_info()[1]
            self.error = e
            return

        

    def modifyTrade(self, ticket, instanceID, stopLoss, takeProfit, orderType):
        url = "https://" + self.server + "/v1/accounts/" + str(self.accountID) + "/trades/" + str(ticket)
        data='stopLoss=' + str(stopLoss)

        try:
            req = requests.put(url, headers = self.headers, data = data)
            resp = req.json()
            if req.status_code != 200:
                self.logger.error(resp)
        except Exception:
            e = Exception
            print sys.exc_info()[1]
            self.error = e
            return

        data='takeProfit' + str(takeProfit)

        try:
            req = requests.put(url, headers = self.headers, data = data)
            resp = req.json()
            if req.status_code != 200:
                self.logger.error(resp)
        except Exception:
            e = Exception
            print sys.exc_info()[1]
            self.error = e
            return

class JForexAccount(Account):
    def __init__(self, depositSymbol, initialBalance, name=None, brokerName=None, login=None, password=None, useOrderWrapper=None, hasStaticIP=None, isDemo=True,  accountID=None, useStreaming = None):

        
        Asirikuy = JPackage('asirikuy')
        if brokerName == 'Dukascopy':
            url = 'https://www.dukascopy.com/client/demo/jclient/jforex.jnlp'
        elif brokerName == 'AlpariUS-JForex':
            url = 'https://demo-swfx.alpari-us.com/fo/platform/jForex'
        elif brokerName == 'FXDD-JForex-demo':
            url = 'https://demo-swfx.fxdd.com/fo/platform/jForex/'
        elif brokerName == 'FXDD-JForex-live':
            url = 'https://live-swfx.fxdd.com/fo/platform/jForex/'
        else:
            raise ValueError("Broker %s not supported in JForex Account" % brokerName)

        self.dcw = None
        self.accountID = accountID
        self.dcw = Asirikuy.DukascopyWrapper(login, password, url, hasStaticIP)
        Account.__init__(self, depositSymbol, initialBalance, name=name, brokerName=brokerName, login=login, password=password, useOrderWrapper=useOrderWrapper, hasStaticIP=hasStaticIP, isDemo=isDemo)

        print self.dcw

    def runStrategies(self, allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded, symbol=None):
        jpype.attachThreadToJVM()
        Account.runStrategies(self, allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded, symbol)

    def getAccountInfo(self, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
        if self.dcw != None:
            self.openTradesCount = len(self.dcw.dumbStrategy.engine.getOrders())
            self.accountCurrency = self.dcw.dumbStrategy.account.getCurrency().toString()
            self.accountInfo[IDX_STOPOUT_PERCENT] = self.dcw.dumbStrategy.account.getMarginCutLevel()
            self.accountInfo[IDX_BALANCE] = self.accountInfo[IDX_MARGIN] = self.dcw.dumbStrategy.account.getBaseEquity()
            self.accountInfo[IDX_EQUITY] = self.dcw.dumbStrategy.account.getEquity()

            if self.accountCurrency != "USD":
	    	if self.accountCurrency == "JPY":
		    BidAskQueryQuote = self.getCustomBidAsk("USDJPY", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)    
		    BidAskQuote = BidAskQueryQuote['BidAsk']
		    self.accountInfo[IDX_BALANCE] /= BidAskQuote[0]
		    self.accountInfo[IDX_EQUITY] /= BidAskQuote[0]
		if self.accountCurrency == "CHF":
		    BidAskQueryQuote = self.getCustomBidAsk("USDCHF", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded) 
		    BidAskQuote = BidAskQueryQuote['BidAsk']    
		    self.accountInfo[IDX_BALANCE] /= BidAskQuote[0]
		    self.accountInfo[IDX_EQUITY] /= BidAskQuote[0]
		if self.accountCurrency == "EUR":
		    BidAskQueryQuote = self.getCustomBidAsk("EURUSD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
                    BidAskQuote = BidAskQueryQuote['BidAsk']     
		    self.accountInfo[IDX_BALANCE] *= BidAskQuote[0]
		    self.accountInfo[IDX_EQUITY] *= BidAskQuote[0]
		if self.accountCurrency == "GBP":
		    BidAskQueryQuote = self.getCustomBidAsk("GBPUSD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)     
		    BidAskQuote = BidAskQueryQuote['BidAsk']
		    self.accountInfo[IDX_BALANCE] *= BidAskQuote[0]
		    self.accountInfo[IDX_EQUITY] *= BidAskQuote[0]
		if self.accountCurrency == "CAD":
		    BidAskQueryQuote = self.getCustomBidAsk("USDCAD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)   
		    BidAskQuote = BidAskQueryQuote['BidAsk']  
		    self.accountInfo[IDX_BALANCE] /= BidAskQuote[0]
		    self.accountInfo[IDX_EQUITY] /= BidAskQuote[0]
		if self.accountCurrency == "AUD":
		    BidAskQueryQuote = self.getCustomBidAsk("AUDUSD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)  
		    BidAskQuote = BidAskQueryQuote['BidAsk']   
		    self.accountInfo[IDX_BALANCE] *= BidAskQuote[0]
		    self.accountInfo[IDX_EQUITY] *= BidAskQuote[0]
            
            self.accountInfo[IDX_LEVERAGE] = self.dcw.dumbStrategy.account.getLeverage()

    def getCandles(self, numCandles, pair, timeframe, allSymbolsLoaded, allRatesLoaded, isPlotter=False):
        candles = self.dcw.dumbStrategy.getCandles(DukascopyTF[timeframe], DukascopySymbol[pair], numCandles)
        RatesType = Rate * numCandles
        rates = RatesType()
        for index, candle in enumerate(candles):
            rates[index].time = int(candle.getTime() / 1000)
            rates[index].open = candle.getOpen()
            rates[index].high = candle.getHigh()
            rates[index].low = candle.getLow()
            rates[index].close = candle.getClose()
            rates[index].volume = candle.getVolume()
        return rates

    def displayTrades(self):
        print '%-10s%-10s%-5s%-21s%-8s%-8s%9s%9s%9s%11s' % (
        'Trade ID', 'Instance ID', 'Type', 'Open Time', 'Lots', 'Symbol', 'Price', 'SL', 'TP', 'Profit')
        for trade in self.openTrades:
            profitStyle = Fore.RED if trade.getProfitLossInAccountCurrency() < 0 else Fore.GREEN
            print '%-10s%-10s%-5s%-21s%-8.5f%-8s%9.5f%9.5f%9.5f%s%11.2f' % (
            trade.getId(), trade.getComment(), 'BUY' if trade.isLong() else 'SELL',
            datetime.datetime.fromtimestamp(trade.getCreationTime() / 1000).strftime('%Y-%m-%d %H:%M:%S'),
            trade.getAmount() * 10, trade.getInstrument().toString(), trade.getOpenPrice(), trade.getStopLossPrice(),
            trade.getTakeProfitPrice(),
            profitStyle, trade.getProfitLossInAccountCurrency())

    def displayVirtualTrades(self):
        print '%-10s%-5s%-21s%-8s%-8s%9s%9s%9s%11s' % (
        'Trade ID', 'Type', 'Open Time', 'Lots', 'Symbol', 'Price', 'SL', 'TP', 'Profit')

        db = TinyDB('./data/' + str(self.accountID) + "_orderHistory.json")
        all_trades = db.all()

        for trade in all_trades:
            direction = 'SELL' if float(trade['Type']) == 1 else 'BUY'
            profitStyle = Fore.RED if float(trade['Profit']) < 0 else Fore.GREEN
            print '%-10s%-5s%-21s%-8.5f%-8s%9.5f%9.5f%9.5f%s%11.2f' % (trade['Ticket'], direction,
                                                                                   datetime.datetime.fromtimestamp(
                                                                                       float(trade['OpenTime'])).strftime(
                                                                                       '%Y-%m-%d %H:%M:%S'),
                                                                                   float(trade['Volume']), trade['Symbol'], float(trade['OpenPrice']),
                                                                                   float(trade['StopLoss']), float(trade['TakeProfit']),
                                                                                   profitStyle, float(trade['Profit']))

    def getTrades(self, maxOrderArraySize, instanceID):

        self.openTrades = self.dcw.dumbStrategy.engine.getOrders()
        self.totalOpenTradesCount = len(self.openTrades) # these are the total trades for the account
        self.openTradesCount = 0 # these are the total trades for this instance ID
        self.openOrderSymbols = []

        OrderInfoType = OrderInfo * int(maxOrderArraySize)
        self.openOrderInfo = OrderInfoType()

        for i in range(0, int(maxOrderArraySize)):
            self.openOrderInfo[i].ticket = 0
            self.openOrderInfo[i].instanceId = 0
            self.openOrderInfo[i].type = 0
            self.openOrderInfo[i].openTime = 0
            self.openOrderInfo[i].openPrice = 0
            self.openOrderInfo[i].stopLoss = 0
            self.openOrderInfo[i].takeProfit = 0
            self.openOrderInfo[i].lots = 0
            self.openOrderInfo[i].isOpen = 0
            self.openOrderInfo[i].profit = 0
            self.openOrderInfo[i].commission = 0
            self.openOrderInfo[i].swap = 0
            self.openOrderInfo[i].closeTime = 0
            self.openOrderInfo[i].expiration = 0
            self.openOrderInfo[i].closePrice = 0

        for index, trade in enumerate(self.openTrades):
            if index < int(maxOrderArraySize) and (trade.getComment() == str(instanceID) or self.useOrderWrapper):
                self.openTradesCount += 1
                self.openOrderSymbols.append(DukascopySymbolReverse[str(trade.getInstrument())])
                self.openOrderInfo[index].ticket = float(trade.getId())

                if self.useOrderWrapper:
                    self.openOrderInfo[index].instanceId = 0
                else:
                    self.openOrderInfo[index].instanceId = float(trade.getComment())

                self.openOrderInfo[index].type = -1

                if trade.isLong():
                    self.openOrderInfo[index].type = BUY
                else:
                    self.openOrderInfo[index].type = SELL

                self.openOrderInfo[index].openTime = trade.getCreationTime()
                self.openOrderInfo[index].closeTime = 0
                self.openOrderInfo[index].stopLoss = trade.getStopLossPrice()
                self.openOrderInfo[index].takeProfit = trade.getTakeProfitPrice()
                self.openOrderInfo[index].expiration = 0
                self.openOrderInfo[index].openPrice = trade.getOpenPrice()
                self.openOrderInfo[index].closePrice = 0
                self.openOrderInfo[index].lots = trade.getAmount() * 10
                self.openOrderInfo[index].profit = 0
                self.openOrderInfo[index].commission = trade.getCommission()
                self.openOrderInfo[index].swap = 0
                self.openOrderInfo[index].isOpen = 1
                self.logger.debug("Order ticket = %s, orderType = %s, openPrice = %.5f", trade.getId(),
                                  'BUY' if trade.isLong() else 'SELL', trade.getOpenPrice())

        self.virtualOpenTradesCount = 0
        self.totalVirtualOpenTradesCount = 0

        VirtualOrderInfoType = OrderInfo * int(maxOrderArraySize)
        self.virtualOpenOrderInfo = VirtualOrderInfoType()

        for i in range(0, int(maxOrderArraySize)):
            self.virtualOpenOrderInfo[i].ticket = 0
            self.virtualOpenOrderInfo[i].instanceId = 0
            self.virtualOpenOrderInfo[i].type = 0
            self.virtualOpenOrderInfo[i].openTime = 0
            self.virtualOpenOrderInfo[i].openPrice = 0
            self.virtualOpenOrderInfo[i].stopLoss = 0
            self.virtualOpenOrderInfo[i].takeProfit = 0
            self.virtualOpenOrderInfo[i].lots = 0
            self.virtualOpenOrderInfo[i].isOpen = 0
            self.virtualOpenOrderInfo[i].profit = 0
            self.virtualOpenOrderInfo[i].commission = 0
            self.virtualOpenOrderInfo[i].swap = 0
            self.virtualOpenOrderInfo[i].closeTime = 0
            self.virtualOpenOrderInfo[i].expiration = 0
            self.virtualOpenOrderInfo[i].closePrice = 0

        db = TinyDB('./data/' + str(self.accountID) + "_openOrders.json")
        all_trades = db.all()
        numTrades = len(all_trades)
        self.totalVirtualOpenTradesCount = numTrades
        i = 0

        if len(all_trades) > 0:
            for trade in all_trades:
                if trade['InstanceID'] == instanceID and i < int(maxOrderArraySize):
                    self.virtualOpenOrderInfo[i].ticket = trade['Ticket']
                    self.virtualOpenOrderInfo[i].instanceId = trade['InstanceID']
                    self.virtualOpenOrderInfo[i].type = trade['Type']
                    self.virtualOpenOrderInfo[i].openTime = trade['OpenTime']
                    self.virtualOpenOrderInfo[i].openPrice = trade['OpenPrice']
                    self.virtualOpenOrderInfo[i].stopLoss = trade['StopLoss']
                    self.virtualOpenOrderInfo[i].takeProfit = trade['TakeProfit']
                    self.virtualOpenOrderInfo[i].lots = trade['Volume']
                    self.virtualOpenOrderInfo[i].isOpen = True
                    self.virtualOpenOrderInfo[i].profit = 0
                    self.virtualOpenOrderInfo[i].commission = 0
                    self.virtualOpenOrderInfo[i].swap = 0
                    self.virtualOpenOrderInfo[i].closeTime = 0
                    self.virtualOpenOrderInfo[i].expiration = 0
                    self.virtualOpenOrderInfo[i].closePrice = 0
                    self.logger.debug("Order ticket = %s, orderType = %s, openPrice = %s",
                                      str(self.virtualOpenOrderInfo[i].ticket),
                                      str(self.virtualOpenOrderInfo[i].type),
                                      str(self.virtualOpenOrderInfo[i].openPrice))
                    self.virtualOpenTradesCount += 1
                    i = i + 1

        db = TinyDB('./data/' + str(self.accountID) + "_orderHistory.json")
        all_trades = db.all()
        numTrades = len(all_trades)
        self.totalHistoryTradesCount = numTrades

        if len(all_trades) > 0:
            for trade in all_trades:
                if trade['InstanceID'] == instanceID and i < int(maxOrderArraySize):
                    self.virtualOpenOrderInfo[i].ticket = trade['Ticket']
                    self.virtualOpenOrderInfo[i].instanceId = trade['InstanceID']
                    self.virtualOpenOrderInfo[i].type = trade['Type']
                    self.virtualOpenOrderInfo[i].openTime = trade['OpenTime']
                    self.virtualOpenOrderInfo[i].openPrice = trade['OpenPrice']
                    self.virtualOpenOrderInfo[i].stopLoss = trade['StopLoss']
                    self.virtualOpenOrderInfo[i].takeProfit = trade['TakeProfit']
                    self.virtualOpenOrderInfo[i].lots = trade['Volume']
                    self.virtualOpenOrderInfo[i].isOpen = False
                    self.virtualOpenOrderInfo[i].profit = trade['Profit']
                    self.virtualOpenOrderInfo[i].commission = 0
                    self.virtualOpenOrderInfo[i].swap = 0
                    self.virtualOpenOrderInfo[i].closeTime = trade['CloseTime']
                    self.virtualOpenOrderInfo[i].expiration = 0
                    self.virtualOpenOrderInfo[i].closePrice = trade['ClosePrice']
                    self.logger.debug("(History) Order ticket = %s, orderType = %s, openPrice = %s",
                                      str(self.virtualOpenOrderInfo[i].ticket),
                                      str(self.virtualOpenOrderInfo[i].type),
                                      str(self.virtualOpenOrderInfo[i].openPrice))
                    i = i + 1

        self.logger.info('Finished getting trades')

    def getBidAsk(self, pair, basePair, quotePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
        global DukascopySymbol
        self.pair = pair
        bidAsk = self.dcw.dumbStrategy.getBidAsk(DukascopySymbol[pair])
        self.currentBrokerTime = self.dcw.dumbStrategy.tick.getTime() / 1000
        self.BidAsk = BidAskType()
        self.BidAsk[0] = bidAsk[0].doubleValue()
        self.BidAsk[1] = bidAsk[1].doubleValue()

        self.BidAsk[IDX_QUOTE_CONVERSION_BID] = 0
        self.BidAsk[IDX_QUOTE_CONVERSION_ASK] = 0
        self.BidAsk[IDX_BASE_CONVERSION_BID]  = 0
        self.BidAsk[IDX_BASE_CONVERSION_ASK]  = 0
            
        if basePair != "":
            BidAskQueryBase = self.getCustomBidAsk(basePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)     
            BidAskBase = BidAskQueryBase['BidAsk']
            self.BidAsk[IDX_BASE_CONVERSION_BID] = BidAskBase[0]
            self.BidAsk[IDX_BASE_CONVERSION_ASK] = BidAskBase[1]
        else:
            self.BidAsk[IDX_BASE_CONVERSION_BID] = 1
            self.BidAsk[IDX_BASE_CONVERSION_ASK] = 1

        if quotePair != "":
            BidAskQueryQuote = self.getCustomBidAsk(quotePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)     
            BidAskQuote = BidAskQueryQuote['BidAsk']
            self.BidAsk[IDX_QUOTE_CONVERSION_BID]  = BidAskQuote[0]
            self.BidAsk[IDX_QUOTE_CONVERSION_ASK]  = BidAskQuote[1]
        else:
            self.BidAsk[IDX_QUOTE_CONVERSION_BID]  = 1
            self.BidAsk[IDX_QUOTE_CONVERSION_ASK]  = 1

        if self.BidAsk[IDX_QUOTE_CONVERSION_BID] == 0 or self.BidAsk[IDX_QUOTE_CONVERSION_ASK] == 0 or self.BidAsk[IDX_BASE_CONVERSION_BID]  == 0 or self.BidAsk[IDX_BASE_CONVERSION_ASK]  ==  0:
            raise Exception("A Bid/ask values is zero")
        
        self.logger.debug("Current bid = %s, current ask = %s, broker time %s", str(self.BidAsk[0]),
                          str(self.BidAsk[1]), str(self.currentBrokerTime))

    def getCustomBidAsk(self, pair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
        global DukascopySymbol
        bidAsk = self.dcw.dumbStrategy.getBidAsk(DukascopySymbol[pair])
        BidAsk = BidAskType()
        BidAsk[0] = bidAsk[0].doubleValue()
        BidAsk[1] = bidAsk[1].doubleValue()
        BidAsk[2] = 1
        BidAsk[3] = 1
        BidAsk[4] = 1
        BidAsk[5] = 1
        error = None

        return {'BidAsk': BidAsk, 'error': error}


    def openTrade(self, lots, direction, pair, stopLoss, takeProfit, instanceID):
        global DukascopySymbol
        lots = lots / 10

        if direction != BUY and direction != SELL:
            print "Dukascopy frontend presently does not support pending orders"
            return

        if takeProfit == 0: takeProfit = self.BidAsk[IDX_BID] / 2
        if stopLoss == 0: stopLoss = self.BidAsk[IDX_BID] / 2

        if direction == SELL:
            directionString = "sell"
            takeProfit = self.BidAsk[IDX_BID] - takeProfit
            stopLoss = self.BidAsk[IDX_BID] + stopLoss
        if direction == BUY:
            directionString = "buy"
            takeProfit = self.BidAsk[IDX_ASK] + takeProfit
            stopLoss = self.BidAsk[IDX_ASK] - stopLoss

        if instanceID == 0: instanceID = -1

        print "Opening %s trade" % directionString
        label = 'ID%d' % time.time()
        self.dcw.dumbStrategy.openOrder(label, lots, directionString, DukascopySymbol[pair], stopLoss, takeProfit, -1.0,
                                        str(instanceID));
        timeoutTries = 0
        message = ''
        result = self.dcw.dumbStrategy.checkOrder(label, message)
        while result == -1:
            timeoutTries = + 1
            if timeoutTries > 50: break
            result = self.dcw.dumbStrategy.checkOrder(label, message)
            if result == 0:
                raise ValueError('Error opening trade')

    def closeTrade(self, ticket, instanceID, orderType):
        print "Closing trade ticket %d" % ticket
        if ticket != -1:
            self.dcw.dumbStrategy.closeOrder(str(ticket))
            timeoutTries = 0
            message = ''
            result = self.dcw.dumbStrategy.checkOrder(str(ticket), message)
            while result == -1:
                timeoutTries = + 1
                if timeoutTries > 50: break
                result = self.dcw.dumbStrategy.checkOrder(str(ticket), message)
                if result == 0:
                    raise ValueError('Error closing trade')
        else:
            for trade in self.openTrades:
                if trade.isLong() and orderType == BUY and trade.getComment() == str(instanceID):
                    self.dcw.dumbStrategy.closeOrder(trade.getId())
                    timeoutTries = 0
                    message = ''
                    result = self.dcw.dumbStrategy.checkOrder(str(trade.getId()), message)
                    while result == -1:
                        timeoutTries = + 1
                        if timeoutTries > 50: break
                        result = self.dcw.dumbStrategy.checkOrder(str(trade.getId()), message)
                        if result == 0:
                            raise ValueError('Error closing trade')
                if not trade.isLong() and orderType == SELL and trade.getComment() == str(instanceID):
                    self.dcw.dumbStrategy.closeOrder(trade.getId())
                    timeoutTries = 0
                    message = ''
                    result = self.dcw.dumbStrategy.checkOrder(str(trade.getId()), message)
                    while result == -1:
                        timeoutTries = + 1
                        if timeoutTries > 50: break
                        result = self.dcw.dumbStrategy.checkOrder(str(trade.getId()), message)
                        if result == 0:
                            raise ValueError('Error closing trade')

    def modifyTrade(self, ticket, instanceID, stopLoss, takeProfit, orderType):
        print "Modifying trade ticket %d" % ticket

        if orderType == SELL:
            takeProfit = self.BidAsk[IDX_BID] - takeProfit
            stopLoss = self.BidAsk[IDX_BID] + stopLoss
        if orderType == BUY:
            takeProfit = self.BidAsk[IDX_ASK] + takeProfit
            stopLoss = self.BidAsk[IDX_ASK] - stopLoss

        if ticket != -1:
            self.dcw.dumbStrategy.modifyOrder(str(ticket), stopLoss, takeProfit)
            timeoutTries = 0
            message = ''
            result = self.dcw.dumbStrategy.checkOrder(str(ticket), message)
            while result == -1:
                timeoutTries = + 1
                if timeoutTries > 50: break
                result = self.dcw.dumbStrategy.checkOrder(str(ticket), message)
                if result == 0:
                    raise ValueError('Error modifying trade')
        else:
            for trade in self.openTrades:
                if trade.isLong() and orderType == BUY and trade.getComment() == str(instanceID):
                    print "modifying buy"
                    self.dcw.dumbStrategy.modifyOrder(trade.getId(), stopLoss, takeProfit)
                    timeoutTries = 0
                    message = ''
                    result = self.dcw.dumbStrategy.checkOrder(str(trade.getId()), message)
                    while result == -1:
                        timeoutTries = + 1
                        if timeoutTries > 50: break
                        result = self.dcw.dumbStrategy.checkOrder(str(trade.getId()), message)
                        if result == 0:
                            raise ValueError('Error modifying trade')
                if not trade.isLong() and orderType == SELL and trade.getComment() == str(instanceID):
                    print "modifying sell"
                    self.dcw.dumbStrategy.modifyOrder(trade.getId(), stopLoss, takeProfit)
                    timeoutTries = 0
                    message = ''
                    result = self.dcw.dumbStrategy.checkOrder(str(trade.getId()), message)
                    while result == -1:
                        timeoutTries = + 1
                        if timeoutTries > 50: break
                        result = self.dcw.dumbStrategy.checkOrder(str(trade.getId()), message)
                        if result == 0:
                            raise ValueError('Error modifying trade')
	
class OandaJavaAccount(Account):
    def __init__(self, depositSymbol, initialBalance, name=None, brokerName=None, login=None, password=None, useOrderWrapper=None, hasStaticIP=None, isDemo=True, accountID=None, useStreaming = None, logger=None):
        self.login = login
        self.password = password
        self.accountID = accountID
        self.error = None       
        self.logger = logger
        self.API = jpype.JPackage('com').oanda.fxtrade.api.API
        self.isDemo = isDemo
        if self.isDemo:
            self.fxclient = self.API.createFXGame()
        else:
            self.fxclient = self.API.createFXTrade()
        self.fxclient.setTimeout(10)
        self.fxclient.setWithRateThread(True);
        self.fxclient.setWithKeepAliveThread(True)
        self.fxclient.setWithTrailingStopMonitor(False) 
        try:
            self.fxclient.login(self.login, self.password)
        except:
            print Fore.WHITE + Style.BRIGHT + Back.RED + str(sys.exc_info()[1])
            quit()
        self.me = self.fxclient.getUser();
        self.account = self.me.getAccountWithId(int(accountID))
        Account.__init__(self, depositSymbol, initialBalance, name=name, brokerName=brokerName, login=login, password=password,
                         useOrderWrapper=useOrderWrapper, hasStaticIP=hasStaticIP, isDemo=isDemo)

    def log_in(self):     
        self.API = jpype.JPackage('com').oanda.fxtrade.api.API
        if self.isDemo:
            self.fxclient = self.API.createFXGame()
        else:
            self.fxclient = self.API.createFXTrade()
        self.fxclient.setTimeout(10)
        self.fxclient.setWithRateThread(True);
        self.fxclient.setWithKeepAliveThread(True)
        self.fxclient.setWithTrailingStopMonitor(False)
        try:
            self.fxclient.login(self.login, self.password)
        except:
            print Fore.WHITE + Style.BRIGHT + Back.RED + str(sys.exc_info()[1])
            quit()
        self.me = self.fxclient.getUser();
        self.account = self.me.getAccountWithId(int(self.accountID))

    def getAccountInfo(self, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
        try:
            self.me = self.fxclient.getUser();
            self.account = self.me.getAccountWithId(int(self.accountID))
            self.openTradesCount = len(self.account.getPositions())
            self.accountCurrency = self.account.getHomeCurrency()
            self.accountInfo[IDX_STOPOUT_PERCENT] = self.account.getMarginRate()
            self.accountInfo[IDX_MARGIN] = self.account.getMarginAvailable()
            self.accountInfo[IDX_BALANCE] = self.account.getBalance()
            self.accountInfo[IDX_EQUITY] = self.account.getBalance() + self.account.getUnrealizedPL()

            if self.accountCurrency != "USD":
	    	    if self.accountCurrency == "JPY":
	    	        BidAskQueryQuote = self.getCustomBidAsk("USDJPY", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
	    	        BidAskQuote = BidAskQueryQuote['BidAsk']
	    	        self.accountInfo[IDX_BALANCE] /= BidAskQuote[0]
	    	        self.accountInfo[IDX_EQUITY] /= BidAskQuote[0]
	    	    if self.accountCurrency == "CHF":
	    	        BidAskQueryQuote = self.getCustomBidAsk("USDCHF", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
	    	        BidAskQuote = BidAskQueryQuote['BidAsk']
	    	        self.accountInfo[IDX_BALANCE] /= BidAskQuote[0]
	    	        self.accountInfo[IDX_EQUITY] /= BidAskQuote[0]
	    	    if self.accountCurrency == "EUR":
	    	        BidAskQueryQuote = self.getCustomBidAsk("EURUSD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
	    	        BidAskQuote = BidAskQueryQuote['BidAsk']
	    	        self.accountInfo[IDX_BALANCE] *= BidAskQuote[0]
	    	        self.accountInfo[IDX_EQUITY] *= BidAskQuote[0]
		        if self.accountCurrency == "GBP":
		            BidAskQueryQuote = self.getCustomBidAsk("GBPUSD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)     
		            BidAskQuote = BidAskQueryQuote['BidAsk']
		            self.accountInfo[IDX_BALANCE] *= BidAskQuote[0]
		            self.accountInfo[IDX_EQUITY] *= BidAskQuote[0]
		        if self.accountCurrency == "CAD":
		            BidAskQueryQuote = self.getCustomBidAsk("USDCAD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
		            BidAskQuote = BidAskQueryQuote['BidAsk']
		            self.accountInfo[IDX_BALANCE] /= BidAskQuote[0]
		            self.accountInfo[IDX_EQUITY] /= BidAskQuote[0]
		        if self.accountCurrency == "AUD":
		            BidAskQueryQuote = self.getCustomBidAsk("AUDUSD", allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
		            BidAskQuote = BidAskQueryQuote['BidAsk']
		            self.accountInfo[IDX_BALANCE] *= BidAskQuote[0]
		            self.accountInfo[IDX_EQUITY] *= BidAskQuote[0]
            
            
            self.accountInfo[IDX_LEVERAGE] = 1 / self.account.getMarginRate()
        except Exception: 
            e = Exception
            self.logger.critical('Error getting AccountInfo %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            self.error = e
            return
            
    def getCandles(self, numCandles, pair, timeframe, allSymbolsLoaded, allRatesLoaded, isPlotter=False):
        self.logger.debug("Symbol = %s, bars needed = %s, base TF = %s", DukascopySymbol[pair], str(numCandles), str(OandaTF[timeframe]))

        savedRateIndex = -1
        currentTime = datetime.datetime.now(dateutil.tz.gettz('Europe/Madrid'))        

        if pair+str(timeframe) in allSymbolsLoaded:
            count = 10
            savedRateIndex = allSymbolsLoaded.index(pair+str(timeframe))
            if (currentTime.hour == 16) and (currentTime.minute == 0):
                count = 500
        else:
            count = 500

        try:
            p = self.API.createFXPair(DukascopySymbol[pair])
            enum = jpype.JPackage('com').oanda.fxtrade.api.CacheMode.valueOf("ALWAYS_FETCH")
            #self.logger.info('Rates caching policy is  %s', enum)
            history = self.fxclient.getRateTable().getHistory(p, timeframe * 60 * 1000, count, enum)
            #put rates into a proper rates array
            RatesType = Rate * numCandles
            rates = RatesType()
        

            for i in range(0, numCandles-1):
                rates[i].time = 0

            i = 0
        
            for candle in reversed(history):
                timestamp = int(candle.getTimestamp())
                
                if (datetime.datetime.utcfromtimestamp(timestamp).isoweekday() < 6) and (i < numCandles):                 
                    rates[numCandles-1-i].time = int(candle.getTimestamp())
                    rates[numCandles-1-i].open = float(candle.getOpen().getBid())
                    rates[numCandles-1-i].high = float(candle.getMax().getBid())
                    rates[numCandles-1-i].low = float(candle.getMin().getBid())
                    rates[numCandles-1-i].close = float(candle.getClose().getBid())
                    rates[numCandles-1-i].volume = 4
                    i += 1


            p = None
            history = None
            totalCandlesLoaded = i
            neededSupplementaryCandles = numCandles-totalCandlesLoaded
            matchingCandleIndex = -1
                            
            if neededSupplementaryCandles > 0:
                if savedRateIndex == -1 or ((currentTime.hour == 16) and (currentTime.minute == 0)):
                    result = loadRates('./history/' + pair + '_' + str(timeframe) + '.csv')
                    ratesTest = result['rates']
                else:
                    ratesTest = copy.copy(allRatesLoaded[savedRateIndex])
                

                for i in range(len(ratesTest)-1, 0, -1):
                    if ratesTest[i].time <= rates[numCandles-1-totalCandlesLoaded+1].time:
                        matchingCandleIndex = i
			break

                if matchingCandleIndex == -1:
                    self.logger.critical('Error, there is no matching index between broker data and historical data. Check that your historical data csv is up to date. If your data is not up to date use data from the DATA_NST zip file available in the Asirikuy backtesting data section (compatible from v0.37).')
                    print 'Error, there is no matching index between broker data and historical data. Check that your historical data csv is up to date. If your data is not up to date use data from the DATA_NST zip file available in the Asirikuy backtesting data section (compatible from v0.37)'
                    self.error = 1001
                    sleep(10)
                    return

                for i in range(0, neededSupplementaryCandles):
                    if rates[i].time == 0:
                       rates[i] = ratesTest[matchingCandleIndex-neededSupplementaryCandles+i]

                
                if (matchingCandleIndex-neededSupplementaryCandles) > 30000:
                    with open('./history/' + pair + '_' + str(timeframe) + '.csv', 'wb') as f:
                        spamwriter  = csv.writer(f, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)
                        for i in range((matchingCandleIndex-neededSupplementaryCandles)-30000, matchingCandleIndex-neededSupplementaryCandles):

			    timeToAdd = ratesTest[i].time
			    correctedTimeStamp = timezone('UTC').localize(datetime.datetime.utcfromtimestamp(timeToAdd))
            		    correctedTimeStamp = correctedTimeStamp.astimezone(timezone('Europe/Madrid'))
	    		    timeToAdd = int(calendar.timegm(correctedTimeStamp.timetuple()))

                            spamwriter.writerow([str(time.strftime("%d/%m/%y %H:%M", time.gmtime(timeToAdd))), str(ratesTest[i].open), str(ratesTest[i].high), str(ratesTest[i].low), str(ratesTest[i].close), str(ratesTest[i].volume)])

                        for i in range(0, numCandles-1):

			    timeToAdd = rates[i].time
			    correctedTimeStamp = timezone('UTC').localize(datetime.datetime.utcfromtimestamp(timeToAdd))
            		    correctedTimeStamp = correctedTimeStamp.astimezone(timezone('Europe/Madrid'))
	    		    timeToAdd = int(calendar.timegm(correctedTimeStamp.timetuple()))

                            spamwriter.writerow([str(time.strftime("%d/%m/%y %H:%M", time.gmtime(timeToAdd))), str(rates[i].open), str(rates[i].high), str(rates[i].low), str(rates[i].close), str(rates[i].volume)])

        except Exception: 
            e = Exception
            self.logger.critical('Error getting Rates %s: %s', type(e), sys.exc_info()[1])
            quit()
            return

        if isPlotter == False:

            if savedRateIndex == -1:
                allSymbolsLoaded.append(pair+str(timeframe))
                allRatesLoaded.append(rates)
            else:
                allRatesLoaded[savedRateIndex]=rates

        self.logger.debug('Finished getting trading history')
        return rates

    def getTrades(self, maxOrderArraySize, instanceID):

        try:
            self.openTrades = self.account.getPositions()
            self.openOrderSymbols = []

            self.openTradesCount = 0
            self.totalOpenTradesCount = len(self.openTrades)

            OrderInfoType = OrderInfo * int(maxOrderArraySize)
            self.openOrderInfo = OrderInfoType()
            ticket = 1

            for index, trade in enumerate(self.openTrades):
                if index < int(maxOrderArraySize):
                    self.openTradesCount += 1
                    self.openOrderSymbols.append(DukascopySymbolReverse[str(trade.getPair().getPair())])
                    self.openOrderInfo[index].ticket = float(ticket)
                    ticket += 1
                    self.openOrderInfo[index].instanceId = 0
                    self.openOrderInfo[index].type = -1
                    if trade.getUnits() >= 0: self.openOrderInfo[index].type = BUY
                    if trade.getUnits() < 0:  self.openOrderInfo[index].type = SELL
                    self.openOrderInfo[index].openTime  =  0
                    self.openOrderInfo[index].closeTime = 0
                    self.openOrderInfo[index].stopLoss = 0
                    self.openOrderInfo[index].takeProfit = 0
                    self.openOrderInfo[index].expiration = 0
                    self.openOrderInfo[index].openPrice = float(trade.getPrice())
                    self.openOrderInfo[index].closePrice = 0
                    self.openOrderInfo[index].lots = abs((trade.getUnits() / 100000))
                    self.openOrderInfo[index].profit = 0
                    self.openOrderInfo[index].commission = 0
                    self.openOrderInfo[index].swap = 0
                    self.openOrderInfo[index].isOpen = 1
                    self.logger.debug("Order ticket = %s, orderType = %s, openPrice = %s",
                                      str(self.openOrderInfo[index].ticket),
                                      str(self.openOrderInfo[index].type),
                                      str(self.openOrderInfo[index].openPrice))

            self.virtualOpenTradesCount = 0
            self.totalVirtualOpenTradesCount = 0
            VirtualOrderInfoType = OrderInfo * int(maxOrderArraySize)
            self.virtualOpenOrderInfo = VirtualOrderInfoType()

            for i in range(0, int(maxOrderArraySize)):
                self.virtualOpenOrderInfo[i].ticket = 0
                self.virtualOpenOrderInfo[i].instanceId = 0
                self.virtualOpenOrderInfo[i].type = 0
                self.virtualOpenOrderInfo[i].openTime = 0
                self.virtualOpenOrderInfo[i].openPrice = 0
                self.virtualOpenOrderInfo[i].stopLoss = 0
                self.virtualOpenOrderInfo[i].takeProfit = 0
                self.virtualOpenOrderInfo[i].lots = 0
                self.virtualOpenOrderInfo[i].isOpen = 0
                self.virtualOpenOrderInfo[i].profit = 0
                self.virtualOpenOrderInfo[i].commission = 0
                self.virtualOpenOrderInfo[i].swap = 0
                self.virtualOpenOrderInfo[i].closeTime = 0
                self.virtualOpenOrderInfo[i].expiration = 0
                self.virtualOpenOrderInfo[i].closePrice = 0

            db = TinyDB('./data/' + str(self.accountID) + "_openOrders.json")
            all_trades = db.all()
            numTrades = len(all_trades)
            self.totalVirtualOpenTradesCount = numTrades
            i = 0
    
            if len(all_trades) > 0:
                for trade in all_trades:
                    if trade['InstanceID'] == instanceID and i < int(maxOrderArraySize):
                        self.virtualOpenOrderInfo[i].ticket = trade['Ticket']
                        self.virtualOpenOrderInfo[i].instanceId = trade['InstanceID']
                        self.virtualOpenOrderInfo[i].type = trade['Type']
                        self.virtualOpenOrderInfo[i].openTime = trade['OpenTime']
                        self.virtualOpenOrderInfo[i].openPrice = trade['OpenPrice']
                        self.virtualOpenOrderInfo[i].stopLoss = trade['StopLoss']
                        self.virtualOpenOrderInfo[i].takeProfit = trade['TakeProfit']
                        self.virtualOpenOrderInfo[i].lots = trade['Volume']
                        self.virtualOpenOrderInfo[i].isOpen = True
                        self.virtualOpenOrderInfo[i].profit = 0
                        self.virtualOpenOrderInfo[i].commission = 0
                        self.virtualOpenOrderInfo[i].swap = 0
                        self.virtualOpenOrderInfo[i].closeTime = 0
                        self.virtualOpenOrderInfo[i].expiration = 0
                        self.virtualOpenOrderInfo[i].closePrice = 0
                        self.logger.debug("Order ticket = %s, orderType = %s, openPrice = %s",
                                          str(self.virtualOpenOrderInfo[i].ticket),
                                          str(self.virtualOpenOrderInfo[i].type),
                                          str(self.virtualOpenOrderInfo[i].openPrice))
                        self.virtualOpenTradesCount += 1
                        i = i + 1

            db = TinyDB('./data/' + str(self.accountID) + "_orderHistory.json")
            all_trades = db.all()
            numTrades = len(all_trades)
            self.totalHistoryTradesCount = numTrades
            
            if len(all_trades) > 0:
                for trade in all_trades:
                    if trade['InstanceID'] == instanceID and i < int(maxOrderArraySize):
                        self.virtualOpenOrderInfo[i].ticket = trade['Ticket']
                        self.virtualOpenOrderInfo[i].instanceId = trade['InstanceID']
                        self.virtualOpenOrderInfo[i].type = trade['Type']
                        self.virtualOpenOrderInfo[i].openTime = trade['OpenTime']
                        self.virtualOpenOrderInfo[i].openPrice = trade['OpenPrice']
                        self.virtualOpenOrderInfo[i].stopLoss = trade['StopLoss']
                        self.virtualOpenOrderInfo[i].takeProfit = trade['TakeProfit']
                        self.virtualOpenOrderInfo[i].lots = trade['Volume']
                        self.virtualOpenOrderInfo[i].isOpen = False
                        self.virtualOpenOrderInfo[i].profit = trade['Profit']
                        self.virtualOpenOrderInfo[i].commission = 0
                        self.virtualOpenOrderInfo[i].swap = 0
                        self.virtualOpenOrderInfo[i].closeTime = trade['CloseTime']
                        self.virtualOpenOrderInfo[i].expiration = 0
                        self.virtualOpenOrderInfo[i].closePrice = trade['ClosePrice']
                        self.logger.debug("(History) Order ticket = %s, orderType = %s, openPrice = %s",
                                          str(self.virtualOpenOrderInfo[i].ticket),
                                          str(self.virtualOpenOrderInfo[i].type),
                                          str(self.virtualOpenOrderInfo[i].openPrice))
                        i = i + 1
                    
        except Exception: 
            e = Exception
            self.logger.critical('Error getting Trades %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            self.error = e
            return

    def getBidAsk(self, pair, basePair, quotePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
        try:
        
            pairIndex = -1
            bidAskExpired = False
            self.BidAsk = BidAskType()
            
            if pair in allBidAskSymbols:
                pairIndex = allBidAskSymbols.index(pair)
                if (datetime.datetime.now()-allBidAskUpdateTimes[pairIndex]).total_seconds() > BID_ASK_EXPIRATION_IN_SECONDS:
                    bidAskExpired = True
                    
            self.currentBrokerTime = self.fxclient.getRateTable().getRate(self.API.createFXPair(DukascopySymbol[pair])).getTimestamp()
    
            if pairIndex == -1 or bidAskExpired == True:                             
                prices = [self.fxclient.getRateTable().getRate(self.API.createFXPair(DukascopySymbol[pair])).getBid(), self.fxclient.getRateTable().getRate(self.API.createFXPair(DukascopySymbol[pair])).getAsk()]               
            else:
                prices = allBidAskLoaded[pairIndex]
                
            self.BidAsk[0] = prices[0]
            self.BidAsk[1] = prices[1]

            self.BidAsk[IDX_QUOTE_CONVERSION_BID] = 0
            self.BidAsk[IDX_QUOTE_CONVERSION_ASK] = 0

            self.BidAsk[IDX_BASE_CONVERSION_BID]  = 0
            self.BidAsk[IDX_BASE_CONVERSION_ASK]  = 0
            
            if basePair != "":
                BidAskQueryBase = self.getCustomBidAsk(basePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)     
                BidAskBase = BidAskQueryBase['BidAsk']
                self.BidAsk[IDX_BASE_CONVERSION_BID] = BidAskBase[0]
                self.BidAsk[IDX_BASE_CONVERSION_ASK] = BidAskBase[1]
            else:
                self.BidAsk[IDX_BASE_CONVERSION_BID] = 1
                self.BidAsk[IDX_BASE_CONVERSION_ASK] = 1

            if quotePair != "":
                BidAskQueryQuote = self.getCustomBidAsk(quotePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)     
                BidAskQuote = BidAskQueryQuote['BidAsk']
                self.BidAsk[IDX_QUOTE_CONVERSION_BID]  = BidAskQuote[0]
                self.BidAsk[IDX_QUOTE_CONVERSION_ASK]  = BidAskQuote[1]
            else:
                self.BidAsk[IDX_QUOTE_CONVERSION_BID]  = 1
                self.BidAsk[IDX_QUOTE_CONVERSION_ASK]  = 1

            if self.BidAsk[IDX_QUOTE_CONVERSION_BID] == 0 or self.BidAsk[IDX_QUOTE_CONVERSION_ASK] == 0 or self.BidAsk[IDX_BASE_CONVERSION_BID]  == 0 or self.BidAsk[IDX_BASE_CONVERSION_ASK]  ==  0:
                raise Exception("A Bid/ask values is zero")
                
            if pair in allBidAskSymbols:
                pairIndex = allBidAskSymbols.index(pair)
            
            if pairIndex == -1:
                allBidAskSymbols.append(pair)
                allBidAskLoaded.append(prices)
                allBidAskUpdateTimes.append(datetime.datetime.now())
                
            if bidAskExpired == True:
                allBidAskLoaded[pairIndex] = prices
                allBidAskUpdateTimes[pairIndex] = datetime.datetime.now()

            self.logger.debug("Current bid = %s, current ask = %s, broker time %s", str(self.BidAsk[0]), str(self.BidAsk[1]), str(self.currentBrokerTime))

            
        except Exception: 
            e = Exception
            self.logger.critical('Error getting BidAsk %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            self.error = e
            return
        
    def getCustomBidAsk(self, pair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
        try:       
            pairIndex = -1
            bidAskExpired = False
            BidAsk = BidAskType()
    
            if allBidAskSymbols != None:
                if pair in allBidAskSymbols:
                    pairIndex = allBidAskSymbols.index(pair)
                    if (datetime.datetime.now()-allBidAskUpdateTimes[pairIndex]).total_seconds() > BID_ASK_EXPIRATION_IN_SECONDS:
                        bidAskExpired = True
    
            currentBrokerTime = self.fxclient.getRateTable().getRate(self.API.createFXPair(DukascopySymbol[pair])).getTimestamp()
            
            if pairIndex == -1 or bidAskExpired == True:                        
                prices = [self.fxclient.getRateTable().getRate(self.API.createFXPair(DukascopySymbol[pair])).getBid(), self.fxclient.getRateTable().getRate(self.API.createFXPair(DukascopySymbol[pair])).getAsk()]
            else:
                prices = allBidAskLoaded[pairIndex]
                       
            BidAsk[0] = prices[0]
            BidAsk[1] = prices[1]
            BidAsk[2] = 1
            BidAsk[3] = 1
            BidAsk[4] = 1
            BidAsk[5] = 1
            error = None
            
            if pairIndex == -1 and allBidAskSymbols != None:
                allBidAskSymbols.append(pair)
                allBidAskLoaded.append(prices)
                allBidAskUpdateTimes.append(datetime.datetime.now())
                
            if bidAskExpired == True:
                allBidAskLoaded[pairIndex] = prices
                allBidAskUpdateTimes[pairIndex] = datetime.datetime.now()
            
            return {'BidAsk': BidAsk, 'error': error}
        
        except Exception: 
            e = Exception
            self.logger.critical('Error getting custom BidAsk %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            self.error = e
            return

    def displayTrades(self):
        try:
            print '%-10s%-5s%-21s%-8s%-8s%9s%9s%9s' % ('Trade ID', 'Type', 'Open Time', 'Lots', 'Symbol', 'Price', 'SL', 'TP')
        
            for trade in self.openTrades:
            
                if trade.getUnits() > 0 :
                    tradeType = "BUY"
                else:
                    tradeType = "SELL"
                    
                ticket = 1
                
                print '%-10s%-5s%-21s%-8.5f%-8s%9.5f%9.5f%9.5f' % (
                    ticket,
                    tradeType,
                    0,
                    abs(float(trade.getUnits()/100000)),
                    DukascopySymbolReverse[str(trade.getPair().getPair())],
                    float(trade.getPrice()),
                    0,
                    0)
                    
                ticket += 1
                
        except Exception: 
            e = Exception
            self.logger.critical('Error DisplayTrades %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            return

    def openTrade(self, lots, direction, pair, stopLoss, takeProfit, instanceID):
        if (int(lots * 100000)) > 0:
            if direction == SELL:
                lots = lots * -1
            try:
                marketOrder = self.API.createMarketOrder()
                FXPair = self.API.createFXPair(str(DukascopySymbol[pair]))
                print(int(lots * 100000))
                marketOrder.setUnits(int(lots * 100000))
                marketOrder.setPair(FXPair)
                if stopLoss != 0 :
                    marketOrder.setStopLoss(stopLoss)
                if takeProfit != 0 :
                    marketOrder.setTakeProft(takeProfit)
                self.account.execute(marketOrder)
                
            except Exception: 
                e = Exception
                print sys.exc_info()[1]
                self.logger.critical('Error openTrade %s: %s', type(e), sys.exc_info()[1])
                self.error = e
                return

    def closeTrade(self, ticket, instanceID, orderType):
        self.logger.info("Closing order %s", str(ticket))
        try:
            self.Account.close(self.account.getTradeWithId(int(ticket)))
        except Exception: 
            e = Exception
            print sys.exc_info()[1]
            self.logger.critical('Error Closetrade %s: %s', type(e), sys.exc_info()[1])
            self.error = e
            return

    def modifyTrade(self, ticket, instanceID, stopLoss, takeProfit, orderType):
        try:
            trade = self.account.getTradeWithId(int(ticket))
            if stopLoss != 0 :
                trade.setStopLoss(stopLoss)
            if takeProfit != 0 :
                trade.setTakeProft(takeProfit)
            self.Account.modify(trade)
        except Exception: 
            e = Exception
            print sys.exc_info()[1]
            self.logger.critical('Error modifyTrade %s: %s', type(e), sys.exc_info()[1])
            self.error = e
            return
