#!/usr/bin/env python

import platform, jpype
from time import sleep

system = platform.system()

if (system == "Windows"):
    jpype.startJVM(jpype.getDefaultJVMPath(),
               '-Djava.class.path=./config;' \
               './include/asirikuy.jar;' \
               './vendor/DDS2-jClient-JForex-2.30.jar;' \
               './vendor/DDS2-Charts-6.9.jar;' \
               './vendor/patterns-1.24.jar;' \
               './vendor/greed-common-300.jar;' \
               './vendor/MQL4Connector-2.9.5.1.jar;' \
               './vendor/MQL4Converter-2.30.jar;' \
               './vendor/commons-lang3-3.0.1.jar;' \
               './vendor/jna-3.5.0.jar;' \
               './vendor/transport-client-2.5.2.jar;' \
               './vendor/transport-common-0.3.0.jar;' \
               './vendor/dds2-common-2.5.2.jar;' \
               './vendor/mina-core-1.1.7.jar;' \
               './vendor/mina-filter-ssl-1.1.7dc.1.jar;' \
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
               './vendor/JForex-API-2.9.5.1.jar;' \
               './vendor/JForex-API-2.9.5.1-sources.jar;' \
               './vendor/slf4j-log4j12-1.6.4.jar;' \
               './vendor/slf4j-api-1.6.4.jar;' \
               './vendor/log4j-1.2.16.jar;' \
               )
else:
    jpype.startJVM(jpype.getDefaultJVMPath(),
               '-Djava.awt.headless=true',
               '-Djava.class.path=./config:' \
               './include/asirikuy.jar:' \
               './vendor/DDS2-jClient-JForex-2.30.jar:' \
               './vendor/DDS2-Charts-6.9.jar:' \
               './vendor/patterns-1.24.jar:' \
               './vendor/greed-common-300.jar:' \
               './vendor/MQL4Connector-2.9.5.1.jar:' \
               './vendor/MQL4Converter-2.30.jar:' \
               './vendor/commons-lang3-3.0.1.jar:' \
               './vendor/jna-3.5.0.jar:' \
               './vendor/transport-client-2.5.2.jar:' \
               './vendor/transport-common-0.3.0.jar:' \
               './vendor/dds2-common-2.5.2.jar:' \
               './vendor/mina-core-1.1.7.jar:' \
               './vendor/mina-filter-ssl-1.1.7dc.1.jar:' \
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
               './vendor/JForex-API-2.9.5.1.jar:' \
               './vendor/JForex-API-2.9.5.1-sources.jar:' \
               './vendor/slf4j-log4j12-1.6.4.jar:' \
               './vendor/slf4j-api-1.6.4.jar:' \
               './vendor/log4j-1.2.16.jar' \
               )

ClientFactory = jpype.JPackage('com').dukascopy.api.system.ClientFactory
client = ClientFactory.getDefaultInstance()
client.connect('https://www.dukascopy.com/client/demo/jclient/jforex.jnlp', 'DEMO2gxLfW', 'gxLfW');

for i in range(10):
    if not client.isConnected():
        sleep(1)
if not client.isConnected():
    print "Failed to connect"
    quit()

HashSet = jpype.JClass('java.util.HashSet')
instruments = HashSet();
instruments.add(jpype.JPackage('com').dukascopy.api.Instrument.EURUSD)
client.setSubscribedInstruments(instruments);

class MyStrategy:
    def onTick(self, instrument, tick):
        print "Tick received"
        print tick.getAsk()
    def onAccount(self, account):
        print "onAccount"
    def onBar(self, instrument, period, ask_bar, bid_bar):
        print "onBar"
    def onMessage(self, message):
        print "onMessage"
    def onStart(self, context):
        print "Connected to the tick streaming"
        print "Suscribed to instruments:", context.getSubscribedInstruments()
    def onStop(self):
        print "Disconnected from the tick streaming"

my_strategy = MyStrategy()
proxy = jpype.JProxy("com.dukascopy.api.IStrategy", inst = my_strategy)
client.startStrategy(proxy)

while True:
    sleep(5)

#jpype.shutdownJVM()