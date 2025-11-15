from ctypes import *
from time import *
import re, os, ctypes,  csv,  calendar, datetime, ConfigParser, sys
import smtplib

def sendemail(from_addr, to_addr_list, cc_addr_list,
              subject, message,
              login, password,
              smtpserver):
        
    header  = 'From: %s\n' % from_addr
    header += 'To: %s\n' % ','.join(to_addr_list)
    header += 'Cc: %s\n' % ','.join(cc_addr_list)
    header += 'Subject: %s\n\n' % subject
    message = header + message
  
    server = smtplib.SMTP(smtpserver)
    server.starttls()
    server.login(login,password)
    problems = server.sendmail(from_addr, to_addr_list, message)
    server.quit()

global DukascopySymbol 
DukascopySymbol = {
					'EURUSD':'EUR/USD',
					'GBPUSD':'GBP/USD',
					'USDJPY':'USD/JPY',
					'AUDUSD':'AUD/USD',
					'USDCHF':'USD/CHF',
					'USDCAD':'USD/CAD'
					}
global DukascopySymbolReverse
DukascopySymbolReverse = {
					'EUR/USD':'EURUSD',
					'GBP/USD':'GBPUSD',
					'USD/JPY':'AUDUSD',
					'USD/CHF':'USDCHF',
					'USD/CAD':'USDCAD'
					}

global OandaSymbol 
OandaSymbol = {
				'EURUSD':'EUR_USD',
				'GBPUSD':'GBP_USD',
				'USDJPY':'USD_JPY',
				'AUDUSD':'AUD_USD',
				'USDCHF':'USD_CHF',
				'USDCAD':'USD_CAD'
				}

global OandaSymbolReverse 
OandaSymbolReverse  = {
				'EUR_USD':'EURUSD',
				'GBP_USD':'GBPUSD',
				'USD_JPY':'USDJPY',
				'AUD_USD':'AUDUSD',
				'USD_CHF':'USDCHF',
				'USD_CAD':'USDCAD'
				}

def loadLibrary(library):
	if os.name == 'nt':
		return windll.LoadLibrary(library)
	else:
		return None
def readConfigFile(file):
	try:
		config = ConfigParser.RawConfigParser()
		config.read(file)
		return config
	except:
		raise ValueError(sys.exc_info()[1])
		return False

def getASKFrameworkVersion(asfdll):
	pMajor = c_int()
	pMinor = c_int()
	pbugFix = c_int()
	asfdll.getFrameworkVersion.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
	asfdll.getFrameworkVersion(byref(pMajor), byref(pMinor), byref(pbugFix))
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
        ("finalBalance", c_double),
        ("maxDDDepth", c_double),
        ("maxDDLength", c_double),
        ("pf", c_double),
        ("r2", c_double),
        ("ulcerIndex", c_double),
        ("numShorts", c_int),
        ("numLongs", c_int),
        ("yearsTraded", c_double),
        ("symbol", c_char*256)
    ]

global barsNeeded
barsNeeded = (
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [300,100,0,0,0,0,0,0,0,0],
        [50,100,0,0,0,0,0,0,0,0],
        [1200,100,0,0,0,0,0,0,0,0]
        )
        

global ratesNeeded
ratesNeeded = (
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"],
 ["N","N","N","N","N","N","N","N"]
)

global goalsDesc
goalsDesc = [
	'Balance',
	'Max DD',
	'Max DD Length',
	'Profit Factor',
	'R2',
	'Ulcer Index'
]
	
class TestSettings(Structure):
	_fields_ = [
		("spread", c_double),
		("fromDate", c_int),
		("toDate", c_int)
	]

class Rate(Structure):
	_fields_ = [
		("open", c_double),
		("high", c_double),
		("low", c_double),
		("close", c_double),
		("volume", c_double),
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
'WATUKUSHAY_FE_BB',
'WATUKUSHAY_FE_CCI',
'ATIPAQ',
'AYOTL',
'COATL',
'COMITL_BB',
'COMITL_KC',
'COMITL_PA',
'GODS_GIFT_ATR',
'QALLARYI',
'QUIMICHI',
'SAPAQ',
'SUNQU',
'TEYACANANI',
'WATUKUSHAY_FE_RSI',
'RUPHAY',   
'TEST_EA',  
'KUYUY',    
'PAQARIN', 
'TAPUY', 
'KANTU',
'RECORD_BARS',
'MUNAY'	
]

opType = [
'BUY',
'SELL'
]

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
ADDITIONAL_PARAM_1   = 0, 
ADDITIONAL_PARAM_2   = 1, 
ADDITIONAL_PARAM_3   = 2, 
ADDITIONAL_PARAM_4   = 3, 
ADDITIONAL_PARAM_5   = 4, 
ADDITIONAL_PARAM_6   = 5, 
ADDITIONAL_PARAM_7   = 6, 
ADDITIONAL_PARAM_8   = 7, 
ADDITIONAL_PARAM_9   = 8, 
ADDITIONAL_PARAM_10  = 9, 
ADDITIONAL_PARAM_11  = 10, 
ADDITIONAL_PARAM_12  = 11, 
ADDITIONAL_PARAM_13  = 12, 
ADDITIONAL_PARAM_14  = 13, 
ADDITIONAL_PARAM_15  = 14, 
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
WFO_PSET_MANAGEMENT  = 54, 
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

ADDITIONAL_PARAM_1   = 0
ADDITIONAL_PARAM_2   = 1
ADDITIONAL_PARAM_3   = 2
ADDITIONAL_PARAM_4   = 3
ADDITIONAL_PARAM_5   = 4
ADDITIONAL_PARAM_6   = 5
ADDITIONAL_PARAM_7   = 6
ADDITIONAL_PARAM_8   = 7
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
WFO_PSET_MANAGEMENT  = 54
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
54: "WFO_PSET_MANAGEMENT",
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

#Define for results
resultsType = c_double * 10
IDX_TICKET_NUMBER           = 0
IDX_TRADING_SIGNALS         = 1
IDX_LOTS                    = 2
IDX_ENTRY_PRICE             = 3
IDX_BROKER_SL               = 4
IDX_BROKER_TP               = 5
IDX_INTERNAL_SL             = 6
IDX_INTERNAL_TP             = 7
IDX_USE_TRAILING            = 8
IDX_EXPIRATION_TIME         = 9
RESULTS_ARRAY_SIZE          = 10

# BidAsk array indexes
BidAskType = c_double * 6
IDX_BID                      = 0
IDX_ASK                      = 1
IDX_BASE_CONVERSION_BID      = 2
IDX_BASE_CONVERSION_ASK      = 3
IDX_QUOTE_CONVERSION_BID     = 4
IDX_QUOTE_CONVERSION_ASK     = 5
BID_ASK_ARRAY_SIZE           = 6

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

#order signal masks
SIGNAL_NONE             = 0x00000000
SIGNAL_OPEN_BUY         = 0x00000001
SIGNAL_CLOSE_BUY        = 0x00000002
SIGNAL_UPDATE_BUY       = 0x00000004
SIGNAL_OPEN_BUYLIMIT    = 0x00000008
SIGNAL_CLOSE_BUYLIMIT   = 0x00000010
SIGNAL_UPDATE_BUYLIMIT  = 0x00000020
SIGNAL_OPEN_BUYSTOP     = 0x00000040
SIGNAL_CLOSE_BUYSTOP    = 0x00000080
SIGNAL_UPDATE_BUYSTOP   = 0x00000100
SIGNAL_OPEN_SELL        = 0x00010000
SIGNAL_CLOSE_SELL       = 0x00020000
SIGNAL_UPDATE_SELL      = 0x00040000
SIGNAL_OPEN_SELLLIMIT   = 0x00080000
SIGNAL_CLOSE_SELLLIMIT  = 0x00100000
SIGNAL_UPDATE_SELLLIMIT = 0x00200000
SIGNAL_OPEN_SELLSTOP    = 0x00400000
SIGNAL_CLOSE_SELLSTOP   = 0x00800000
SIGNAL_UPDATE_SELLSTOP  = 0x01000000

#position defines
BUY     = 0
SELL    = 1

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
