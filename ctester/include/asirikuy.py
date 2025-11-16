from ctypes import *
from time import *
import ntpath
import re, os, ctypes,  csv,  calendar, datetime, configparser
from include.fastcsv import fastcsv
import io
import base64
import pandas as pd

VERSION = "0.56"

def loadLibrary(library):
    """Load a shared library, checking multiple possible locations."""
    if os.name == 'nt':
        # Windows: try current directory and build directories
        possible_paths = [
            library,
            f"../bin/gmake/x64/Debug/{library}",
            f"../bin/gmake/x64/Release/{library}",
        ]
        for path in possible_paths:
            if os.path.exists(path):
                return windll.LoadLibrary(path)
        return windll.LoadLibrary(library)  # Fallback to system search
    elif os.name == 'posix':
        # Unix/Linux/macOS: try current directory and build directories
        possible_paths = [
            library,
            f"../bin/gmake/x64/Debug/{library}",
            f"../bin/gmake/x64/Release/{library}",
            f"./bin/gmake/x64/Debug/{library}",
            f"./bin/gmake/x64/Release/{library}",
        ]
        for path in possible_paths:
            if os.path.exists(path):
                return cdll.LoadLibrary(path)
        return cdll.LoadLibrary(library)  # Fallback to system search
    else:
        return None

def csvToHTML(csvFile, htmlFile, testResult, imageFile):
    reader = csv.reader(open(csvFile))
    htmlfile = open(htmlFile,"w")
    rownum = 0
    htmlfile.write('<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.10.1/jquery.min.js"></script>')
    htmlfile.write('<script src="http://ajax.aspnetcdn.com/ajax/jquery.dataTables/1.9.4/jquery.dataTables.min.js"></script>')
    htmlfile.write('<script>\n\n' + open('include/ast.js', 'rb').read() + '</script>')
    htmlfile.write('<style>\n\n' + open('include/ast.css', 'rb').read() + '</style>')
    data_uri = base64.b64encode(open(imageFile, 'rb').read()).decode('utf-8').replace('\n', '')
    htmlfile.write ('<img src="data:image/png;base64,{0}">'.format(data_uri))
    htmlfile.write('<table id=summary><thead>')
    htmlfile.write('<th>Total trades</th>')
    htmlfile.write('<th>Longs</th>')
    htmlfile.write('<th>Shorts</th>')
    htmlfile.write('<th>Final Balance</th>')
    htmlfile.write('<th>max DD</th>')
    htmlfile.write('<th>max DD Length</th>')
    htmlfile.write('<th>PF</th>')
    htmlfile.write('<th>R2</th>')
    htmlfile.write('<th>Ulcer Index</th>')
    htmlfile.write('</thead><tr>')
    htmlfile.write('<td>%d</td>' %(testResult.totalTrades))
    htmlfile.write('<td>%d</td>' %(testResult.numLongs))
    htmlfile.write('<td>%d</td>' %(testResult.numShorts))
    htmlfile.write('<td>%.2lf</td>' %(testResult.finalBalance))
    htmlfile.write('<td>%.2lf</td>' %(testResult.maxDDDepth))
    htmlfile.write('<td>%.0lf</td>' %(testResult.maxDDLength/60/60/24))
    htmlfile.write('<td>%.2lf</td>' %(testResult.pf))
    htmlfile.write('<td>%.2lf</td>' %(testResult.r2))
    htmlfile.write('<td>%.2lf</td>' %(testResult.ulcerIndex))
    htmlfile.write('</tr></table>')

    htmlfile.write('<table id=trades>')
    for row in reader:
        if rownum == 0:
            htmlfile.write('<thead>')
            for column in row:
                htmlfile.write('<th>' + column + '</th>')
            htmlfile.write('</thead>')
        else:
            colnum = 1
            htmlfile.write('<tr>')
            for column in row:
                htmlfile.write('<td class="column_' + str(colnum) + '">' + column + '</td>')
                colnum += 1
            htmlfile.write('</tr>')
        rownum += 1
    htmlfile.write('</table>')

def path_leaf(path):
    head, tail = ntpath.split(path)
    return tail or ntpath.basename(head)

def makeRatesCoherent(historyFilePathsInput):

    print("Making rates coherent")
    historyFilePaths = list(set(historyFilePathsInput))

    print("Processing {0} rate files".format(len(historyFilePaths)))
    #add swap rates if needed
    for historyFilePath1 in historyFilePaths:
        with fastcsv.Reader(io.open(historyFilePath1)) as reader:
            numCandles = sum(1 for row in reader)

        with fastcsv.Reader(io.open(historyFilePath1)) as reader:
            for row in reader:
                numberOfColumns = len(row)
                break

            basePath = os.path.abspath(os.path.join(str(historyFilePath1), os.path.pardir))
            if numberOfColumns < 8:
                symbolList=list(path_leaf(historyFilePath1))
                baseName=symbolList[0]+symbolList[1]+symbolList[2]
                termName=symbolList[3]+symbolList[4]+symbolList[5]
                addSwapToRates(historyFilePath1,  basePath + "/SWAP_" + baseName + ".csv" , basePath + "/SWAP_" + termName + ".csv")

    #load rates into memory
    allRates = []

    for index, historyFilePath1 in enumerate(historyFilePaths):

        print("loading {}".format(historyFilePath1))
        loaded_series = pd.read_csv(historyFilePath1, index_col=0, parse_dates=True, dayfirst=True)
        loaded_series.columns = ["open_{}".format(index), "high_{}".format(index), "low_{}".format(index), "close_{}".format(index), "volume_{}".format(index), "swap0_{}".format(index), "swap1_{}".format(index)]
        
        loaded_series = loaded_series.loc[~loaded_series.index.duplicated(keep='first')]
        
        if index == 0:
            rates = loaded_series
        else:
            rates = pd.concat([rates, loaded_series], axis=1)

    rates = rates.dropna()
    
    for index, historyFilePath1 in enumerate(historyFilePaths):
        rates_to_save = rates[rates.columns[(index*7):(index*7+7)]]
        rates_to_save.to_csv(historyFilePath1, date_format="%d/%m/%y %H:%M", header=False)

def loadRates(historyFilePath1, arbitraryNum, symbol, updateQuotes):

    # Python 3: symbol may be bytes, decode to string first
    symbol_str = symbol.decode('utf-8') if isinstance(symbol, bytes) else symbol
    symbolList=list(symbol_str)
    baseName=symbolList[0]+symbolList[1]+symbolList[2]
    termName=symbolList[3]+symbolList[4]+symbolList[5]

    with fastcsv.Reader(io.open(historyFilePath1)) as reader:
        if arbitraryNum == 0:
            numCandles = sum(1 for row in reader)
            print(numCandles)
        else:\
            numCandles = arbitraryNum

        for row in reader:
            numberOfColumns = len(row)
            break

        basePath = os.path.abspath(os.path.join(str(historyFilePath1), os.path.pardir))

        if numberOfColumns < 8:
            addSwapToRates(historyFilePath1,  basePath + "/SWAP_" + baseName + ".csv" , basePath + "/SWAP_" + termName + ".csv")

    with fastcsv.Reader(io.open(historyFilePath1)) as reader:

        RatesType = Rate * numCandles
        rates = RatesType()

        for i,row in enumerate(reader):
            if i < numCandles:
                year = int(row[0][6:8])
                if year > 50:
                    year += 1900
                else:
                    year += 2000

                time = int((datetime.datetime(int(year), int(row[0][3:5]), int(row[0][0:2]), int(row[0][9:11]), int(row[0][12:14]), 0)-datetime.datetime(1970,1,1)).total_seconds())
                rates[i].time = time
                rates[i].open = float(row[1])
                rates[i].high = float(row[2])
                rates[i].low = float(row[3])
                rates[i].close = float(row[4])
                rates[i].volume = float(row[5])
                rates[i].shortSwap = float(row[6])
                rates[i].longSwap = float(row[7])

    endingDate = time

    if updateQuotes:
        # Python 3: use 'w' mode for text, not 'wb' for binary
        with open(baseName+termName+'_QUOTES.csv', 'w', newline='') as f:
            spamwriter  = csv.writer(f, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)
            for i in range(0, numCandles):
                spamwriter.writerow([str(strftime("%d/%m/%y %H:%M", gmtime(rates[i].time))), str(rates[i].open)])
        return

    return {'rates':rates, 'numCandles':numCandles, 'endingDate':endingDate}

def addSwapToRates(historyFilePath1,  baseFilePath, termFilePath):

    import datetime

    if os.path.exists(baseFilePath) == False or os.path.exists(termFilePath) == False:
        print("Cannot generate swap information, no available interest rate data for base or term currencies")
        return

    print("Loading rates for swap generation..")
    with fastcsv.Reader(io.open(historyFilePath1)) as reader:
        numCandles = sum(1 for row in reader)

    with fastcsv.Reader(io.open(historyFilePath1)) as reader:
        RatesType = Rate * numCandles
        rates = RatesType()
        baseBid = []
        baseAsk = []
        baseTime = []
        termTime = []
        termBid = []
        termAsk = []
        timeString = []
        ratesTermBid = list(range(numCandles))
        ratesTermAsk = list(range(numCandles))
        ratesBaseBid = list(range(numCandles))
        ratesBaseAsk = list(range(numCandles))
        i=0
        for row in reader:
            if i < numCandles:
                year = int(row[0][6:8])
                if year > 50:
                    year += 1900
                else:
                    year += 2000

                time = int((datetime.datetime(int(year), int(row[0][3:5]), int(row[0][0:2]), int(row[0][9:11]), int(row[0][12:14]), 0)-datetime.datetime(1970,1,1)).total_seconds())

                timeString.append(row[0])
                rates[i].time = time
                rates[i].open = float(row[1])
                rates[i].high = float(row[2])
                rates[i].low = float(row[3])
                rates[i].close = float(row[4])
                rates[i].volume = float(row[5])
                i=i+1

        with fastcsv.Reader(io.open(baseFilePath)) as reader:
            baseDataSize=0
            for index,row in enumerate(reader):
                year = int(row[2][6:8])
                if year > 50:
                    year += 1900
                else:
                    year += 2000

                time = int((datetime.datetime(int(year), int(row[2][3:5]), int(row[2][0:2]), int(row[2][9:11]), int(row[2][12:14]), 0)-datetime.datetime(1970,1,1)).total_seconds())

                baseTime.append(time)
                baseBid.append(float(row[0]))
                baseAsk.append(float(row[1]))
                baseDataSize+=1;

        with fastcsv.Reader(io.open(termFilePath)) as reader2:
            termDataSize=0
            for row in reader2:

                year = int(row[2][6:8])
                if year > 50:
                    year += 1900
                else:
                    year += 2000

                time = int((datetime.datetime(int(year), int(row[2][3:5]), int(row[2][0:2]), int(row[2][9:11]), int(row[2][12:14]), 0)-datetime.datetime(1970,1,1)).total_seconds())

                termTime.append(time)
                termBid.append(float(row[0]))
                termAsk.append(float(row[1]))
                termDataSize += 1

        print("Using %d interest rate changes for base currency" % baseDataSize)
        print("Using %d interest rate changes for term currency" % termDataSize)

        print("Generating all long/short swap data for each candle...")
        for j in range(0, numCandles):
            ratesTermBid[j]=termBid[0]
            ratesTermAsk[j]=termAsk[0]
            for i in range(0, termDataSize):
                if rates[j].time > termTime[i] and i>0:
                    ratesTermBid[j]=termBid[i-1]
                    ratesTermAsk[j]=termAsk[i-1]

        for j in range(0, numCandles):
            ratesBaseBid[j]=baseBid[0]
            ratesBaseAsk[j]=baseAsk[0]
            for i in range(0, baseDataSize):
                if rates[j].time > baseTime[i] and i>0:
                    ratesBaseBid[j]=baseBid[i-1]
                    ratesBaseAsk[j]=baseAsk[i-1]
        print("Saving data back to file...")

        from datetime import datetime
        with open(historyFilePath1, 'wb') as f:
            spamwriter = csv.writer(f, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)
            for i in range(0, numCandles):
                spamwriter.writerow([timeString[i], str(rates[i].open), str(rates[i].high), str(rates[i].low), str(rates[i].close), str(rates[i].volume), str(ratesBaseBid[i]-ratesTermAsk[i]), str(ratesTermBid[i]-ratesBaseAsk[i])])

def checkRates(historyFilePath1):
    with fastcsv.Reader(io.open(historyFilePath1)) as reader:
        numCandles = sum(1 for row in reader)
    return {'numCandles':numCandles}

def readConfigFile(file):
    try:
        config = configparser.RawConfigParser()
        config.read(file)
        return config
    except Exception as e:
        print(e)
    except:
        return False

def readSetFile(file):
    try:
        config = configparser.RawConfigParser()
        config.read_file(FakeSecHead(open(file)))
        return config
    except:
        return False

def getASKFrameworkVersion(asfdll):
    pMajor = c_int()
    pMinor = c_int()
    pbugFix = c_int()
    asfdll.getFrameworkVersion.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
    asfdll.getFrameworkVersion(byref(pMajor), byref(pMinor), byref(pbugFix))
    return "%d.%d.%d" % (pMajor.value, pMinor.value,pbugFix.value)

def getASTFrameworkVersion(astdll):
    pMajor = c_int()
    pMinor = c_int()
    pbugFix = c_int()
    astdll.getCTesterFrameworkVersion.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
    astdll.getCTesterFrameworkVersion(byref(pMajor), byref(pMinor), byref(pbugFix))
    return "%d.%d.%d" % (pMajor.value, pMinor.value,pbugFix.value)

global signalType
signalType = [
    'buy',
    'sell',
    'modify',
    'close',
    's/l',
    't/p'
]

class TradeSignal(Structure):
    _fields_ = [
        ("testId", c_int),
        ("no", c_int),
        ("time", c_int),
        ("type", c_int),
        ("orderId", c_int),
        ("lots", c_double),
        ("price", c_double),
        ("sl", c_double),
        ("tp", c_double),
        ("profit", c_double),
        ("balance", c_double)
    ]

class TestResult(Structure):
    _fields_ = [
        ("testId", c_int),
        ("totalTrades", c_int),
        ("cagr", c_double),
        ("sharpe", c_double),
        ("martin", c_double),
        ("risk_reward", c_double),
        ("winning",c_double),
        ("finalBalance", c_double),
        ("maxDDDepth", c_double),
        ("maxDDLength", c_double),
        ("pf", c_double),
        ("r2", c_double),
        ("ulcerIndex", c_double),
        ("avgTradeDuration",c_double),
        ("numShorts", c_int),
        ("numLongs", c_int),
        ("yearsTraded", c_double),
        ("symbol", c_char*5000)
    ]



global goalsDesc
goalsDesc = [
    'Balance',
    'Max DD',
    'Max DD Length',
    'Profit Factor',
    'R2',
    'Ulcer Index',
    'Sharpe',
    'CAGR to Max DD ratio',
    'CAGR'
]

class TestSettings(Structure):
    _fields_ = [
        ("spread", c_double),
        ("fromDate", c_int),
        ("toDate", c_int),
        ("is_calculate_expectancy", c_int)
    ]

class Rate(Structure):
    _fields_ = [
        ("open", c_double),
        ("high", c_double),
        ("low", c_double),
        ("close", c_double),
        ("volume", c_double),
        ("shortSwap", c_double),
        ("longSwap", c_double),
        ("time", c_int)
    ]

class OrderInfo(Structure):
    _fields_ = [
        ("ticket", c_double),
        ("instanceId", c_double),
        ("type", c_double),
        ("openTime", c_double),
        ("closeTime", c_double),
        ("stopLoss", c_double),
        ("takeProfit", c_double),
        ("expiration", c_double),
        ("openPrice", c_double),
        ("closePrice", c_double),
        ("lots", c_double),
        ("profit", c_double),
        ("commission", c_double),
        ("swap", c_double),
        ("isOpen", c_double)
    ]

class RateInfo(Structure):
    _fields_ = [
        ("isEnabled", c_double),
        ("requiredTimeFrame", c_double),
        ("totalBarsRequired", c_double),
        ("actualTimeFrame", c_double),
        ("ratesArraySize", c_double),
        ("point", c_double),
        ("digits", c_double)
    ]


MAX_RATES_BUFFERS = 10
RatesInfoType = RateInfo * MAX_RATES_BUFFERS

#Strategy IDs
strategies = [
    'WATUKUSHAY_FE_BB', #0
    'WATUKUSHAY_FE_CCI',#1
    'ATIPAQ',           #2
    'AYOTL',            #3
    'COATL',            #4
    'COMITL_BB',        #5
    'COMITL_KC',        #6
    'COMITL_PA',        #7
    'GODS_GIFT_ATR',    #8
    'QALLARYI',         #9
    'QUIMICHI',         #10
    'SAPAQ',            #11
    'ASIRIKUY_BRAIN',   #12
    'TEYACANANI',       #13
    'WATUKUSHAY_FE_RSI',#14
    'RUPHAY',           #15
    'TEST_EA',          #16
    'EURCHF_GRID',      #17
    'KANTU',            #18
    'RECORD_BARS',      #19
    'MUNAY',            #20
    'RENKO_TEST',       #21
    'PKANTU_ML',        #22
	'KANTU_RL',			#23
	'KELPIE',			#24
    'BBS',              #25
    'TakeOver',         #26
    'SCREENING',        #27
    'KEYK',             #28
    'AUTOBBS',          #29
    'AUTOBBSWEEKLY'     #30
]

opType = [
    'BUY',
    'SELL'
]

#Define regression types
LINEAR_REGRESSION      = 1
EXPONENTIAL_REGRESSION = 0

#Defines for errors
SUCCESS               = 0
INVALID_CURRENCY      = 3000
UNKNOWN_SYMBOL        = 3001
SYMBOL_TOO_SHORT      = 3002
PARSE_SYMBOL_FAILED   = 3003
NO_CONVERSION_SYMBOLS = 3004
NULL_POINTER          = 3005
INVALID_STRATEGY      = 3006
NOT_ENOUGH_MARGIN     = 3007
SPREAD_TOO_WIDE       = 3008
NOT_ENOUGH_RATES_DATA = 3009
WORST_CASE_SCENARIO   = 3010
NORMALIZE_BARS_FAILED = 3011
INIT_LOG_FAILED       = 3012
DEINIT_LOG_FAILED     = 3013
ZERO_DIVIDE           = 3014
TA_LIB_ERROR          = 3015
INVALID_TIME_OFFSET   = 3016
INVALID_PARAMETER     = 3017
NN_TRAINING_FAILED    = 3018
UNKNOWN_TIMEZONE      = 3019
LOCAL_TZ_MISMATCH     = 3020
BROKER_TZ_MISMATCH    = 3021
TOO_MANY_INSTANCES    = 3022
INVALID_CONFIG        = 3023
MISSING_CONFIG        = 3024
INIT_XML_FAILED       = 3025
UNKNOWN_INSTANCE_ID   = 3026
INSUFFICIENT_MEMORY   = 3027
WAIT_FOR_INIT         = 3028

#Defines for Settings
SettingsType = c_double * 64
global paramIndexes
paramIndexes = dict(
    MA_SHORT_PERIOD=0,
    SELECTED_STRATEGY_ID=0,
    DAILY_TREND=0,
    AUTOBBS_ADJUSTPOINTS=0,
    TESTEA_ADJUSTPOINTS=0,
    KK_SL_MODE=0,
    ADDITIONAL_PARAM_1=0,
    MA_LONG_PERIOD=1,
    VOLATILITY_CALCULATION_MODE=1,
    BBS_PERIOD=1,
    KK_MULTIPLE_ATR=1,
    AUTOBBS_TP_MODE=1,
    ADDITIONAL_PARAM_2=1,
    TL_ATR_MULTIPLIER =2,
    BBS_DEVIATION=2,
    KK_ADJUSTPOINTS=2,
    AUTOBBS_TREND_MODE=2,
    ADDITIONAL_PARAM_3=2,
    USE_AFMTL=3,
    BBS_ADJUSTPOINTS=3,
    AUTOBBS_RISK_CAP=3,
    ADDITIONAL_PARAM_4=3,
    SELECT_KANTU_SYSTEM_FILE=4,
    BBS_TP_MODE=4,
    AUTOBBS_LONG_SHORT_MODE=4,
    ADDITIONAL_PARAM_5=4,
    DSL_BREAKEVEN_BARS=5,
    BBS_SL_MODE=5,
    AUTOBBS_MAX_ACCOUNT_RISK=5,
    ADDITIONAL_PARAM_6=5,
    DSL_TYPE=6,
    AUTOBBS_KEYK=6,
    ADDITIONAL_PARAM_7=6,
    DSL_EXIT_TYPE     =7,
    AUTOBBS_RANGE     =7,
    ADDITIONAL_PARAM_8=7,  
    AUTOBBS_MAX_STRATEGY_RISK = 8,  
    ADDITIONAL_PARAM_9   = 8,
    AUTOBBS_MACRO_TREND = 9,
    ADDITIONAL_PARAM_10  = 9,
    AUTOBBS_EXECUTION_RATES = 10,
    ADDITIONAL_PARAM_11  = 10,
    AUTOBBS_ONE_SIDE     = 11,
    ADDITIONAL_PARAM_12  = 11,
    AUTOBBS_IS_AUTO_MODE = 12,
    ADDITIONAL_PARAM_13  = 12,
    AUTOBBS_IS_ATREURO_RANGE = 13,
    ADDITIONAL_PARAM_14  = 13,
    AUTOBBS_STARTHOUR = 14,
    ADDITIONAL_PARAM_15  = 14,
    AUTOBBS_VIRTUAL_BALANCE_TOPUP = 15,
    ADDITIONAL_PARAM_16  = 15,
    ADDITIONAL_PARAM_17  = 16,
    ADDITIONAL_PARAM_18  = 17,
    ADDITIONAL_PARAM_19  = 18,
    ADDITIONAL_PARAM_20  = 19,
    ADDITIONAL_PARAM_21  = 20,
    ADDITIONAL_PARAM_22  = 21,
    ADDITIONAL_PARAM_23  = 22,
    ADDITIONAL_PARAM_24  = 23,
    ADDITIONAL_PARAM_25  = 24,
    ADDITIONAL_PARAM_26  = 25,
    ADDITIONAL_PARAM_27  = 26,
    ADDITIONAL_PARAM_28  = 27,
    ADDITIONAL_PARAM_29  = 28,
    ADDITIONAL_PARAM_30  = 29,
    ADDITIONAL_PARAM_31  = 30,
    ADDITIONAL_PARAM_32  = 31,
    ADDITIONAL_PARAM_33  = 32,
    ADDITIONAL_PARAM_34  = 33,
    ADDITIONAL_PARAM_35  = 34,
    ADDITIONAL_PARAM_36  = 35,
    ADDITIONAL_PARAM_37  = 36,
    ADDITIONAL_PARAM_38  = 37,
    ADDITIONAL_PARAM_39  = 38,
    ADDITIONAL_PARAM_40  = 39,
    ADDITIONAL_PARAM_41  = 40,
    USE_SL               = 41,
    USE_TP               = 42,
    RUN_EVERY_TICK       = 43,
    INSTANCE_MANAGEMENT  = 44,
    MAX_OPEN_ORDERS      = 45,
    IS_BACKTESTING       = 46,
    DISABLE_COMPOUNDING  = 47,
    TIMED_EXIT_BARS      = 48,
    ORIGINAL_EQUITY      = 49,
    OPERATIONAL_MODE     = 50,
    STRATEGY_INSTANCE_ID = 51,
    INTERNAL_STRATEGY_ID = 52,
    TIMEFRAME            = 53,
    SAVE_TICK_DATA       = 54,
    ANALYSIS_WINDOW_SIZE = 55,
    PARAMETER_SET_POOL   = 56,
    ACCOUNT_RISK_PERCENT = 57,
    MAX_DRAWDOWN_PERCENT = 58,
    MAX_SPREAD           = 59,
    SL_ATR_MULTIPLIER    = 60,
    TP_ATR_MULTIPLIER    = 61,
    ATR_AVERAGING_PERIOD = 62,
    ORDERINFO_ARRAY_SIZE = 63
)


MA_SHORT_PERIOD=0
SELECTED_STRATEGY_ID=0
ADDITIONAL_PARAM_1 = 0
MA_LONG_PERIOD=1
VOLATILITY_CALCULATION_MODE=1
ADDITIONAL_PARAM_2 = 1
TL_ATR_MULTIPLIER=2
ADDITIONAL_PARAM_3 = 2
USE_AFMTL=3
ADDITIONAL_PARAM_4 = 3
SELECT_KANTU_SYSTEM_FILE=4
ADDITIONAL_PARAM_5 = 4
DSL_BREAKEVEN_BARS=5
ADDITIONAL_PARAM_6 = 5
DSL_TYPE=6
ADDITIONAL_PARAM_7 = 6
DSL_EXIT_TYPE      = 7
ADDITIONAL_PARAM_8 = 7
ADDITIONAL_PARAM_9   = 8
ADDITIONAL_PARAM_10  = 9
ADDITIONAL_PARAM_11  = 10
ADDITIONAL_PARAM_12  = 11
ADDITIONAL_PARAM_13  = 12
ADDITIONAL_PARAM_14  = 13
ADDITIONAL_PARAM_15  = 14
ADDITIONAL_PARAM_16  = 15
ADDITIONAL_PARAM_17  = 16
ADDITIONAL_PARAM_18  = 17
ADDITIONAL_PARAM_19  = 18
ADDITIONAL_PARAM_20  = 19
ADDITIONAL_PARAM_21  = 20
ADDITIONAL_PARAM_22  = 21
ADDITIONAL_PARAM_23  = 22
ADDITIONAL_PARAM_24  = 23
ADDITIONAL_PARAM_25  = 24
ADDITIONAL_PARAM_26  = 25
ADDITIONAL_PARAM_27  = 26
ADDITIONAL_PARAM_28  = 27
ADDITIONAL_PARAM_29  = 28
ADDITIONAL_PARAM_30  = 29
ADDITIONAL_PARAM_31  = 30
ADDITIONAL_PARAM_32  = 31
ADDITIONAL_PARAM_33  = 32
ADDITIONAL_PARAM_34  = 33
ADDITIONAL_PARAM_35  = 34
ADDITIONAL_PARAM_36  = 35
ADDITIONAL_PARAM_37  = 36
ADDITIONAL_PARAM_38  = 37
ADDITIONAL_PARAM_39  = 38
ADDITIONAL_PARAM_40  = 39
ADDITIONAL_PARAM_41  = 40
USE_SL               = 41
USE_TP               = 42
RUN_EVERY_TICK       = 43
INSTANCE_MANAGEMENT  = 44
MAX_OPEN_ORDERS      = 45
IS_BACKTESTING       = 46
DISABLE_COMPOUNDING  = 47
TIMED_EXIT_BARS      = 48
ORIGINAL_EQUITY      = 49
OPERATIONAL_MODE     = 50
STRATEGY_INSTANCE_ID = 51
INTERNAL_STRATEGY_ID = 52
TIMEFRAME            = 53
SAVE_TICK_DATA       = 54
ANALYSIS_WINDOW_SIZE = 55
PARAMETER_SET_POOL   = 56
ACCOUNT_RISK_PERCENT = 57
MAX_DRAWDOWN_PERCENT = 58
MAX_SPREAD           = 59
SL_ATR_MULTIPLIER    = 60
TP_ATR_MULTIPLIER    = 61
ATR_AVERAGING_PERIOD = 62
ORDERINFO_ARRAY_SIZE = 63

global paramNames
paramNames = {
    7:  "ADDITIONAL_PARAM_8",
    43: "RUN_EVERY_TICK",
    44: "INSTANCE_MANAGEMENT",
    45: "MAX_OPEN_ORDERS",
    46: "IS_BACKTESTING",
    47: "DISABLE_COMPOUNDING",
    48: "TIMED_EXIT_BARS",
    49: "ORIGINAL_EQUITY",
    50: "OPERATIONAL_MODE",
    51: "INTERNAL_STRATEGY_ID",
    52: "STRATEGY_INSTANCE_ID",
    53: "TIMEFRAME",
    54: "SAVE_TICK_DATA",
    55: "ANALYSIS_WINDOW_SIZE",
    56: "PARAMETER_SET_POOL",
    57: "ACCOUNT_RISK_PERCENT",
    58: "MAX_DRAWDOWN_PERCENT",
    59: "MAX_SPREAD",
    60: "SL_ATR_MULTIPLIER",
    61: "TP_ATR_MULTIPLIER",
    62: "ATR_AVERAGING_PERIOD",
    63: "ORDERINFO_ARRAY_SIZE"
}

#Defines for Account Info
accountInfoType = c_double * 10
IDX_ACCOUNT_NUMBER                = 0
IDX_BALANCE                       = 1
IDX_EQUITY                        = 2
IDX_MARGIN                        = 3
IDX_LEVERAGE                      = 4
IDX_CONTRACT_SIZE                 = 5
IDX_MINIMUM_STOP                  = 6
IDX_STOPOUT_PERCENT               = 7
IDX_TOTAL_OPEN_TRADE_RISK_PERCENT = 8
IDX_LARGEST_DRAWDOWN_PERCENT      = 9

#Defines for Optimization
class OptimizationParam(Structure):
    _fields_ = [
        ("index", c_int),
        ("start", c_double),
        ("step", c_double),
        ("stop", c_double)
    ]

class GeneticOptimizationSettings(Structure):
    _fields_ = [
        ("population", c_int),
        ("crossoverProbability", c_double),
        ("mutationProbability", c_double),
        ("migrationProbability", c_double),
        ("evolutionaryMode", c_int),
        ("elitismMode", c_int),
        ("mutationMode", c_int),
        ("crossoverMode", c_int),
        ("maxGenerations", c_int),
        ("stopIfConverged", c_int),
        ("discardAssymetricSets", c_int),
        ("minTradesAYear", c_int),
        ("optimizationGoal", c_int)
    ]

OPTI_BRUTE_FORCE = 0
OPTI_GENETIC = 1

class FakeSecHead(object):
    def __init__(self, fp):
        self.fp = fp
        self.sechead = '[main]\n'
    def readline(self):
        if self.sechead:
            try: return self.sechead
            finally: self.sechead = None
        else:
            line = self.fp.readline()
        return re.sub('^SECTION_3.*$', '[additional]', line)
