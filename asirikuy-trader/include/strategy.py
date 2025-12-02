from mt import *
from asirikuy import *
import urllib2, json, httplib, math
import platform

class Strategy:
    def __init__(self, setfilePath, pair, strategyID, passedTimeFrame, name, accountCurrency, rateRequirements, symbolRequirements, timeframeRequirements, asirikuyFrameworkPath, traderInstance):
        #Load Asirikuy Framework
        self.isRunning = False
        self.lastRunningTime = datetime.datetime.now()
        system = platform.system()
        if (system == "Windows"):
            self.asfdll = loadLibrary('AsirikuyFrameworkAPI')
        elif (system == "Linux"):
            self.asfdll = loadLibrary('libAsirikuyFrameworkAPI.so')
        elif (system == "Darwin"):
            self.asfdll = loadLibrary('libAsirikuyFrameworkAPI.dylib')
        else:
            print "No shared library loading support for OS %s" % (system)

        if pair == None or strategyID == None or passedTimeFrame == None or setfilePath == None: raise ValueError('Values not valid to initialize Strategy')
        
        self.setFilePath = setfilePath
        self.set = MT4Set(setfilePath)

        self.instanceID = int(self.set.mainParams["STRATEGY_INSTANCE_ID"]['value']) if self.set.content.has_option('main',  'STRATEGY_INSTANCE_ID') else 0
        
        init = self.asfdll.initInstanceC(self.instanceID, 0, asirikuyFrameworkPath, str(traderInstance))
        if init > 0: raise ValueError("initInstanceC returned %d" % (init))

        self.pair = pair
        self.strategyID = int(strategyID)
        self.passedTimeFrame = int(passedTimeFrame)
        self.digits = 5
        self.name = name

        self.rateRequirements = rateRequirements
        self.symbolRequirements = symbolRequirements
        self.timeframeRequirements = timeframeRequirements

        if len(self.rateRequirements) < 10:
            for j in range(len(self.rateRequirements), 10):
                self.rateRequirements.append(0)
                
        if len(self.symbolRequirements) < 10:
            for j in range(len(self.symbolRequirements), 10):
                self.symbolRequirements.append("N")

        if len(self.timeframeRequirements) < 10:
            for j in range(len(self.timeframeRequirements), 10):
                self.timeframeRequirements.append(0)

        bidAskType = c_char * 6
        basePair =  bidAskType()
        quotePair = bidAskType()

        self.asfdll.mql5_getConversionSymbols.argtypes = [
                                            ctypes.POINTER(ctypes.c_char),
                                            ctypes.POINTER(ctypes.c_char),
                                            ctypes.POINTER(bidAskType),
                                            ctypes.POINTER(bidAskType)]
        
	# we treat everything as if we were trading dollar deposits, conversion is done on
	# the balance and equity on the account.py instead.
        self.asfdll.mql5_getConversionSymbols(self.pair,"USD", byref(basePair), byref(quotePair));

        self.basePair = ''.join(basePair)
        self.quotePair = ''.join(quotePair)

        if self.basePair == "\x00\x00\x00\x00\x00\x00":
            self.basePair = ""

        if self.quotePair == "\x00\x00\x00\x00\x00\x00":
            self.quotePair = ""
                 
                
        self.settings = SettingsType()
        self.settings[IS_BACKTESTING]       = False
        self.settings[DISABLE_COMPOUNDING]  = float(self.set.mainParams["DISABLE_COMPOUNDING"]['value']) if self.set.content.has_option('main',  'DISABLE_COMPOUNDING') else 0
        self.settings[TIMED_EXIT_BARS]      = float(self.set.mainParams["TIMED_EXIT_BARS"]['value']) if self.set.content.has_option('main',  'TIMED_EXIT_BARS') else 0
        self.settings[ORIGINAL_EQUITY]      = 0
        self.settings[OPERATIONAL_MODE]     = 1
        self.settings[STRATEGY_INSTANCE_ID] = float(self.instanceID)
        self.settings[INTERNAL_STRATEGY_ID] = float(self.strategyID)
        self.settings[RUN_EVERY_TICK]       = float(self.set.mainParams["RUN_EVERY_TICK"]['value']) if self.set.content.has_option('main',  'RUN_EVERY_TICK') else 0
        self.settings[INSTANCE_MANAGEMENT]  = float(self.set.mainParams["INSTANCE_MANAGEMENT"]['value']) if self.set.content.has_option('main',  'INSTANCE_MANAGEMENT') else 0
        self.settings[MAX_OPEN_ORDERS]      = float(self.set.mainParams["MAX_OPEN_ORDERS"]['value']) if self.set.content.has_option('main',  'MAX_OPEN_ORDERS') else 1
        self.settings[TIMEFRAME]            = float(self.set.mainParams["STRATEGY_TIMEFRAME"]['value']) if self.set.content.has_option('main',  'STRATEGY_TIMEFRAME') else 0
        self.settings[SAVE_TICK_DATA]  = float(self.set.mainParams["SAVE_TICK_DATA"]['value']) if self.set.content.has_option('main',  'SAVE_TICK_DATA') else 0
        self.settings[ANALYSIS_WINDOW_SIZE] = float(self.set.mainParams["ANALYSIS_WINDOW_SIZE"]['value']) if self.set.content.has_option('main',  'ANALYSIS_WINDOW_SIZE') else 0
        self.settings[PARAMETER_SET_POOL]   = float(self.set.mainParams["PARAMETER_SET_POOL"]['value']) if self.set.content.has_option('main',  'PARAMETER_SET_POOL') else 0
        self.settings[ACCOUNT_RISK_PERCENT] = float(self.set.mainParams["ACCOUNT_RISK_PERCENT"]['value']) if self.set.content.has_option('main',  'ACCOUNT_RISK_PERCENT') else 0
        self.settings[MAX_DRAWDOWN_PERCENT] = float(self.set.mainParams["MAX_DRAWDOWN_PERCENT"]['value']) if self.set.content.has_option('main',  'MAX_DRAWDOWN_PERCENT') else 0
        self.settings[MAX_SPREAD]           = float(self.set.mainParams["MAX_SPREAD_PIPS"]['value']) if self.set.content.has_option('main',  'MAX_SPREAD_PIPS') else 0
        self.settings[SL_ATR_MULTIPLIER]    = float(self.set.mainParams["SL_ATR_MULTIPLIER"]['value']) if self.set.content.has_option('main',  'SL_ATR_MULTIPLIER') else 0
        self.settings[TP_ATR_MULTIPLIER]    = float(self.set.mainParams["TP_ATR_MULTIPLIER"]['value']) if self.set.content.has_option('main',  'TP_ATR_MULTIPLIER') else 0
        self.settings[ATR_AVERAGING_PERIOD] = float(self.set.mainParams["ATR_AVERAGING_PERIOD"]['value']) if self.set.content.has_option('main',  'ATR_AVERAGING_PERIOD') else 0
        self.settings[USE_SL]    = 1
        self.settings[USE_TP]    = 1

        
        #self.settings[ADDITIONAL_PARAM_8] = float(self.set.additionalParams["DSL_EXIT_TYPE"]['value']) if self.set.content.has_option('additional', 'DSL_EXIT_TYPE') else 0


        if self.settings[TP_ATR_MULTIPLIER] == 0:
            self.settings[USE_TP] = 0

        if self.settings[SL_ATR_MULTIPLIER] == 0:
            self.settings[USE_SL] = 0

        if self.settings[MAX_OPEN_ORDERS] > 0:
            self.settings[USE_TP] = 0
            self.settings[USE_SL] = 0
        
        self.settings[ORDERINFO_ARRAY_SIZE] = 20
        
        additionalParams = self.set.content.items("additional");
        params = []
        global paramNames
        i=0
        for index, param in enumerate(additionalParams):
            if param[0].find(",") == -1:
                params.append(param[1])
                paramNames[i] = param[0].upper()
                i=i+1
    
        for index, value in enumerate (params):
            #print paramNames[index]
            #print paramIndexes[paramNames[index]]
            #print value
            self.settings[paramIndexes[paramNames[index]]] = float(value)
    
#    def initInstance(self, asirikuyFrameworkPath, traderInstance):
#        init = self.asfdll.initInstanceC(self.instanceID, 0, asirikuyFrameworkPath, str(traderInstance))
#        if init > 0: raise ValueError("initInstanceC returned %d" % (init))
    
    
    def run(self, allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
        try:
            self.account.error = None
            self.account.getAccountInfo(allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
            self.lastRunningTime = datetime.datetime.now()

            if 'JPY' in self.pair:
                self.account.accountInfo[IDX_MINIMUM_STOP] = 0.05
            else:
                self.account.accountInfo[IDX_MINIMUM_STOP] = 0.0005

            ratesInfo = RatesInfoType()
            maxNumCandles = 0

            for i in range(0,9):
                if self.symbolRequirements[i] != "N":

                    if self.timeframeRequirements[i] == 0:
                        ratesInfo[i].requiredTimeFrame = int(self.settings[TIMEFRAME])
                    else:
                        ratesInfo[i].requiredTimeFrame = int(self.timeframeRequirements[i])
                    
                    numCandles = int(self.rateRequirements[i] * 1.2 * ratesInfo[i].requiredTimeFrame/self.passedTimeFrame + 1)
                    
                    if numCandles > maxNumCandles:
                        maxNumCandles = numCandles

            for i in range(0,9):
                if self.symbolRequirements[i] != "N":
                    
                    ratesInfo[i].ratesArraySize = int(maxNumCandles)

                    if self.symbolRequirements[i] != "D":
                        rates = self.account.getCandles(maxNumCandles, self.symbolRequirements[i], self.passedTimeFrame, allSymbolsLoaded, allRatesLoaded)
                    else:
                        rates = self.account.getCandles(maxNumCandles, self.pair, self.passedTimeFrame, allSymbolsLoaded, allRatesLoaded)
                         
                    ratesInfo[i].isEnabled = True
                    
                    ratesInfo[i].actualTimeFrame = self.passedTimeFrame
                      
                    ratesInfo[i].totalBarsRequired = self.rateRequirements[i]   
                    ratesInfo[i].digits  = self.digits
                    ratesInfo[i].point  = 1 / (math.pow(10,ratesInfo[0].digits))

                    
            self.account.getTrades(self.settings[ORDERINFO_ARRAY_SIZE], self.instanceID)
            self.account.getBidAsk(self.pair, self.basePair, self.quotePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)

            StrategyResultsArrayType = int(self.settings[MAX_OPEN_ORDERS])  * resultsType
            strategyResults = StrategyResultsArrayType()

            for j in range(0, int(self.settings[MAX_OPEN_ORDERS])):
                strategyResults[j] = resultsType()
                for i in range(0, 9):
                    strategyResults[j][i] = 0
            
            RatesType = Rate * maxNumCandles
            passedOrderInfoType = OrderInfo * int(self.settings[ORDERINFO_ARRAY_SIZE])
            passedOpenOrderInfo = passedOrderInfoType()
            
            if self.account.useOrderWrapper:
                passedOpenOrderInfo = self.account.virtualOpenOrderInfo
                passedOpenTradesCount = self.account.virtualOpenTradesCount
                self.account.orderWrapper.calculateFloatingProfit(allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
            else:
                passedOpenOrderInfo = self.account.openOrderInfo
                passedOpenTradesCount = self.account.openTradesCount

            #return error if present and avoid running F4
            if self.account.error != None:
                print self.account.error
                sleep(10)
                self.isRunning = False
                return 

            try:

                self.asfdll.c_runStrategy.argtypes = [
                                            ctypes.POINTER(SettingsType),
                                            ctypes.POINTER(ctypes.c_char),
                                            ctypes.POINTER(ctypes.c_char),
                                            ctypes.POINTER(ctypes.c_char),
                                            ctypes.POINTER(ctypes.c_char),
                                            ctypes.POINTER(ctypes.c_int),
                                            ctypes.POINTER(ctypes.c_int),
                                            ctypes.POINTER(passedOrderInfoType),
                                            ctypes.POINTER(accountInfoType),
                                            ctypes.POINTER(BidAskType),
                                            ctypes.POINTER(RatesInfoType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(RatesType),
                                            ctypes.POINTER(StrategyResultsArrayType),                        
                                            ]
            
                f4output = self.asfdll.c_runStrategy (
                                self.settings,
                                self.pair,
                                "USD",
                                self.account.brokerName,
                                self.account.refBrokerName,
                                c_int(int(self.account.currentBrokerTime)),
                                c_int(int(passedOpenTradesCount)),
                                ctypes.pointer(passedOpenOrderInfo),
                                ctypes.pointer(self.account.accountInfo),
                                self.account.BidAsk,
                                ctypes.pointer(ratesInfo),
                                ctypes.pointer(rates),
                                ctypes.pointer(rates),
                                ctypes.pointer(rates),
                                ctypes.pointer(rates),
                                ctypes.pointer(rates),
                                ctypes.pointer(rates),
                                ctypes.pointer(rates),
                                ctypes.pointer(rates),
                                ctypes.pointer(rates),
                                ctypes.pointer(rates),
                                byref(strategyResults)
                                )
                if f4output != 0:
                    if f4output == 3010:
                        self.logger.warning('Strategy %s reached WCS', self.instanceID)
                    else:
                        self.logger.warning('Error %s running strategy %s', f4output, self.instanceID)
            except Exception:
                e = Exception
                self.logger.critical('Error running F4 %s: %s', type(e), sys.exc_info()[1])
                print sys.exc_info()[1]
                self.isRunning = False
                return
                
            for j in range(0, int(self.settings[MAX_OPEN_ORDERS])):

                if strategyResults[j][IDX_TRADING_SIGNALS] > 0:
                    operation = int(strategyResults[j][IDX_TRADING_SIGNALS])
                    

                    if ((operation & SIGNAL_CLOSE_BUY) != 0) or ((operation & SIGNAL_CLOSE_BUYSTOP) != 0) or ((operation & SIGNAL_CLOSE_BUYLIMIT) != 0):

                        if ((operation & SIGNAL_CLOSE_BUY) != 0):
                            signalOrderType = BUY

                        if ((operation & SIGNAL_CLOSE_BUYSTOP) != 0):
                            signalOrderType = BUYSTOP

                        if ((operation & SIGNAL_CLOSE_BUYLIMIT) != 0):
                            signalOrderType = BUYLIMIT
                        
                        self.account.getTrades(self.settings[ORDERINFO_ARRAY_SIZE], self.instanceID)
                        if self.account.useOrderWrapper:
                            passedOpenTradesCount = self.account.virtualOpenTradesCount
                        else:
                            passedOpenTradesCount = self.account.openTradesCount
                        if (passedOpenTradesCount > 0):
                            self.logger.debug("New close BUY trigger")
                            if self.account.useOrderWrapper: self.account.orderWrapper.closeTrade(strategyResults[j][IDX_TICKET_NUMBER], self.instanceID, self.account.currentBrokerTime, self.account.BidAsk[IDX_BID], signalOrderType)
                            else: self.account.closeTrade(strategyResults[j][IDX_TICKET_NUMBER], self.instanceID, signalOrderType)

                    if((operation & SIGNAL_CLOSE_SELL) != 0) or ((operation & SIGNAL_CLOSE_SELLLIMIT) != 0) or ((operation & SIGNAL_CLOSE_SELLSTOP) != 0):

                        if ((operation & SIGNAL_CLOSE_SELL) != 0):
                            signalOrderType = SELL

                        if ((operation & SIGNAL_CLOSE_BUYSTOP) != 0):
                            signalOrderType = SELLSTOP

                        if ((operation & SIGNAL_CLOSE_SELLLIMIT) != 0):
                            signalOrderType = SELLLIMIT
                            
                        self.account.getTrades(self.settings[ORDERINFO_ARRAY_SIZE], self.instanceID)
                        if self.account.useOrderWrapper:
                            passedOpenTradesCount = self.account.virtualOpenTradesCount
                        else:
                            passedOpenTradesCount = self.account.openTradesCount
                        if (passedOpenTradesCount > 0):
                            self.logger.debug("New close SELL trigger")
                            if self.account.useOrderWrapper: self.account.orderWrapper.closeTrade(strategyResults[j][IDX_TICKET_NUMBER], self.instanceID, self.account.currentBrokerTime, self.account.BidAsk[IDX_ASK], signalOrderType)
                            else: self.account.closeTrade(strategyResults[j][IDX_TICKET_NUMBER], self.instanceID, signalOrderType)
                    
                    if((operation & SIGNAL_OPEN_BUY) != 0) or ((operation & SIGNAL_OPEN_BUYSTOP) != 0) or ((operation & SIGNAL_OPEN_BUYLIMIT) != 0):


                        entryPrice = strategyResults[j][IDX_ENTRY_PRICE]
                        
                        if ((operation & SIGNAL_OPEN_BUY) != 0):
                            entryPrice = self.account.BidAsk[IDX_ASK]
                            signalOrderType = BUY

                        if ((operation & SIGNAL_OPEN_BUYSTOP) != 0):
                            signalOrderType = BUYSTOP

                        if ((operation & SIGNAL_OPEN_BUYLIMIT) != 0):
                            signalOrderType = BUYLIMIT              
                            
                        self.logger.debug("New BUY trigger")
                        self.logger.debug(self.account.login)
                        self.account.getTrades(self.settings[ORDERINFO_ARRAY_SIZE], self.instanceID)
                        if self.account.useOrderWrapper: ticket = self.account.orderWrapper.openTrade(strategyResults[j][IDX_LOTS], signalOrderType, self.pair, self.instanceID, self.account.currentBrokerTime, entryPrice, strategyResults[j][IDX_BROKER_SL], strategyResults[j][IDX_BROKER_TP])
                        else: self.account.openTrade(strategyResults[j][IDX_LOTS], signalOrderType, self.pair, strategyResults[j][IDX_BROKER_SL], strategyResults[j][IDX_BROKER_TP], self.instanceID)

                    if((operation & SIGNAL_OPEN_SELL) != 0) or ((operation & SIGNAL_OPEN_SELLSTOP) != 0) or ((operation & SIGNAL_OPEN_SELLLIMIT) != 0):


                        entryPrice = strategyResults[j][IDX_ENTRY_PRICE]
                        
                        if ((operation & SIGNAL_OPEN_SELL) != 0):
                            entryPrice = self.account.BidAsk[IDX_BID]
                            signalOrderType = SELL

                        if ((operation & SIGNAL_OPEN_SELLSTOP) != 0):
                            signalOrderType = SELLSTOP

                        if ((operation & SIGNAL_OPEN_SELLLIMIT) != 0):
                            signalOrderType = SELLLIMIT
                            
                        self.logger.debug("New SELL trigger")
                        self.account.getTrades(self.settings[ORDERINFO_ARRAY_SIZE], self.instanceID)
                        if self.account.useOrderWrapper: ticket = self.account.orderWrapper.openTrade(strategyResults[j][IDX_LOTS], signalOrderType, self.pair, self.instanceID, self.account.currentBrokerTime, entryPrice, strategyResults[j][IDX_BROKER_SL], strategyResults[j][IDX_BROKER_TP])
                        else: self.account.openTrade(strategyResults[j][IDX_LOTS], signalOrderType, self.pair, strategyResults[j][IDX_BROKER_SL], strategyResults[j][IDX_BROKER_TP], self.instanceID)

                    if((operation & SIGNAL_UPDATE_BUY) != 0) or ((operation & SIGNAL_UPDATE_BUYSTOP) != 0) or ((operation & SIGNAL_UPDATE_BUYLIMIT) != 0):

                        if ((operation & SIGNAL_UPDATE_BUY) != 0):
                            signalOrderType = BUY

                        if ((operation & SIGNAL_UPDATE_BUYSTOP) != 0):
                            signalOrderType = BUYSTOP

                        if ((operation & SIGNAL_UPDATE_BUYLIMIT) != 0):
                            signalOrderType = BUYLIMIT
                            
                        self.account.getTrades(self.settings[ORDERINFO_ARRAY_SIZE], self.instanceID)
                        if self.account.useOrderWrapper:
                            passedOpenTradesCount = self.account.virtualOpenTradesCount
                        else:
                            passedOpenTradesCount = self.account.openTradesCount
                        if (passedOpenTradesCount > 0):
                            self.logger.debug("New UPDATE BUY trigger")
                            if self.account.useOrderWrapper: self.account.orderWrapper.modifyTrade(strategyResults[j][IDX_TICKET_NUMBER], strategyResults[j][IDX_BROKER_SL], strategyResults[j][IDX_BROKER_TP], self.instanceID, signalOrderType, self.account.BidAsk[IDX_ASK])
                            else: self.account.modifyTrade(strategyResults[j][IDX_TICKET_NUMBER], self.instanceID, strategyResults[j][IDX_BROKER_SL], strategyResults[j][IDX_BROKER_TP], signalOrderType)

                    if((operation & SIGNAL_UPDATE_SELL) != 0) or ((operation & SIGNAL_UPDATE_SELLSTOP) != 0) or ((operation & SIGNAL_UPDATE_SELLLIMIT) != 0):

                        if ((operation & SIGNAL_UPDATE_SELL) != 0):
                            signalOrderType = SELL

                        if ((operation & SIGNAL_UPDATE_SELLSTOP) != 0):
                            signalOrderType = SELLSTOP

                        if ((operation & SIGNAL_UPDATE_SELLLIMIT) != 0):
                            signalOrderType = SELLLIMIT
                            
                        self.account.getTrades(self.settings[ORDERINFO_ARRAY_SIZE], self.instanceID)
                        if self.account.useOrderWrapper:
                            passedOpenTradesCount = self.account.virtualOpenTradesCount
                        else:
                            passedOpenTradesCount = self.account.openTradesCount
                        if (passedOpenTradesCount > 0):
                            self.logger.debug("New UPDATE SELL trigger")
                            if self.account.useOrderWrapper: self.account.orderWrapper.modifyTrade(strategyResults[j][IDX_TICKET_NUMBER], strategyResults[j][IDX_BROKER_SL], strategyResults[j][IDX_BROKER_TP], self.instanceID, signalOrderType, self.account.BidAsk[IDX_BID])
                            else: self.account.modifyTrade(strategyResults[j][IDX_TICKET_NUMBER], self.instanceID, strategyResults[j][IDX_BROKER_SL], strategyResults[j][IDX_BROKER_TP], signalOrderType)

            if self.account.useOrderWrapper:                             
                self.account.orderWrapper.checkLimitOrders(allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)                                     
                self.account.orderWrapper.calculateFloatingProfit(allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)

            self.isRunning = False

        except Exception:
                e = Exception
                self.logger.critical('Error getting Trades %s: %s', type(e), sys.exc_info()[1])
                self.isRunning = False
                return
            
                
                
                
                
