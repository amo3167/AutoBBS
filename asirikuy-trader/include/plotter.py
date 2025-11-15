import threading, time, datetime, sys, glob, shutil, os
import matplotlib as mpl

mpl.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import matplotlib.dates as mdates
import matplotlib.finance as finance

from matplotlib.dates import DateFormatter, WeekdayLocator, HourLocator, \
    DayLocator, MONDAY

from mako.lookup import TemplateLookup
from mako import exceptions

def testtest():
    plt.plot([1, 2, 3, 4], [1, 4, 9, 16], 'ro')
    plt.show()


class GraphPlotter(threading.Thread):
    def __init__(self, account=None, refresh_period=0, num_candles=100, timeframe=60):
        try:
            # threading.Thread.__init__(self)
            self.account = account
            self.period = refresh_period
            self.num_candles = num_candles
            self.timeframe = timeframe
        except Exception:
            e = Exception
            self.logger.critical('Error GraphPlotter %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            self.error = e
            return

    def format_candles(self, candles=None):
        try:
            result = []
            for index in range(0, self.num_candles):
                formatted_time = mdates.date2num(datetime.datetime.fromtimestamp(int(candles[index].time)))
                result.append((
                formatted_time, candles[index].open, candles[index].high, candles[index].low, candles[index].close,
                candles[index].volume))
        except Exception:
            e = Exception
            self.logger.critical('Error format_candles %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            self.error = e
            return

        return result


    def run(self,allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
        try:
            symbols = []
            images = []
            bgcolor = '#ffffff'
            rcolor = 'r'
            gcolor = 'g'

            for strategy in self.account.strategies:
                symbols.append(strategy.pair)
            symbols = list(set(symbols))

            # while True:
            trades = self.account.getVirtualTrades()
            for symbol in symbols:
                candlesFromData = self.account.getCandles(self.num_candles, symbol, self.timeframe, allSymbolsLoaded, allRatesLoaded, True)

                bidask = self.account.getCustomBidAsk(symbol, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
                candles = []

                candles = self.format_candles(candlesFromData)

                fig = plt.figure(figsize=(20.48, 16), dpi=100, facecolor=bgcolor)
                fig.suptitle('{} - {}'.format(self.account.accountID, symbol), fontsize=28)
                ax1 = plt.subplot2grid((5, 4), (0, 0), rowspan=4, colspan=4, axisbg=bgcolor)
                ax1.grid(True, color='gray')
                ax1.tick_params(axis='y')
                ax1.tick_params(axis='x')

                initial_time = mdates.date2num(datetime.datetime.fromtimestamp(int(candlesFromData[0].time)))
                final_time = mdates.date2num(
                    datetime.datetime.fromtimestamp(int(candlesFromData[self.num_candles - 1].time)))
                ax1.set_xlim(initial_time, final_time + 1.2)
                #ax1.set_ylabel('Price', fontsize=24)
                #ax1.set_xlabel('Date', fontsize=24)


                majorHour = HourLocator(0)
                allhours = HourLocator()
                hourFormatter = DateFormatter('%d/%m/%y %H:%M')

                plt.subplots_adjust()
                plt.rc("font", size=18)
                plt.xlabel("Date", labelpad=20)
                plt.ylabel("Price", labelpad=20)

                ax1.xaxis.set_major_locator(majorHour)
                ax1.xaxis.set_minor_locator(allhours)
                ax1.xaxis.set_major_formatter(hourFormatter)

                #print candles

                #try:
                #    finance.candlestick(ax1, candles, width=0.02,colorup=gcolor, colordown=rcolor)                    
                #except:
                finance.candlestick_ohlc(ax1, candles, width=0.02, colorup=gcolor, colordown=rcolor)
                    
                ax1.xaxis_date()

                plt.setp(plt.gca().get_xticklabels(), rotation=45, horizontalalignment='right')

                limits = ax1.get_xlim()

                for trade in trades:
                    if trade['symbol'] == symbol:
                        plt.axhline(y=trade['price'], color='b', linewidth=1, linestyle='dashed')
                        plt.text(final_time + 0.2, trade['price'], '{:.5f}{}'.format(trade['price'], ' - ' + trade[
                            'strategy_name'] + ' - ' + str(int(trade['id'])) + ' - ' + trade['direction']),
                                 bbox=dict(facecolor='#496D89'), fontsize=18)
                        if trade['tp'] != 0:
                            plt.axhline(y=trade['tp'], color=gcolor, linewidth=1, linestyle='dashed')
                            plt.text(final_time + 0.2, trade['tp'], '{:.5f}{}'.format(trade['tp'], ' - ' + trade[
                                'strategy_name'] + ' - ' + str(int(trade['id']))),
                                     bbox=dict(facecolor='#378B2E'), fontsize=18)
                        if trade['sl'] != 0:
                            plt.axhline(y=trade['sl'], color=rcolor, linewidth=1, linestyle='dashed')
                            plt.text(final_time + 0.2, trade['sl'], '{:.5f}{}'.format(trade['sl'], ' - ' + trade[
                                'strategy_name'] + ' - ' + str(int(trade['id']))),
                                     bbox=dict(facecolor='#BB3C39'), fontsize=18)

                plt.axhline(y=bidask['BidAsk'][0], color='gray', linewidth=1, linestyle='dashed')
                plt.text(final_time + 0.2, bidask['BidAsk'][0], '{}'.format(bidask['BidAsk'][0]),
                         bbox=dict(facecolor='gray'), fontsize=18)

                fig.subplots_adjust(left=0.1, bottom=0, top=0.9, right=0.87)
                plt.savefig('./plots/{}_{}.png'.format(self.account.accountID, symbol), facecolor=fig.get_facecolor())
                plt.close(fig)
                images.append('{}_{}.png'.format(self.account.accountID, symbol))

        except Exception:
            e = Exception
            self.account.logger.critical('Error GraphPlotter %s: %s', type(e), sys.exc_info()[1])
            print sys.exc_info()[1]
            self.error = e
            return

        for file in glob.glob(r'./tmp/*_WorstCaseTrack.png'):
            shutil.copy(file, './plots/')
        self.write_html(self.account, images, trades)

    def write_html(self, account, images, trades):
        templatePath = os.path.join(os.getcwd(), 'plots', 'templates')
        mylookup = TemplateLookup(directories=[templatePath])
        templateTrades = mylookup.get_template('trades.html')
        templateHistory = mylookup.get_template('history.html')
        file_trades = open('plots/{}.html'.format(account.accountID), "w")
        file_history = open('plots/{}_history.html'.format(account.accountID), "w")
        historyTrades = account.getVirtualHistoryTrades()

        #print historyTrades
        
        historyAnalysis = account.getVirtualHistoryAnalysis()
        try:
            file_trades.write(templateTrades.render(images=images, account=account, trades=trades))
            file_history.write(templateHistory.render(account=account, historyAnalysis = historyAnalysis, historyTrades = historyTrades))
        except Exception:
            e = Exception
            print exceptions.text_error_template().render()
            self.error = e
            return
        file_trades.close()
        file_history.close()

