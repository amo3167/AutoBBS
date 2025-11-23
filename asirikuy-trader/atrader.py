#!/usr/bin/env python

from colorama import init, Fore, Back, Style
import logging, logging.handlers, argparse, ctypes, sys, os
from include.asirikuy import *
from include.misc import *
from include.account import *
from include.strategy import *
from include.orderwrapper import *
from include.web_server import *
from time import strftime
from time import sleep
from jpype import *
import platform
import dateutil.parser
import datetime

def main():
    
    interface_width = 150

    if os.path.isfile(os.path.join(os.getcwd(), 'stop')):
        print "Stop file present, will quit in 5 seconds..."
        sleep(5)
        quit()

    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config-file')
    parser.add_argument('-v','--version', nargs='*')
    
    args = parser.parse_args()

    if args.version != None:
        system = platform.system()
        if (system == "Windows"):
            asfdll = loadLibrary('AsirikuyFrameworkAPI')
        elif (system == "Linux"):
            asfdll = loadLibrary('libAsirikuyFrameworkAPI.so')
        elif (system == "Darwin"):
            asfdll = loadLibrary('libAsirikuyFrameworkAPI.dylib')
        else:
            print "No shared library loading support for OS %s" % (system)
            return False

        ASKFrameworkVersion = getASKFrameworkVersion(asfdll)

        print "AsirikuyFrameworkAPI v"+ASKFrameworkVersion
        print "AsirikuyTrader v{}".format(PROGRAM_VERSION)
        quit()

    global DukascopySymbol

    #Paths
    if args.config_file:
        if os.path.exists(args.config_file):
            configFilePath = args.config_file
        else:
            print "Config file doesn't exist"
            quit()
    else:
        configFilePath = './config/atrader.config'

    asirikuyFrameworkPath = './config/AsirikuyConfig.xml'

    #Colorama Initialization
    init(autoreset=True)

    #Read config file
    config = readConfigFile(configFilePath)
    if not config:
        print (Style.BRIGHT + Fore.RED + 'Error reading config file %s' % (configFilePath))
        return False

    logSeverity = config.getint('misc', 'logSeverity') * 10 if config.has_option('misc', 'logSeverity') else 20
    screenLogSeverity = config.getint('misc', 'screenLogSeverity') * 10 if config.has_option('misc', 'screenLogSeverity') else 40
    emailLogSeverity = config.getint('misc', 'emailLogSeverity') * 10 if config.has_option('misc', 'emailLogSeverity') else 40
    executionTime = config.getint('misc', 'executionTime') if config.has_option('misc', 'executionTime') else 30
    traderInstance = 1 # only one account per folder. (config.getint('misc', 'traderInstance'))

    graphicPaintingPeriod = config.getint('plots', 'graphicPaintingPeriod') if config.has_option('plots', 'graphicPaintingPeriod') else 0
    enableWebServer = config.getboolean('plots', 'enableWebServer') if config.has_option('plots', 'enableWebServer') else False
    webServerPort = config.getint('plots', 'webServerPort') if config.has_option('plots', 'webServerPort') else 25000

    # if another instance is running halt execution
    try:
        if os.path.exists(os.path.join(os.getcwd(), 'log', '1_atrader.log')):
            os.unlink(os.path.join(os.getcwd(), 'log', '1_atrader.log'))
    except Exception:
        print "Trader instance already running on this folder. Aborting execution"
        quit()    

    logPath = "./log/" + str(traderInstance) + "_atrader.log"

    #Initialize loggers
    global logger
    logger = logging.getLogger('atrader')
    logger.setLevel(logging.INFO)

    fh = logging.FileHandler(logPath)
    fh.setLevel(logSeverity)
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    fh.setFormatter(formatter)

    ch = logging.StreamHandler()
    ch.setLevel(screenLogSeverity)
    formatter = logging.Formatter('%(asctime)s%(levelname)s - %(message)s', Fore.RED)
    ch.setFormatter(formatter)

    smtp = config.get('misc', 'smtp')
    port = config.getint('misc', 'port')
    email_from = config.get('misc', 'from')
    email_to = config.get('misc', 'to')
    smtpUser = config.get('misc', 'user')
    smtpPassword = config.get('misc', 'password') if config.has_option('misc', 'password') else ''
    eh = ThreadedTlsSMTPHandler(mailhost=(smtp, port), fromaddr=email_from, toaddrs=email_to,
                                    subject='Log from account {}'.format(config.get('account', 'accountName')),
                                    credentials=(smtpUser, smtpPassword), secure=None)
    eh.setLevel(emailLogSeverity)
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    eh.setFormatter(formatter)

    if logSeverity > 0: logger.addHandler(fh)
    if screenLogSeverity > 0: logger.addHandler(ch)
    if emailLogSeverity > 0: logger.addHandler(eh)

    #Load Asirikuy Framework
    global asfdll

    logger.info('Initializing Asirikuy Trader v{}'.format(PROGRAM_VERSION))
    logger.info('current time is = ' + str(datetime.date.today()))

    system = platform.system()
    if (system == "Windows"):

        # use the following code to prevent windows
        #error messages from popping up on exceptions
        import ctypes
        SEM_NOGPFAULTERRORBOX = 0x0002 # From MSDN
        ctypes.windll.kernel32.SetErrorMode(SEM_NOGPFAULTERRORBOX);
        subprocess_flags = 0x8000000 #win32con.CREATE_NO_WINDOW?
        
        asfdll = loadLibrary('AsirikuyFrameworkAPI')
    elif (system == "Linux"):
        asfdll = loadLibrary('libAsirikuyFrameworkAPI.so')
    elif (system == "Darwin"):
        asfdll = loadLibrary('libAsirikuyFrameworkAPI.dylib')
    else:
        print "No shared library loading support for OS %s" % (system)

    version = getASKFrameworkVersion(asfdll)
    logger.info('Initialized Asirikuy Framework %s', version)

    try:
        accountName = config.get('account', 'accountName')
        brokerName = config.get('account', 'brokerName')
        login = config.get('account', 'login')
        password = config.get('account', 'password')
        useOrderWrapper = config.getboolean('account', 'useOrderWrapper')
        hasStaticIP = config.getboolean('account', 'hasStaticIP')
        isDemo = config.getboolean('account', 'isDemo')
        useStreaming = config.getboolean('account', 'useStreaming')
        initialBalance = config.getfloat('account', 'initialBalance')
        depositSymbol = config.get('account', 'depositSymbol')
    except:
        print Fore.WHITE + Style.BRIGHT + Back.RED + str(sys.exc_info()[1])
        quit()

    if brokerName == 'OANDA Corporation':
        isJava = config.getboolean('account', 'isJava')
        if isJava:
            logger.info('Initializing Oanda Java account')
            class OandaStreaming:
                def __init__(self, logger):
                    self.logger = logger
                
                def onTick(self, symbol, bid, ask):
                    try:
                        account.streamingHB = time()
                        #filter Oanda execution so that it does not happen when the Oanda servers
                        #cannot execute trades
                        currentTime =  time()
                        if (datetime.datetime.utcfromtimestamp(currentTime).isoweekday() == 5 and datetime.datetime.utcfromtimestamp(currentTime).hour < 21) or (datetime.datetime.utcfromtimestamp(currentTime).isoweekday() < 5) or (datetime.datetime.utcfromtimestamp(currentTime).isoweekday() == 7 and datetime.datetime.utcfromtimestamp(currentTime).hour > 21):
                            account.runStrategies(allSymbolsLoaded, allRatesLoaded, symbol=symbol)
                        else:
                            print "Account not executing, Oanda week-end time (orders cannot be placed)."
                            for strategy in account.strategies:
                                account.getTrades(strategy.settings[ORDERINFO_ARRAY_SIZE], strategy.instanceID)
                            sleep(5)
                    except Exception as e: 
                        self.logger.critical('Error in general Oanda JAVA Call %s: %s', type(e).__name__, str(e))
                        print e
                    return
                    

            if (system == "Windows"):
                startJVM(jpype.getDefaultJVMPath(),
                               '-Dlog4j.configuration=file:./include/log4j.properties',
                               '-Djava.class.path=./config;' \
                               './src/OandaWrapper/oanda_helper.jar;' \
                               './vendor/oanda_fxtrade_implementation.jar;' \
                               './vendor/oanda_fxtrade.jar;' \
                               './vendor/slf4j-log4j12-1.6.4.jar;' \
                               './vendor/slf4j-api-1.6.4.jar;' \
                               './vendor/log4j-1.2.16.jar;' \
                )
            else:
                startJVM(jpype.getDefaultJVMPath(),
                               '-Djava.awt.headless=true',
                               '-Dlog4j.configuration=file:./include/log4j.properties',
                               '-Djava.class.path=./config:' \
                               './src/OandaWrapper/oanda_helper.jar:' \
                               './vendor/oanda_fxtrade_implementation.jar:' \
                               './vendor/oanda_fxtrade.jar:' \
                               './vendor/slf4j-log4j12-1.6.4.jar:' \
                               './vendor/slf4j-api-1.6.4.jar:' \
                               './vendor/log4j-1.2.16.jar:'
                )
            accountID = config.get('account', 'accountID')
            account = OandaJavaAccount(depositSymbol,initialBalance, accountName, brokerName, login, password, useOrderWrapper, hasStaticIP, isDemo, accountID, logger)
            account.streamingHB = time() + 20 #We give 20 seconds to start getting ticks

            if useStreaming:
                subscribeSymbols = ['EUR/USD', 'EUR/CHF', 'GBP/USD', 'USD/JPY']
                for symbol in subscribeSymbols:
                    oanda_streaming = OandaStreaming(logger)
                    package = jpype.JPackage('asirikuy')
                    oanda_wrapper = package.OandaWrapper
                    proxy = jpype.JProxy(oanda_wrapper, inst=oanda_streaming)
                    ticker = package.Ticker(proxy, symbol)
                    account.fxclient.getRateTable().getEventManager().add(ticker)
        else:
            logger.info('Initializing Oanda REST account')
            accountID = config.get('account', 'accountID')
            account = OandaAccount(depositSymbol,initialBalance, accountName, brokerName, login, password, useOrderWrapper, hasStaticIP, isDemo, accountID, logger)

    elif brokerName == 'Dukascopy' or brokerName == 'AlpariUS-JForex' or 'FXDD-JForex' in brokerName:
        logger.info('Initializing Dukascopy account')
        if (system == "Windows"):
            startJVM(jpype.getDefaultJVMPath(),
                     '-Djava.class.path=./config;' \
                     './include/asirikuy.jar;' \
                     './vendor/DDS2-jClient-JForex-2.45.37.jar;' \
                     './vendor/DDS2-Charts-6.23.40.jar;' \
                     './vendor/patterns-1.59.21.jar;' \
                     './vendor/greed-common-317.43.jar;' \
                     './vendor/MQL4Connector-2.12.37.jar;' \
                     './vendor/jna-3.5.0.jar;' \
                     './vendor/MQL4Converter-2.51.39.jar;' \
                     './vendor/commons-lang3-3.0.1.jar;' \
                     './vendor/JForex-API-2.12.33.jar;' \
                     './vendor/transport-client-0.4.93.jar;' \
                     './vendor/transport-client-2.5.8.jar;' \
                     './vendor/auth-protocol-client-1.0.2.jar;' \
                     './vendor/dds2-common-2.5.29.jar;' \
                     './vendor/srp6a-1.5.3.jar;'\
                     './vendor/ui-core-1.5.20.jar;'\
                     './vendor/msg-1.0.46.jar;' \
                     './vendor/transport-common-0.5.74.jar;' \
                     './vendor/mina-core-1.1.7dc.11.jar;' \
                     './vendor/mina-filter-ssl-1.1.7dc.11.jar;' \
                     './vendor/mail-1.4.jar;' \
                     './vendor/activation-1.1.jar;' \
                     './vendor/jcalendar-1.3.3.jar;' \
                     './vendor/DDS2-TextEditor-1.21.jar;' \
                     './vendor/jakarta-oro-2.0.8.jar;' \
                     './vendor/lucene-core-3.4.0.jar;' \
                     './vendor/lucene-highlighter-3.4.0.jar;' \
                     './vendor/lucene-memory-3.4.0.jar;' \
                     './vendor/lucene-queries-3.4.0.jar;' \
                     './vendor/jakarta-regexp-1.4.jar;' \
                     './vendor/ecj-3.5.2.jar;' \
                     './vendor/ta-lib-0.4.4dc.jar;' \
                     './vendor/7zip-4.65.jar;' \
                     './vendor/JForex-API-2.12.33-sources.jar;' \
                     './vendor/slf4j-log4j12-1.7.12.jar;' \
                     './vendor/slf4j-api-1.7.12.jar;' \
                     './vendor/log4j-1.2.17.jar;' \
            )
        else:
            startJVM(jpype.getDefaultJVMPath(),
                     '-Djava.awt.headless=true',
                     '-Djava.class.path=./config:' \
                     './include/asirikuy.jar:' \
                     './vendor/DDS2-jClient-JForex-2.45.37.jar:' \
                     './vendor/DDS2-Charts-6.23.40.jar:' \
                     './vendor/patterns-1.59.21.jar:' \
                     './vendor/greed-common-317.43.jar:' \
                     './vendor/MQL4Connector-2.12.37.jar:' \
                     './vendor/jna-3.5.0.jar:' \
                     './vendor/MQL4Converter-2.51.39.jar:' \
                     './vendor/commons-lang3-3.0.1.jar:' \
                     './vendor/JForex-API-2.12.33.jar:' \
                     './vendor/transport-client-0.4.93.jar:' \
                     './vendor/transport-client-2.5.8.jar:' \
                     './vendor/auth-protocol-client-1.0.2.jar:' \
                     './vendor/dds2-common-2.5.29.jar:' \
                     './vendor/srp6a-1.5.3.jar:'\
                     './vendor/ui-core-1.5.20.jar:'\
                     './vendor/msg-1.0.46.jar:' \
                     './vendor/transport-common-0.5.74.jar:' \
                     './vendor/mina-core-1.1.7dc.11.jar:' \
                     './vendor/mina-filter-ssl-1.1.7dc.11.jar:' \
                     './vendor/mail-1.4.jar:' \
                     './vendor/activation-1.1.jar:' \
                     './vendor/jcalendar-1.3.3.jar:' \
                     './vendor/DDS2-TextEditor-1.21.jar:' \
                     './vendor/jakarta-oro-2.0.8.jar:' \
                     './vendor/lucene-core-3.4.0.jar:' \
                     './vendor/lucene-highlighter-3.4.0.jar:' \
                     './vendor/lucene-memory-3.4.0.jar:' \
                     './vendor/lucene-queries-3.4.0.jar:' \
                     './vendor/jakarta-regexp-1.4.jar:' \
                     './vendor/ecj-3.5.2.jar:' \
                     './vendor/ta-lib-0.4.4dc.jar:' \
                     './vendor/7zip-4.65.jar:' \
                     './vendor/JForex-API-2.12.33-sources.jar:' \
                     './vendor/slf4j-log4j12-1.7.12.jar:' \
                     './vendor/slf4j-api-1.7.12.jar:' \
                     './vendor/log4j-1.2.17.jar:' \
            )
        accountID = config.get('account', 'accountID')
        account = JForexAccount(depositSymbol,initialBalance, accountName, brokerName, login, password, useOrderWrapper, hasStaticIP, isDemo, accountID)
    else:
        logger.error('Broker %s not supported' % brokerName)
        quit()
    account.orderWrapper = OrderWrapper(account)
    account.refBrokerName = config.get('account', 'refBrokerName')
    account.logger = logger

    strategySections = map(str.strip, config.get('account', 'strategies').split(','))
    account.strategies = []
    all_instanceIDs = []
    
    for strategySection in strategySections:
        try:

            rateRequirements = []
            symbolRequirements = []
            timeframeRequirements = []
    
            setfilePath = os.path.join('./sets', config.get(strategySection, 'setFile'))
            pair = config.get(strategySection, 'pair')
            strategyID = config.getint(strategySection, 'strategyId')
            strategyName = config.get(strategySection, 'name')
            passedTimeFrame = config.get(strategySection, 'passedTimeFrame')
            
            for s in config.get(strategySection, "rateRequirements").split(','): rateRequirements.append(int(s))
            for s in config.get(strategySection, "symbolRequirements").split(','): symbolRequirements.append(s)
            for s in config.get(strategySection, "timeframeRequirements").split(','): timeframeRequirements.append(int(s))

            settings = MT4Set(setfilePath)
            instanceID = int(settings.mainParams["STRATEGY_INSTANCE_ID"]['value']) if settings.content.has_option('main',  'STRATEGY_INSTANCE_ID') else 0

            if instanceID in all_instanceIDs:
                logger.critical("Duplicate instance ID found {}. Aborting run".format(instanceID))
                sleep(5)
                quit()
                
            all_instanceIDs.append(instanceID)      
            
            strategy = Strategy(setfilePath, pair, strategyID, passedTimeFrame, strategyName, account.accountCurrency, rateRequirements, symbolRequirements, timeframeRequirements, asirikuyFrameworkPath, traderInstance)
            strategy.account = account
            strategy.logger = logger
            #strategy.initInstance(asirikuyFrameworkPath, traderInstance)
        except:
            logger.critical('Error initializing strategy %s: %s', strategySection, sys.exc_info()[1])
            sleep(5)
            quit()

        account.strategies.append(strategy)
        account.logger = logger

    if graphicPaintingPeriod > 0:
        import include.plotter
        graph_plotter = include.plotter.GraphPlotter(account = account, refresh_period = graphicPaintingPeriod)

    if enableWebServer:
        if graphicPaintingPeriod <= 0:
            graphicPaintingPeriod = 1
        web_server = WebServer(webServerPort)
        web_server.daemon = True
        web_server.start()
        
    graphExecutionCounter = 0

    #initialize rate saving arrays
    allSymbolsLoaded     = []
    allRatesLoaded       = []
    allBidAskSymbols     = []
    allBidAskLoaded      = []
    allBidAskUpdateTimes = []

    #close all trades from non-running systems if using order wrapper
    if account.useOrderWrapper:
        account.orderWrapper.closeTradesFromNonRunning(all_instanceIDs)   
    
    while True:
        try :
            startTime = datetime.datetime.now()
            currentTime = datetime.datetime.now(dateutil.tz.gettz('Europe/Madrid'))

            if (currentTime.isoweekday() == 5 and currentTime.hour > 23) or (currentTime.isoweekday() == 7 and currentTime.hour < 22) or currentTime.isoweekday() == 6  :
                print 'Weekend time, will sleep for 60 seconds...'
                sleep(60)
                continue
                         
            if brokerName == 'OANDA Corporation' and isJava:
                if not account.fxclient.isLoggedIn():
                    print Fore.WHITE + Style.BRIGHT + Back.RED + 'No connection, will quit in 5 seconds...'
                    sleep(5)
                    quit()
                else:
                    account.runStrategies(allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)                   
            else:
                account.runStrategies(allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
                
            print (Fore.WHITE + Style.BRIGHT + Back.BLUE + 'Account: {0} -- Running F4 v{1}'.format(account.name, version)).ljust(interface_width)
            for strategy in account.strategies:
                print (
                (Fore.WHITE + Style.BRIGHT + Back.CYAN + "Strategy %s (ID: %d)" % (strategy.name, strategy.instanceID)).ljust(
                    interface_width))
                account.displaySystemUI(strategy.instanceID)
            print ((Fore.WHITE + Style.BRIGHT + Back.BLUE + "Running %d strategies (%d open real positions)" %
                    (len(account.strategies), account.totalOpenTradesCount)
                   ).ljust(interface_width))
            if account.totalOpenTradesCount > 0: account.displayTrades()
            if account.useOrderWrapper:
                print ((Fore.WHITE + Style.BRIGHT + Back.BLUE + "Running %d strategies (%d open virtual trades)" %
                        (len(account.strategies), account.totalVirtualOpenTradesCount)
                       ).ljust(interface_width))
                if account.totalVirtualOpenTradesCount > 0: account.displayVirtualTrades()
                print ((Fore.WHITE + Style.BRIGHT + Back.BLUE + "Historical Analysis").ljust(interface_width))
                if account.totalHistoryTradesCount > 0: account.displayVirtualHistoryAnalysis()
            account.displayBalance()

            systemExecutionTime = (datetime.datetime.now() - startTime).total_seconds()

            print ((Fore.WHITE + Style.BRIGHT + Back.GREEN + 'Last UI refresh %s, sleeping for %s secs.' % (
            strftime("%a, %d %b %Y %X"), executionTime)).ljust(interface_width))
            print ((Fore.WHITE + Style.BRIGHT + Back.GREEN + 'Last execution of all systems took %s secs.' % (
            systemExecutionTime)).ljust(interface_width))

            try:
                if os.path.exists('./tmp/' + str(account.strategies[0].instanceID) + '_heartBeat.hb'): 
                    f = open('./tmp/' + str(account.strategies[0].instanceID) + '_heartBeat.hb')
                    lines = f.readlines()
                    f.close()
                    line = 'Last execution {0}'.format(lines[1].rstrip('\n'))
                    print (((Fore.WHITE + Style.BRIGHT + Back.GREEN + line).ljust(interface_width)))
            except Exception: 
                e = Exception
                print sys.exc_info()[1]
                return

            if useOrderWrapper and graphicPaintingPeriod > 0:
                if graphExecutionCounter == 0:
                    graph_plotter.run(allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
                    graphExecutionCounter = graphExecutionCounter + 1
                else:
                    graphExecutionCounter = graphExecutionCounter + 1 

                if graphExecutionCounter >= graphicPaintingPeriod:
                    graphExecutionCounter = 0
            
            sleep(executionTime)

            if os.path.isfile(os.path.join(os.getcwd(), 'stop')):
                print "Stop file present, will quit in 5 seconds..."
                sleep(5)
                quit()
        except KeyboardInterrupt:
            logger.critical('Keyboard interrupt detected. Exiting graciously.')
            exit()
        except Exception: 
            e = Exception
            print sys.exc_info()[1]
            return

if __name__ == "__main__": main()
