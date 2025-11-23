from __future__ import division
from ctypes import *
from time import *
from asirikuy import *
import re, os, ctypes,  csv,  calendar, datetime
from tinydb import TinyDB, Query
from random import randint

class OrderWrapper:
    def __init__(self, account):
        self.account = account

    #open trade function
    def openTrade(self, lots, orderType, pair, instanceID, openTime, openPrice, stopLoss, takeProfit):

        try:

            print "Opening trade..."
            ticket = randint(0,10000000)

            db = TinyDB('./data/' + str(self.account.accountID) + "_openOrders.json")
            db_history = TinyDB('./data/' + str(self.account.accountID) + "_openOrders.json")
            Trade = Query()
            Trade_history = Query()

            while len(db.search(Trade.Ticket == ticket)) > 0 or len(db_history.search(Trade_history.Ticket == ticket)) > 0:
                ticket = randint(0,10000000)

            print "new trade ticket {}".format(ticket)

            if orderType == BUY or orderType == BUYSTOP or orderType == BUYLIMIT:
                newStopLoss   = openPrice - stopLoss
                newTakeProfit = openPrice + takeProfit
            if orderType == SELL or orderType == SELLSTOP or orderType == SELLLIMIT:
                newStopLoss   = openPrice + stopLoss
                newTakeProfit = openPrice - takeProfit

            if takeProfit == 0:
                newTakeProfit = 0

            if stopLoss == 0:
                newStopLoss = 0

            db = TinyDB('./data/' + str(self.account.accountID) + "_openOrders.json")
            newTrade = {'Ticket':ticket, 'InstanceID':instanceID, 'Type':orderType, 'OpenTime': openTime, 'OpenPrice':openPrice, 'StopLoss':newStopLoss, 'TakeProfit': newTakeProfit, 'Volume': lots, 'Symbol':pair, 'Profit':0.0}
            db.insert(newTrade)
            
            print "Virtual position opened successfuly."

        except Exception:
            e = Exception
            print sys.exc_info()[1]
            return
                
    #close trade function
    def closeTrade(self, ticket, instanceID, closeTime, closePrice, orderType):
        try:            
           
            db = TinyDB('./data/' + str(self.account.accountID) + "_openOrders.json")

            Trade = Query()
            if ticket == -1:
                openTrade = db.search((Trade.InstanceID == instanceID) & (Trade.Type == orderType))[0]
            else:
                openTrade = db.search(Trade.Ticket == ticket)[0]

            if orderType == BUY or orderType == BUYLIMIT or orderType == BUYSTOP :
                
               #if openTrade["Type"] != orderType :
               #    self.logger.critical('CloseTrade: %s', openTrade )
               #    return              
               #else:
               print "Closing BUY trades..."   
            if orderType == SELL or orderType == SELLLIMIT or orderType == SELLSTOP:
               # if openTrade["Type"] != orderType :
               #    self.logger.critical('CloseTrade: %s', openTrade )
               #    return                              
               # else:
               print "Closing SELL trades..."

            openPrice = openTrade['OpenPrice']
            openTime = openTrade['OpenTime']
            lots = openTrade['Volume']
            takeProfit = openTrade['TakeProfit']
            stopLoss = openTrade['StopLoss']
            pair = openTrade['Symbol']
            profit = openTrade['Profit']

            if ticket == -1:
                db.remove(Trade.InstanceID == instanceID)
            else:
                db.remove(Trade.Ticket == ticket)

            db_history = TinyDB('./data/' + str(self.account.accountID) + "_orderHistory.json")

            newClosedTrade = {'Ticket':ticket, 'InstanceID':instanceID, 'Type':orderType, 'OpenTime':openTime, 'OpenPrice': openPrice,  'CloseTime':closeTime, 'ClosePrice':closePrice, 'StopLoss':stopLoss, 'TakeProfit':takeProfit, 'Volume':lots, 'Symbol':pair, 'Profit':profit, 'ProfitPL':profit/self.account.accountInfo[IDX_BALANCE]}

            db_history.insert(newClosedTrade)
           
        except Exception:
            e = Exception
            #self.logger.critical('Error in OrderWrapper closeTrade %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            return
            
            
    # modify trade function    
    def modifyTrade(self, ticket, stopLoss, takeProfit, instanceID, orderType, marketPrice):
        try:
            if orderType == BUY or orderType == BUYLIMIT or orderType == BUYSTOP:
                print "Modifying BUY trades..."
            if orderType == SELL or orderType == SELLLIMIT or orderType == SELLSTOP:
                print "Modifying SELL trades..."

            db = TinyDB('./data/' + str(self.account.accountID) + "_openOrders.json")

            Trade = Query()

            if (ticket == -1):
                all_trades_to_modify = db.search((Trade.InstanceID == instanceID) & (Trade.Type == orderType))
            else:
                all_trades_to_modify = db.search(Trade.Ticket == ticket)

            for trade in all_trades_to_modify:
                if trade['Type'] == BUY:
                    new_stopLoss   = marketPrice - stopLoss
                    new_takeProfit = marketPrice + takeProfit
                if trade['Type'] == SELL:
                    new_stopLoss   = marketPrice + stopLoss
                    new_takeProfit = marketPrice - takeProfit
                if trade['Type'] == BUYLIMIT or trade['Type'] == BUYSTOP:
                    new_stopLoss   = trade['OpenPrice'] - stopLoss
                    new_takeProfit = trade['OpenPrice'] + takeProfit
                if trade['Type'] == SELLLIMIT or trade['Type'] == SELLSTOP:
                    new_stopLoss   = trade['OpenPrice'] + stopLoss
                    new_takeProfit = trade['OpenPrice'] - takeProfit
                if takeProfit == 0:
                    new_takeProfit = 0
                if stopLoss == 0:
                    new_stopLoss = 0

                db.update({'StopLoss': new_stopLoss, 'TakeProfit': new_takeProfit}, Trade.Ticket==trade['Ticket'])

        except Exception:
            e = Exception
            #self.logger.critical('Error in OrderWrapper modifyTrade %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            return

    # calculate floating P/L trade function    
    def calculateFloatingProfit(self, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):

        try:

            virtualOpenPL = 0

            db = TinyDB('./data/' + str(self.account.accountID) + "_openOrders.json")

            Trade = Query()

            all_trades = db.all()

            for trade in all_trades:

                symbol = trade['Symbol']
                termName = symbol[-3:]
                BidAskQuery = self.account.getCustomBidAsk(trade['Symbol'], allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)

                if BidAskQuery['error'] != None:
                    return
                        
                BidAsk = BidAskQuery['BidAsk']

                if trade['Type'] == BUY:
                    profit = (trade['Volume']*100000*(BidAsk[IDX_BID]-trade['OpenPrice'])) 
                if trade['Type'] == SELL:
                    profit = (trade['Volume']*100000*(trade['OpenPrice']-BidAsk[IDX_ASK]))

                if termName != "USD":
                    BidAskQuery = self.account.getCustomBidAsk(str("USD"+termName), allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)                     
                    BidAsk = BidAskQuery['BidAsk']
                    if trade['Type'] == SELL:
                        profit *= 1/BidAsk[IDX_ASK]
                    if trade['Type'] == BUY:
                        profit *= 1/BidAsk[IDX_BID]

                db.update({'Profit':profit}, Trade.Ticket == trade['Ticket'])

                virtualOpenPL  += float(profit)
                    
            self.account.virtualBalance = self.account.accountInfo[IDX_EQUITY]-virtualOpenPL
            self.account.virtualEquity = self.account.virtualBalance + virtualOpenPL

        except Exception:
            e = Exception
            #self.logger.critical('Error in OrderWrapper calculating floating Profit %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            return

        
    # balance the net positioning
    def balancePositioning(self, pair):
        
        try:
            idealNetPositioning = 0
            currentRealPositioning = 0
            volumeCorrection = 0

            db = TinyDB('./data/' + str(self.account.accountID) + "_openOrders.json")

            all_trades = db.all()          

            for trade in all_trades:
                if trade['Type'] == BUY and trade['Symbol'] == pair :
                    idealNetPositioning += float(trade['Volume'])
                if trade['Type'] == SELL and trade['Symbol'] == pair  :
                    idealNetPositioning -= float(trade['Volume'])

            for i in range(0,self.account.totalOpenTradesCount):
                try:
                    if self.account.openOrderInfo[i].type == BUY and self.account.openOrderSymbols[i] == pair:
                        currentRealPositioning += self.account.openOrderInfo[i].lots
                    if self.account.openOrderInfo[i].type == SELL and self.account.openOrderSymbols[i] == pair:
                        currentRealPositioning -= self.account.openOrderInfo[i].lots
                except Exception:
                    print "Trade index handling issue. Check if persistent"
                    
            volumeCorrection = idealNetPositioning-currentRealPositioning

            if volumeCorrection > 0:
                ticket = self.account.openTrade(abs(volumeCorrection), BUY, pair, 0, 0, 0)
            if volumeCorrection < 0:
                ticket = self.account.openTrade(abs(volumeCorrection), SELL, pair, 0, 0, 0)

        except Exception:
            e = Exception
            #self.logger.critical('Error in OrderWrapper Balance Positioning %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            return

    # close trades from non running systems
    def closeTradesFromNonRunning(self, all_instanceIDs):
        db = TinyDB('./data/' + str(self.account.accountID) + "_openOrders.json")
        all_trades = db.all()
        deleteQuery = Query()

        for trade in all_trades:
            if trade['InstanceID'] not in all_instanceIDs:
                db.remove(deleteQuery.Ticket == trade['Ticket'])               
    
    # check whether SL or TP have been breached for any orders
    def checkLimitOrders(self, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):

        try:

            db = TinyDB('./data/' + str(self.account.accountID) + "_openOrders.json")
            all_trades = db.all()          

            for trade in all_trades:
            
                ticket   = trade['Ticket']
                stopLoss = trade['StopLoss']
                takeProfit = trade['TakeProfit']
                closeTime = self.account.currentBrokerTime
                lots = trade['Volume']
                openPrice = trade['OpenPrice']
                pair = trade['Symbol']
                orderType = trade['Type']
                instanceID = trade['InstanceID']
                
                BidAskQueryQuote = self.account.getCustomBidAsk(pair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
                BidAskQuote = BidAskQueryQuote['BidAsk']  
                Bid = BidAskQuote[0]
                Ask = BidAskQuote[1]
                
                if orderType == BUYLIMIT and Ask < openPrice:
                    self.account.orderWrapper.closeTrade(ticket, instanceID, closeTime, Bid, BUYLIMIT)
                    self.account.orderWrapper.openTrade(lots, BUY, pair, instanceID, closeTime, openPrice, stopLoss, takeProfit)

                if orderType == BUYSTOP and Ask > openPrice:
                    self.account.orderWrapper.closeTrade(ticket, instanceID, closeTime, Bid, BUYLIMIT)
                    self.account.orderWrapper.openTrade(lots, BUY, pair, instanceID, closeTime, openPrice, stopLoss, takeProfit)

                if orderType == SELLLIMIT and Bid > openPrice:
                    self.account.orderWrapper.closeTrade(ticket, instanceID, closeTime, Bid, BUYLIMIT)
                    self.account.orderWrapper.openTrade(lots, SELL, pair, instanceID, closeTime, openPrice, stopLoss, takeProfit)

                if orderType == SELLSTOP and Bid < openPrice:
                    self.account.orderWrapper.closeTrade(ticket, instanceID, closeTime, Bid, BUYLIMIT)
                    self.account.orderWrapper.openTrade(lots, SELL, pair, instanceID, closeTime, openPrice, stopLoss, takeProfit)
                        
                if stopLoss != 0:
                    if orderType == BUY and Bid < stopLoss:
                        self.account.orderWrapper.closeTrade(ticket, instanceID, closeTime, Bid, BUY)
                    if orderType == SELL and Ask > stopLoss:
                        self.account.orderWrapper.closeTrade(ticket, instanceID, closeTime, Ask, SELL)
                if takeProfit != 0:
                    if orderType == BUY and Bid > takeProfit:
                        self.account.orderWrapper.closeTrade(ticket, instanceID, closeTime, Bid, BUY)
                    if orderType == SELL and Ask < takeProfit:
                        self.account.orderWrapper.closeTrade(ticket, instanceID, closeTime, Ask, SELL)
                        
            for pair in allBidAskSymbols:
                self.account.orderWrapper.balancePositioning(pair)  

        except Exception:
            e = Exception
            #self.logger.critical('Error in OrderWrapper Balance Positioning %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            return
        
