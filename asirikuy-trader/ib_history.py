from time import sleep, strftime, localtime
from ib.ext.Contract import Contract
from ib.opt import ibConnection, message

new_symbolinput = ['SPY','GLD','SLV']
new_symbolinput = ['EUR']
newDataList = []
dataDownload = []

def makeStkContract(contractTuple):
    newContract = Contract()
    newContract.m_symbol = contractTuple[0]
    newContract.m_secType = contractTuple[1]
    newContract.m_exchange = contractTuple[2]
    newContract.m_currency = contractTuple[3]
    newContract.m_expiry = contractTuple[4]
    newContract.m_strike = contractTuple[5]
    newContract.m_right = contractTuple[6]
    print 'Contract Values:%s,%s,%s,%s,%s,%s,%s:' % contractTuple
    return newContract

def historical_data_handler(msg):
   global newDataList
   print msg
   print msg.reqId, msg.date, msg.open, msg.high, msg.low, msg.close, msg.volume
   if ('finished' in str(msg.date)) == False:
     new_symbol = new_symbolinput[msg.reqId]
     dataStr = '%s, %s, %s, %s, %s, %s, %s' % (new_symbol, strftime("%Y-%m-%d %H:%M:%S", localtime(int(msg.date))), msg.open, msg.high, msg.low, msg.close, msg.volume)
     newDataList = newDataList + [dataStr]
   else:
     new_symbol = new_symbolinput[msg.reqId]
     filename = 'minutetrades' + new_symbol + '.csv'
     csvfile = open('/tmp/' + filename,'wb')
     for item in newDataList:
       csvfile.write('%s \n' % item)
     csvfile.close()
     newDataList = []
     global dataDownload
     dataDownload.append(new_symbol)

con = ibConnection('localhost', 4001, 0)
con.register(historical_data_handler, message.historicalData)
con.connect()

symbol_id = 0

contractTuple = ('EUR', 'CASH', 'IDEAL', 'USD', '', 0.0, '')
stkContract = makeStkContract(contractTuple)
con.reqHistoricalData(symbol_id, stkContract, '', '30 D', '1 hour', 'TRADES', 2, 1)

sleep(10)

print dataDownload
