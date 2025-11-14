#!/usr/bin/env python#
import ast
from include.auto_installer import *

installIfNeeded('requests')
installIfNeeded('colorama')
installIfNeeded('numpy')
installIfNeeded('matplotlib')
installIfNeeded('fastcsv')

from include.mt import *
from time import *
from include.misc import *
import decimal
import datetime, ctypes, os, platform, ConfigParser, math, csv, calendar, argparse,  sys,  signal, shutil
import xml.etree.ElementTree as ET
from include.asirikuy import *

def main():
    main_start_time = datetime.datetime.now()
    global iterationNumber, lines, numSystemsInPortfolio, numPairs, setFilePaths
    global fromDate, toDate
    global astdll
    global xmlRoot, xmlTrades, no, write_xml
    global signalCounter

    signalCounter = 0
    lines = ''

    signal.signal(signal.SIGINT, stopOptimization)

    #Check if the script was launched under MPI
    global execUnderMPI
    execUnderMPI = False
    if os.getenv('PMI_RANK') or os.getenv('OMPI_COMM_WORLD_RANK'):
        execUnderMPI = True
        from mpi4py import MPI

    if execUnderMPI:
        global rank
        global comm
        global size
        comm = MPI.COMM_WORLD
        size = comm.Get_size()
        rank = comm.Get_rank()
        #name = MPI.Get_processor_name()
        #print "Hello, World! I am process %d of %d on %s" % (rank, size, name)

    iterationNumber = 0

    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config-file')
    parser.add_argument('-oo', '--output-optimization-file')
    parser.add_argument('-ot', '--output-test-file')
    parser.add_argument('-v','--version', nargs='*')

    args = parser.parse_args()

    if args.version != None:
        version()
        quit()

    system = platform.system()
    if (system == "Windows"):
        astdll = loadLibrary('CTesterFrameworkAPI')
    elif (system == "Linux"):
        astdll = loadLibrary('libCTesterFrameworkAPI.so')
    elif (system == "Darwin"):
        astdll = loadLibrary('libCTesterFrameworkAPI.dylib')
    else:
        print "No shared library loading support for OS %s" % (system)
        return False

    from include.graphics import plotTestResult, plotMultipleTestResults, plotOptimizationResult, plotPortfolioTestResult

    #Paths
    historyPath = "./history/"
    asirikuyCtesterLogPath = './log/AsirikuyCTester.log'

    if execUnderMPI and rank > 0:
        asirikuyCtesterLogPath = './log/AsirikuyCTester_%d.log' % (rank)

    global configFilePath

    if args.config_file == None or not os.path.isfile(args.config_file):
        configFilePath = os.path.join(os.getcwd(), 'config', 'ast.config')
    else:
        configFilePath = args.config_file
    if args.output_optimization_file == None:
        outputOptimizationFile = 'optimization'
    else:
        outputOptimizationFile = args.output_optimization_file
    if args.output_test_file == None:
        outputFile = 'results'
    else:
        outputFile = args.output_test_file
    global outputXMLPath
    outputXMLPath = outputFile + '.xml'

    #Read config file
    global config
    config = readConfigFile(configFilePath)
    if not config:
        print "Error reading config file %s" % (configFilePath)
        return False

    #Get portfolio data
    setFilePaths = []
    symbolsArrayType = c_char_p * len(config.get("strategy", "pair").split(','))
    symbols = symbolsArrayType()
    strategyIDs = []
    spreads = []
    minimumStops = []
    digits = []
    rateRequirements = []
    symbolRequirements = []
    timeframeRequirements = []
    passedTimeFrame = []

    for s in config.get("strategy", "set").split(','):
        if os.path.exists(s):
            setFilePaths.append(s)
        else:
            setFilePaths.append("./sets/%s" % (s))
    for index,s in enumerate(config.get("strategy", "pair").split(',')):
        symbols[index] = s

    for s in config.get("strategy", "strategyID").split(','): strategyIDs.append(int(s))
    for s in config.get("account", "spread").split(','): spreads.append(float(s))
    for s in config.get("account", "minimumStop").split(','): minimumStops.append(float(s))
    for s in config.get("account", "digits").split(','): digits.append(int(s))
    for s in config.get("strategy", "passedtimeframe").split(','): passedTimeFrame.append(int(s))
    for s in config.get("strategy", "rateRequirements").split('|'): rateRequirements.append(s)
    for s in config.get("strategy", "symbolRequirements").split('|'): symbolRequirements.append(s)
    for s in config.get("strategy", "timeframeRequirements").split('|'): timeframeRequirements.append(s)    

    numSystemsInPortfolio = len(setFilePaths)

    if numSystemsInPortfolio > 1:
        if numSystemsInPortfolio != len(digits) or numSystemsInPortfolio != len(strategyIDs) or numSystemsInPortfolio != len(spreads) or numSystemsInPortfolio != len(rateRequirements) or numSystemsInPortfolio != len(symbolRequirements) or numSystemsInPortfolio != len(timeframeRequirements) or numSystemsInPortfolio != len(minimumStops):
            print numSystemsInPortfolio
            print len(digits)
            print len(strategyIDs)
            print len(spreads)
            print len(rateRequirements)


            print "You must have the same number of items for minimum stops, requirement arrays, setFile, pair, spread, digits, and strategyID"
            return False
    else:
        if len(symbols) != len(digits) or len(symbols) != len(spreads) or len(spreads) != len(digits) or len(minimumStops) != len(digits):
            print "For multi-instrument tests the number of symbols, minimum stop values, digits and spreads must be equal."
            return False

    numPairs = len(symbols)

    barsNeeded = [None] * 200
    ratesNeededSymbol = [None] * 200
    ratesNeededTF = [None] * 200

    for i in range(0, numSystemsInPortfolio):

        rateRequirementItems = []
        symbolRequirementItems = []
        timeframeRequirementItems = []

        for s in rateRequirements[i].split(','): rateRequirementItems.append(int(s))
        for s in symbolRequirements[i].split(','): symbolRequirementItems.append(s)
        for s in timeframeRequirements[i].split(','): timeframeRequirementItems.append(int(s))

        if len(rateRequirementItems) < 10:
            for j in range(len(rateRequirementItems), 10):
                rateRequirementItems.append(0)

        if len(symbolRequirementItems) < 10:
            for j in range(len(symbolRequirementItems), 10):
                symbolRequirementItems.append("N")

        if len(timeframeRequirementItems) < 10:
            for j in range(len(timeframeRequirementItems), 10):
                timeframeRequirementItems.append(0)

        barsNeeded[strategyIDs[i]] = rateRequirementItems
        ratesNeededSymbol[strategyIDs[i]] = symbolRequirementItems
        ratesNeededTF[strategyIDs[i]] = timeframeRequirementItems

    #init tester    
    astdll.initCTesterFramework(asirikuyCtesterLogPath, config.getint("misc", "logSeverity"))

    #Read set files
    sets = []
    for index, s in enumerate(setFilePaths):
        sets.append(MT4Set(s))
        if not sets[index].content:
            print "Error reading set file %s" % (s)
            return False

        
    #Optimization 
    optimize            = config.getboolean("optimization", "optimize")
    optimizationType    = config.getint("optimization", "optimizationType")
    numCores            = config.getint("optimization", "numCores")

    #Config values
    accountCurrency = config.get("account", "currency")
    
    #hardcode brokerName and refBrokerName
    #brokerName        = config.get("account", "brokerName")
    #refBrokerName     = config.get("account", "refBrokerName")
    
    brokerName        = "Pepperstone Group Limited"
    refBrokerName     = "Pepperstone Group Limited"
    
    
    #passedTimeFrame   = config.get("strategy", "passedTimeFrame")

    try:
        generate_plot = config.getboolean("account", "generate_plot")
    except:
        generate_plot = False

    try:
        generate_html = config.getboolean("account", "generate_html")
    except:
        generate_html = False

    try:
        is_calculate_expectancy = int(config.get("account", "calculate_expectancy"))
    except:
        is_calculate_expectancy = 0

    try:
        write_xml     = config.get("account", "write_xml")
    except:
        write_xml     = False

    #Settings values
    optimizationParams = []
    optimizationArrays = []
    numOptimizationParams = []
    paramNamesArray = []

    TestSettingsArrayType = TestSettings * numSystemsInPortfolio
    testSettings = TestSettingsArrayType()
    SettingsArrayType = numPairs * ctypes.POINTER(c_double)
    settings = SettingsArrayType()
    for i in range(numSystemsInPortfolio):
        settings[i] = SettingsType()
        settings[i][IS_BACKTESTING]       = True
        settings[i][DISABLE_COMPOUNDING]  = float(sets[i].mainParams["DISABLE_COMPOUNDING"]['value']) if sets[i].content.has_option('main',  'DISABLE_COMPOUNDING') else 0
        settings[i][TIMED_EXIT_BARS]      = float(sets[i].mainParams["TIMED_EXIT_BARS"]['value']) if sets[i].content.has_option('main',  'TIMED_EXIT_BARS') else 0
        settings[i][ORIGINAL_EQUITY]      = config.getfloat("account", "balance")
        settings[i][OPERATIONAL_MODE]     = 1
        settings[i][STRATEGY_INSTANCE_ID] = float(sets[i].mainParams["STRATEGY_INSTANCE_ID"]['value']) if sets[i].content.has_option('main',  'STRATEGY_INSTANCE_ID') else 0
        settings[i][INTERNAL_STRATEGY_ID] = strategyIDs[i]
        settings[i][RUN_EVERY_TICK]       = float(sets[i].mainParams["RUN_EVERY_TICK"]['value']) if sets[i].content.has_option('main',  'RUN_EVERY_TICK') else 0
        settings[i][INSTANCE_MANAGEMENT]  = float(sets[i].mainParams["INSTANCE_MANAGEMENT"]['value']) if sets[i].content.has_option('main',  'INSTANCE_MANAGEMENT') else 0
        settings[i][MAX_OPEN_ORDERS]      = float(sets[i].mainParams["MAX_OPEN_ORDERS"]['value']) if sets[i].content.has_option('main',  'MAX_OPEN_ORDERS') else 1
        settings[i][TIMEFRAME]            = float(sets[i].mainParams["STRATEGY_TIMEFRAME"]['value']) if sets[i].content.has_option('main',  'STRATEGY_TIMEFRAME') else 0
        settings[i][SAVE_TICK_DATA]       = float(sets[i].mainParams["SAVE_TICK_DATA"]['value']) if sets[i].content.has_option('main',  'SAVE_TICK_DATA') else 0
        settings[i][ANALYSIS_WINDOW_SIZE] = float(sets[i].mainParams["ANALYSIS_WINDOW_SIZE"]['value']) if sets[i].content.has_option('main',  'ANALYSIS_WINDOW_SIZE') else 0
        settings[i][PARAMETER_SET_POOL]   = float(sets[i].mainParams["PARAMETER_SET_POOL"]['value']) if sets[i].content.has_option('main',  'PARAMETER_SET_POOL') else 0
        settings[i][ACCOUNT_RISK_PERCENT] = float(sets[i].mainParams["ACCOUNT_RISK_PERCENT"]['value']) if sets[i].content.has_option('main',  'ACCOUNT_RISK_PERCENT') else 0
        settings[i][MAX_DRAWDOWN_PERCENT] = float(sets[i].mainParams["MAX_DRAWDOWN_PERCENT"]['value']) if sets[i].content.has_option('main',  'MAX_DRAWDOWN_PERCENT') else 0
        settings[i][MAX_SPREAD]           = float(sets[i].mainParams["MAX_SPREAD_PIPS"]['value']) if sets[i].content.has_option('main',  'MAX_SPREAD_PIPS') else 0
        settings[i][SL_ATR_MULTIPLIER]    = float(sets[i].mainParams["SL_ATR_MULTIPLIER"]['value']) if sets[i].content.has_option('main',  'SL_ATR_MULTIPLIER') else 0
        settings[i][TP_ATR_MULTIPLIER]    = float(sets[i].mainParams["TP_ATR_MULTIPLIER"]['value']) if sets[i].content.has_option('main',  'TP_ATR_MULTIPLIER') else 0
        settings[i][ATR_AVERAGING_PERIOD] = float(sets[i].mainParams["ATR_AVERAGING_PERIOD"]['value']) if sets[i].content.has_option('main',  'ATR_AVERAGING_PERIOD') else 0
        settings[i][ORDERINFO_ARRAY_SIZE] = settings[i][MAX_OPEN_ORDERS]+1;
        settings[i][ADDITIONAL_PARAM_8] = float(sets[i].additionalParams["DSL_EXIT_TYPE"]['value']) if sets[i].content.has_option('additional', 'DSL_EXIT_TYPE') else 0
           
        optimizationArrays.append(sets[i].optimizationArray)

        paramNamesArray.append(paramNames)
        index = 0
        for param in sets[i].content.items("additional"):
            if param[0].find(",") == -1:
                settings[i][paramIndexes[param[0].upper()]] = float(param[1])
                paramNamesArray[i][index] = param[0].upper()
                index += 1


        OptimizationParamType = OptimizationParam * len(optimizationArrays[i])
        optimizationParams.append(OptimizationParamType())

        numOptimizationParams.append(0)
        for key, value in optimizationArrays[i].iteritems():
            optimizationParams[i][numOptimizationParams[i]].index = key
            optimizationParams[i][numOptimizationParams[i]].start = value[0]
            optimizationParams[i][numOptimizationParams[i]].step  = value[1]
            optimizationParams[i][numOptimizationParams[i]].stop  = value[2]
            numOptimizationParams[i] += 1

        testSettings[i] = TestSettings()
        testSettings[i].spread = spreads[i]
        testSettings[i].is_calculate_expectancy = is_calculate_expectancy

    if numOptimizationParams[0] == 0 and optimize:
        print "Nothing to optimize. Check your set file."
        return True

    #Account info values
    accountInfoArrayType = numSystemsInPortfolio * ctypes.POINTER(c_double)
    accountInfo = accountInfoArrayType()   
   
    for i in range(numSystemsInPortfolio):
        accountInfo[i] = accountInfoType()
        accountInfo[i][IDX_ACCOUNT_NUMBER] = 0
        accountInfo[i][IDX_BALANCE] = config.getfloat("account", "balance")
        accountInfo[i][IDX_EQUITY] = accountInfo[i][IDX_BALANCE]
        accountInfo[i][IDX_MARGIN] = 0
        #TODO: here it is only support one contract size. Refer to mainwindow.py
        accountInfo[i][IDX_CONTRACT_SIZE] = config.getfloat("account", "contractSize")
        accountInfo[i][IDX_LEVERAGE] = config.getfloat("account", "leverage")
        accountInfo[i][IDX_MINIMUM_STOP] = float(minimumStops[i])
        accountInfo[i][IDX_STOPOUT_PERCENT] = config.getfloat("account", "stopOutPercent")
        accountInfo[i][IDX_TOTAL_OPEN_TRADE_RISK_PERCENT] = 0 #UI Values. Ignore
        accountInfo[i][IDX_LARGEST_DRAWDOWN_PERCENT] = 0 #UI Values. Ignore

    minLotSize = config.getfloat("account", "minLotSize")

    #set initial balance
    global initialBalance
    initialBalance = accountInfo[0][IDX_BALANCE]

    clearScreen();

    #Load Rates 
    fromDate = calendar.timegm(strptime(config.get("strategy", "fromDate"),"%Y-%m-%d"))
    toDate = calendar.timegm(strptime(config.get("strategy", "toDate"),"%Y-%m-%d"))
    for i in range(numPairs):
        testSettings[i].fromDate = fromDate
        testSettings[i].toDate = toDate
        testSettings.spread = spreads[i]

    numCandles = 0

    historyFilePaths = []
    endingDate = []
    
    
    for i in range(numPairs):
        historyFilePaths.append(historyPath + symbols[i] + '_' + str(passedTimeFrame[i]) + '.csv')
        result = checkRates(historyFilePaths[i])
        numCandles = max(result['numCandles'], numCandles)

    RatesType = Rate*numCandles
    RatesArrayType = 10 * ctypes.POINTER(Rate)
    MasterRatesArrayType = numPairs * ctypes.POINTER(ctypes.POINTER(Rate))
    ratesArray = MasterRatesArrayType()
    RatesInfoArrayType = numPairs * ctypes.POINTER(RateInfo)
    ratesInfoArray = RatesInfoArrayType()

    if len(list(set(historyFilePaths))) > 1:
        makeRatesCoherent(historyFilePaths)

    for symbol in list(set(symbols)):

        print "Generating quote files..."
        symbolList = list(symbol)
        baseName=symbolList[0]+symbolList[1]+symbolList[2]
        termName=symbolList[3]+symbolList[4]+symbolList[5]
        additionalName= ''.join(symbolList[6:])
        quoteRatePath = historyPath + symbol + '_' + str(passedTimeFrame[i]) + '.csv'
        loadRates(quoteRatePath, numCandles, symbol, True)

        if accountCurrency != baseName and accountCurrency != termName:
            newSymbolsList = [baseName+accountCurrency+additionalName, accountCurrency+baseName+additionalName, termName+accountCurrency+additionalName, accountCurrency+termName+additionalName]
            for newSymbol in newSymbolsList:
                quoteRatePath = historyPath + newSymbol + '_' + str(passedTimeFrame[i]) + '.csv'
                if os.path.isfile(quoteRatePath):
                    loadRates(quoteRatePath, numCandles, newSymbol, True)

    for i in range(numPairs):
        if not os.path.isfile(historyFilePaths[i]):
            print "Can't find history file: %s" % historyFilePaths[i]
            return False
        print "Loading rates for %s..." % (symbols[i])

        if numSystemsInPortfolio == 1:
            n = 0
        else:
            n = i

        ratesArray[i] = RatesArrayType()
        ratesInfoArray[i] = RatesInfoType()

        for j in range(0,9):

            if ratesNeededSymbol[strategyIDs[n]][j] != "N":

                ratesArray[i][j] = RatesType()
                ratesInfoArray[i][j].totalBarsRequired = 0

                if ratesNeededSymbol[strategyIDs[n]][j] != "D":
                    print "Loading additional rates, symbol %s..." % (ratesNeededSymbol[strategyIDs[n]][j])
                    additionalRatePath = historyPath + ratesNeededSymbol[strategyIDs[n]][j] + '_' + str(passedTimeFrame[i]) + '.csv'
                    symbolUsed=ratesNeededSymbol[strategyIDs[n]][j]
                else:
                    additionalRatePath = historyFilePaths[i]
                    symbolUsed=symbols[i]

                if ratesNeededTF[strategyIDs[n]][j] == 0:

                    #requested and passed rates sanity check
                    if int(settings[n][TIMEFRAME]) < int(passedTimeFrame[i]):
                        print "passedTimeFrame needs to be below required time frame for proper refactoring."
                        return False

                    result = loadRates(additionalRatePath, numCandles, symbolUsed, False)
                    ratesInfoArray[i][j].requiredTimeFrame = int(settings[n][TIMEFRAME])

                else:

                    #requested and passed rates sanity check
                    if int(ratesNeededTF[strategyIDs[n]][j]) < int(passedTimeFrame[i]):
                        print "passedTimeFrame needs to be below required time frame for proper refactoring."
                        return False

                    result = loadRates(additionalRatePath, numCandles, symbolUsed, False)
                    ratesInfoArray[i][j].requiredTimeFrame = ratesNeededTF[strategyIDs[n]][j]

                ratesArray[i][j] = result['rates']
                endingDate.append(result['endingDate'])
                ratesInfoArray[i][j].isEnabled = 1
                ratesInfoArray[i][j].actualTimeFrame = int(passedTimeFrame[i])
                ratesInfoArray[i][j].totalBarsRequired = barsNeeded[strategyIDs[n]][j]
                ratesInfoArray[i][j].ratesArraySize = barsNeeded[strategyIDs[n]][j] * 1.2 * ratesInfoArray[i][j].requiredTimeFrame/int(passedTimeFrame[i])
                ratesInfoArray[i][j].digits = digits[i]
                ratesInfoArray[i][j].point = 1 / (math.pow(10,ratesInfoArray[i][j].digits))
                #print "Checking rate consistency..."
                #for t in range(0, numCandles):
                #    if ratesArray[i][j][t].time != ratesArray[0][j][t].time:
                #        print "Rate array time stamps out of sync. Aborting simulation."
                #        return False


    clearScreen()

    #Initialize XML Tree
    if write_xml:
        xmlRoot = ET.Element('AsirikuyPortfolio', attrib = {'version': '1.1'})
        xmlHeader = ET.SubElement(xmlRoot, 'Header')
        ET.SubElement(xmlHeader, 'Deposit').text = str(settings[0][ORIGINAL_EQUITY])

        if fromDate <= ratesArray[0][0][0].time:
            ET.SubElement(xmlHeader, 'PeriodStart').text = datetime.datetime.fromtimestamp(ratesArray[0][0][0].time).isoformat()
        else:
            ET.SubElement(xmlHeader, 'PeriodStart').text = datetime.datetime.fromtimestamp(fromDate).isoformat()

        ET.SubElement(xmlHeader, 'PeriodEnd').text = datetime.datetime.fromtimestamp(endingDate[0]).isoformat()
        xmlTrades = ET.SubElement(xmlRoot, 'Trades')

        for i in range (numSystemsInPortfolio):
            xmlSystemInstance = ET.SubElement(xmlHeader, 'SystemInstance')
            xmlSystemInstance.set('id', str(i))
            ET.SubElement(xmlSystemInstance, 'AsirikuyID').text = str(strategyIDs[i])
            ET.SubElement(xmlSystemInstance, 'SystemName').text = strategies[strategyIDs[i]]
            ET.SubElement(xmlSystemInstance, 'Symbol').text = symbols[i]
            ET.SubElement(xmlSystemInstance, 'TimeFrame').text = str(settings[i][TIMEFRAME])
            ET.SubElement(xmlSystemInstance, 'RiskPercent').text = str(settings[i][ACCOUNT_RISK_PERCENT])
            #ET.SubElement(xmlSystemInstance, 'Deposit').text = str(settings[i][ORIGINAL_EQUITY])
            with open (setFilePaths[i], "r") as myfile: data = myfile.read()
            ET.SubElement(xmlSystemInstance, 'OtherParams').text = data


    #Run the test/optimization
    error_c = c_char_p();

    start = time()
    global f
    if optimize:
        if execUnderMPI == False or (execUnderMPI == True and rank == 1):
            f = open(outputOptimizationFile + ".txt", 'w', 0)
            f.write("Iteration, Symbol, NumTrades, Profit, maxDD, maxDDLength, PF, R2, ulcerIndex, Sharpe, CAGR, CAGR to Max DD, numShorts, numLongs, Set Parameters\n")

        OPTIMIZATION_UPDATE = CFUNCTYPE(c_void_p, TestResult, POINTER(c_double), c_int)
        optimizationUpdate_c = OPTIMIZATION_UPDATE(optimizationUpdate)

        OPTIMIZATION_FINISHED = CFUNCTYPE(c_void_p)
        optimizationFinished_c = OPTIMIZATION_FINISHED(optimizationFinished)

        astdll.runOptimizationMultipleSymbols.restype = c_int

        optimizationSettings = GeneticOptimizationSettings()

        optimizationSettings.population = config.getint("optimization", "population")
        optimizationSettings.crossoverProbability =  config.getfloat("optimization", "crossoverProbability")
        optimizationSettings.mutationProbability =  config.getfloat("optimization", "mutationProbability")
        optimizationSettings.migrationProbability =  config.getfloat("optimization", "migrationProbability")
        optimizationSettings.evolutionaryMode = config.getint("optimization", "evolutionaryMode")
        optimizationSettings.elitismMode = config.getint("optimization", "elitismMode")
        optimizationSettings.mutationMode = config.getint("optimization", "mutationMode")
        optimizationSettings.crossoverMode = config.getint("optimization", "crossoverMode")
        optimizationSettings.maxGenerations = config.getint("optimization", "maxGenerations")
        optimizationSettings.stopIfConverged = config.getint("optimization", "stopIfConverged")
        optimizationSettings.discardAssymetricSets = config.getint("optimization", "discardAssymetricSets")
        optimizationSettings.minTradesAYear = config.getint("optimization", "minTradesAYear")
        optimizationSettings.optimizationGoal = config.getint("optimization", "optimizationGoal")

        if not astdll.runOptimizationMultipleSymbols (
                ctypes.pointer(optimizationParams[0]),
                c_int(numOptimizationParams[0]),
                c_int(optimizationType),
                optimizationSettings,
                c_int(numCores),
                settings[0],
                ctypes.pointer(symbols),
                accountCurrency,
                brokerName,
                refBrokerName,
                accountInfo[0],
                ctypes.pointer(testSettings),
                ctypes.pointer(ratesInfoArray),
                c_int(numCandles),
                c_int(numPairs),
                ctypes.pointer(ratesArray),
                c_double(minLotSize),
                optimizationUpdate_c,
                optimizationFinished_c,
                byref(error_c)
        ):
            print "Error executing framework: " + str(error_c.value)
        else:
            if execUnderMPI == True:
                lines = comm.gather(lines, root = 0)
                if rank == 0:
                    print "Waiting results from other MPI instances"
                    for i in range(size):
                        print "Getting data from instance %d" % (i)
                        f.write(lines[i])
                    print "Optimization finished!"

        elapsed = (time() - start)

        f.close()

    else:
        print "Running test..."
        if (numSystemsInPortfolio == 1): #Simple Test
            TEST_UPDATE = CFUNCTYPE(c_void_p, c_int, c_double, OrderInfo, c_double, c_char_p)
            testUpdate_c = TEST_UPDATE(testUpdate)

            SIGNAL_UPDATE = CFUNCTYPE(c_void_p, TradeSignal)
            signalUpdate_c = SIGNAL_UPDATE(signalUpdate)

            shutil.copyfile (setFilePaths[0], outputFile + ".set")

            for i in range(numPairs):
                if numPairs > 1:
                    f = open(symbols[i]+"_"+outputFile + '.txt', 'w')

                else:
                    f = open(outputFile + ".txt", 'w')
                f.write("Order Number, Order Type, Open Time, Close Time, Open Price, Close Price, Lots, Profit, SL, TP, Balance, ID, Pair, Swap\n")

                _settings = SettingsArrayType()
                _settings[0] = settings[0]
                _symbols = symbolsArrayType()
                _symbols[0] = symbols[i]
                _accountInfo = accountInfoArrayType()
                _accountInfo[0] = accountInfo[0]

                accountInfo[0][IDX_MINIMUM_STOP] = float(minimumStops[i])

                _testSettings = TestSettingsArrayType()
                _testSettings[0] = testSettings[0]
                _ratesArray = MasterRatesArrayType()
                _ratesArray[0] = ratesArray[i]
                _ratesInfoArray = RatesInfoArrayType()
                _ratesInfoArray[0] = ratesInfoArray[i]

                astdll.runPortfolioTest.restype = TestResult
                result = astdll.runPortfolioTest (
                    c_int(1),ctypes.pointer(_settings), ctypes.pointer(_symbols), accountCurrency, brokerName, refBrokerName,
                    ctypes.pointer(_accountInfo), ctypes.pointer(_testSettings), ctypes.pointer(_ratesInfoArray), c_int(numCandles),
                    c_int(1), ctypes.pointer(_ratesArray), c_double(minLotSize), testUpdate_c, None, signalUpdate_c
                )
                if result.testId == 0:
                    print "Error executing framework: " + str(error_c.value)
                else:
                    testFinished(result)
                f.close()

                if numPairs > 1:
                    if settings[0][DISABLE_COMPOUNDING] == 1:
                        if generate_plot == True:
                            plotTestResult(symbols[i] + "_" + outputFile + ".txt")
                    else:
                        if generate_plot == True:
                            plotTestResult(symbols[i] + "_" + outputFile + ".txt", EXPONENTIAL_REGRESSION)
                    if generate_html == True:
                        csvToHTML(symbols[i]+"_"+outputFile + ".txt", symbols[i]+"_" + outputFile + ".htm", result, symbols[i]+"_" + outputFile + ".png")
                else:
                    if settings[0][DISABLE_COMPOUNDING] == 1:
                        if generate_plot == True:
                            plotTestResult(outputFile + ".txt")
                    else:
                        if generate_plot == True:
                            plotTestResult(outputFile + ".txt", EXPONENTIAL_REGRESSION)
                    if generate_html == True:
                        csvToHTML(outputFile + ".txt", outputFile + ".htm", result, outputFile + ".png")

        else: #Portfolio Test
            TEST_UPDATE = CFUNCTYPE(c_void_p, c_int, c_double, OrderInfo, c_double, c_char_p)
            testUpdate_c = TEST_UPDATE(testUpdate)

            SIGNAL_UPDATE = CFUNCTYPE(c_void_p, TradeSignal)
            signalUpdate_c = SIGNAL_UPDATE(signalUpdate)

            for i in range(numSystemsInPortfolio):
                shutil.copyfile (setFilePaths[i], outputFile + "_" + str(i) + ".set")

            f = open(outputFile + ".txt", 'w')
            f.write("Order Number, Order Type, Open Time, Close Time, Open Price, Close Price, Lots, Profit, SL, TP, Balance, ID, Pair, Swap\n")
            astdll.runPortfolioTest.restype = TestResult
            result = astdll.runPortfolioTest (
                c_int(1), ctypes.pointer(settings), ctypes.pointer(symbols), accountCurrency, brokerName, refBrokerName, ctypes.pointer(accountInfo),
                ctypes.pointer(testSettings), ctypes.pointer(ratesInfoArray), c_int(numCandles), c_int(numSystemsInPortfolio), ctypes.pointer(ratesArray), c_double(minLotSize),
                testUpdate_c, None, signalUpdate_c
            )
            print "finished executing"
            if result.testId == 0:
                print "Error executing framework: " + str(error_c.value)
            else:
                print "finished trying to execute testFinished"
                testFinished(result)
            f.close()
            systems = []
            for i in range(numSystemsInPortfolio):
                systems.append(strategies[strategyIDs[i]] + ' - ' + symbols[i])
            plotPortfolioTestResult(outputFile + ".txt", systemNames = systems)
            csvToHTML(outputFile + ".txt", outputFile + ".htm", result, outputFile + ".png")

        elapsed = (time() - start)

    if execUnderMPI == False or (execUnderMPI == True and rank ==0):
        print "Time elapsed:", str(datetime.timedelta(seconds = elapsed))

    print "total wall time: {}".format((datetime.datetime.now()-main_start_time).total_seconds())


##################################
###         FUNCTIONS         ####
##################################

def version():
    system = platform.system()
    if (system == "Windows"):
        asfdll = loadLibrary('AsirikuyFrameworkAPI')
        astdll = loadLibrary('CTesterFrameworkAPI')
    elif (system == "Linux"):
        asfdll = loadLibrary('libAsirikuyFrameworkAPI.so')
        astdll = loadLibrary('libCTesterFrameworkAPI.so')
    elif (system == "Darwin"):
        asfdll = loadLibrary('libAsirikuyFrameworkAPI.dylib')
        astdll = loadLibrary('libCTesterFrameworkAPI.dylib')
    else:
        print "No shared library loading support for OS %s" % (system)
        return False

    ASKFrameworkVersion = getASKFrameworkVersion(asfdll)
    ASTFrameworkVersion = getASTFrameworkVersion(astdll)

    print "AsirikuyFrameworkAPI v"+ASKFrameworkVersion
    print "CTesterFrameworkAPI v"+ASTFrameworkVersion
    print "New Strategy Tester v" + VERSION
    quit()


def optimizationUpdate(testResults, settings, numSettings):
    global f, paramNames, iterationNumber, execUnderMPI, rank, lines

    if execUnderMPI == False:
        iterationNumber+=1
        print "Iteration %d finished" % (iterationNumber)

    parameters = []
    lines = ""

    if testResults.maxDDDepth != 0:
        ratio = testResults.cagr/testResults.maxDDDepth
    else:
        ratio = 0

    for i in range(numSettings):
        parameters.append("%s=%lf" % (paramNames[int(settings[i*2])], settings[i*2+1]))

    if execUnderMPI == False:
        line = "%d,%s,%d,%lf,%lf,%d,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%s\n" % (iterationNumber, testResults.symbol, testResults.totalTrades, testResults.finalBalance-initialBalance, testResults.maxDDDepth,
                                                                           int(testResults.maxDDLength/60/60/24), float(testResults.pf), testResults.r2, testResults.ulcerIndex, testResults.sharpe, testResults.cagr, ratio,
                                                                           testResults.numShorts, testResults.numLongs, " ".join(parameters))
    else:
        line = "%d,%s,%d,%lf,%lf,%d,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%s\n" % (iterationNumber, testResults.symbol, testResults.totalTrades,testResults.finalBalance-initialBalance, testResults.maxDDDepth,
                                                                           int(testResults.maxDDLength/60/60/24), float(testResults.pf), testResults.r2, testResults.ulcerIndex, testResults.sharpe, testResults.cagr, ratio,
                                                                           testResults.numShorts, testResults.numLongs, " ".join(parameters))
    if execUnderMPI == False:
        f.write(line)
    else:
        if rank == 1:
            iterationNumber+=1
            iterationString = "%d," % (iterationNumber)
            lines = iterationString + line
            print "Iteration %d finished, %d" % (iterationNumber, rank)
            for i in range(2, size):
                iterationNumber+=1
                iterationString = "%d," % (iterationNumber)
                lines = lines + iterationString + comm.recv(source=i, tag=11)
                print "Iteration %d finished, %d" % (iterationNumber, rank)
            f.write(lines)
        else:
            comm.send(line, dest=1, tag=11)

def optimizationFinished():
    if execUnderMPI == False:
        print "Optimization finished!!"

def testUpdate(testId, percentageOfTestCompleted, lastTrade, currentBalance, symbol):
    #print "Test running %.2lf%%" % (percentageOfTestCompleted)
    #clearScreen();
    #print "Last closed trade date is {0}".format(strftime("%d/%m/%Y %H:%M", gmtime(lastTrade.closeTime)))
    global f

    f.write("%d,%s,%s,%s,%.5lf,%.5lf,%lf,%.2lf,%.5lf,%.5lf,%.2lf,%d,%s,%.2lf\n" % (
        int(lastTrade.ticket), opType[int(lastTrade.type)], strftime("%d/%m/%Y %H:%M", gmtime(lastTrade.openTime)), strftime("%d/%m/%Y %H:%M", gmtime(lastTrade.closeTime)),
        lastTrade.openPrice, lastTrade.closePrice, lastTrade.lots, lastTrade.profit, lastTrade.stopLoss, lastTrade.takeProfit, currentBalance,testId,symbol, lastTrade.swap)
            )

def signalUpdate(tradeSignal):
    #print "Test running %.2lf%%" % (percentageOfTestCompleted)
    global xmlRoot, numSystemsInPortfolio, signalType, signalCounter, write_xml

    if write_xml:
        xmlTrades = xmlRoot.find('Trades')
        xmlTrade = ET.SubElement(xmlTrades, 'Trade')
        xmlTrade.set('instance', str(int(tradeSignal.testId)))
        xmlTrade.set('no', str(int(tradeSignal.no)))
        xmlTrade.set('time', datetime.datetime.fromtimestamp(tradeSignal.time).isoformat())
        xmlTrade.set('type', signalType[tradeSignal.type])
        xmlTrade.set('order', str(tradeSignal.orderId))
        xmlTrade.set('size', str(tradeSignal.lots))
        xmlTrade.set('price', str(tradeSignal.price))
        xmlTrade.set('sl', str(tradeSignal.sl))
        xmlTrade.set('tp', str(tradeSignal.tp))
        xmlTrade.set('profit', str(tradeSignal.profit))
        xmlTrade.set('balance',  str(tradeSignal.balance))

    signalCounter = signalCounter + 1

def testFinished(testResult):
    global xmlRoot, outputXMLPath, numSystemsInPortfolio,configFilePath, write_xml

    print ' '

    with open(configFilePath, 'r') as fin:
        print fin.read()

    print ' '
    print 'Test finished!!'
    print 'Total trades: %d' % (testResult.totalTrades)
    print 'Longs: %d Shorts: %d' % (testResult.numLongs, testResult.numShorts)
    print 'Final balance: %.2lf$' % (testResult.finalBalance)
    print 'Max DD: %.2lf$' % (testResult.maxDDDepth)
    print 'Max DD Length: %d' % (int(testResult.maxDDLength/60/60/24))
    print 'PF: %.2lf' % (testResult.pf)
    print 'R2: %.2lf' % (testResult.r2)
    print 'Ulcer Index: %.2lf' % (testResult.ulcerIndex)

    if write_xml:
        f = open(outputXMLPath, "w")
        f.write('<?xml version="1.0" encoding="UTF-8"?>\n<?xml-stylesheet type="text/xsl" href="include/ast.xsl"?>\n')


        xmlHeader = xmlRoot.find('Header')
        ET.SubElement(xmlHeader, 'PortfolioName').text = ''
        ET.SubElement(xmlHeader, 'NoOfTrades').text = str(signalCounter)
        f.write(ET.tostring(xmlRoot, encoding="UTF-8"))
        f.close()

def stopOptimization(signal, frame):
    global config,  astdll

    if config.getboolean("optimization", "optimize"):
        if config.getint("optimization", "optimizationType") == OPTI_BRUTE_FORCE:
            print 'Stopping optimization...'
            sys.stdout.flush()
            astdll.stopOptimization()
        elif config.getint("optimization", "optimizationType") == OPTI_GENETIC:
            print 'The optimization will stop when then current generation is finished'
            sys.stdout.flush()
            astdll.stopOptimization()

##################################
###           MAIN           ####
##################################

if __name__ == "__main__": main()
