# -*- coding: utf-8 -*-

"""
Module implementing MainWindow.
"""

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from PyQt4 import QtGui,  QtCore, Qt

from .Ui_mainwindow import Ui_MainWindow

from ctypes import *
from time import *
import datetime, ctypes, os, platform, configparser, math, csv, calendar, argparse, sys,  subprocess,  shutil
sys.path.insert(0, os.path.join(os.getcwd(), "include"))
#sys.path.insert(0, 'E:\\FX\\Asirikuy\\asirikuy_python_tester_v56_new\\asirikuy_python_tester_v56/include')
from include.asirikuy import *
from include.graphics import *
from include.misc import *
from include.mt import *
import xml.etree.ElementTree as ET
import decimal

class MainWindow(QMainWindow, Ui_MainWindow):
    """
    Class documentation goes here.
    """
    def __init__(self, parent = None):
        """
        Constructor
        """
        QMainWindow.__init__(self, parent)
        self.setupUi(self)
        #Config UI
        self.editBalance.setInputMask('00000000')
        self.editLeverage.setInputMask('000')
        #self.editContractSize.setInputMask('000000')
        self.editStopOut.setInputMask('00')
        self.editMinLotSize.setInputMask('0.00000')
        self.editMinTrades.setInputMask('000')
        self.editPopulation.setInputMask('000')
        self.editMaxGenerations.setInputMask('00000')
        self.editCrossoverProbability.setInputMask('0.00')
        self.editMutationProbability.setInputMask('0.00')
        self.editMigrationProbability.setInputMask('0.00')
        self.labelInfo.setText('')
        self.running = False

        #print Version
        clearScreen();
        print("New Strategy Tester (NST) " + VERSION)
        
        for strategy in strategies:
            self.comboStrategy.addItem(strategy)
        
        self.tableOptimization.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.connect(self.tableOptimization, QtCore.SIGNAL('customContextMenuRequested(const QPoint&)'), self.on_context_menu)
        # create context menu
        self.popMenu = QtGui.QMenu(self)
        action = QtGui.QAction('Run test for this result', self)
        self.connect(action, SIGNAL('triggered()'), self.runTestFromOptimizationResult)
        self.popMenu.addAction(action)
        action = QtGui.QAction('Save result as a set file', self)
        self.connect(action, SIGNAL('triggered()'), self.saveTestFromOptimizationResult)
        self.popMenu.addAction(action)
        #self.popMenu.addSeparator()  
        
        #Paths
        self.asirikuyFrameworkPath = os.path.join(os.getcwd(), 'config' ,'AsirikuyConfig.xml')
        self.asirikuyCtesterLogPath = os.path.join(os.getcwd(), 'log', 'AsirikuyCTester.log')
        self.configFilePath = os.path.join(os.getcwd(), 'config', 'vast.config')
        self.setFilePath = ''
        self.tempSetFilePath = os.path.join(os.getcwd(), 'tmp', 'tmp.set')
        self.baseSetFilePath = os.path.join(os.getcwd(), 'tmp', 'base.set')
        if os.path.exists(self.setFilePath): shutil.copyfile (self.setFilePath,  self.baseSetFilePath)
        self.testResultsFilePath = os.path.join(os.getcwd(), 'results.txt')
        self.optimizationResultsFilePath = os.path.join(os.getcwd(), 'optimization.txt')
        self.outputXMLPath = os.path.join(os.getcwd(), 'results.xml')
        
        #Account Info
        self.brokerName		= "Pepperstone Group Limited"
        self.refBrokerName 	= "Pepperstone Group Limited"
        self.accountInfo = accountInfoType()
        #Rates
        self.ratesInfo = RatesInfoType()
        #Other
        self.testSettings = TestSettings()
        self.settings = SettingsType()
        self.logSeverity = 5
        self.optimizationArray = {}
        self.params = []
        self.optimizationSettings = GeneticOptimizationSettings()
        
        
        
        #Load DLLs and get framework versions
        system = platform.system()
        if (system == "Windows"):
            self.asfdll = loadLibrary('AsirikuyFrameworkAPI')
            self.astdll = loadLibrary('CTesterFrameworkAPI')
        elif (system == "Linux"):
            self.asfdll = loadLibrary('libAsirikuyFrameworkAPI.so')
            self.astdll = loadLibrary('libCTesterFrameworkAPI.so')
        elif (system == "Darwin"):
            self.asfdll = loadLibrary('libAsirikuyFrameworkAPI.dylib')
            self.astdll = loadLibrary('libCTesterFrameworkAPI.dylib')
        else:
            print("No shared library loading support for OS %s" % (system))
            return False
        self.labelVersions.setText ('ASKF v' +  getASKFrameworkVersion(self.asfdll) + '  CTester v' + getASTFrameworkVersion(self.astdll))

        #Load config files
        self.loadConfigFile()
    
    def on_context_menu(self, point):
        # show context menu
        if not self.running and self.tableOptimization.selectedIndexes():
            self.popMenu.exec_(self.tableOptimization.mapToGlobal(point)) 
    
    def generateTempSet(self):
        selectedRow = self.tableOptimization.selectedIndexes()[0].row()
        self.checkOptimize.setCheckState(0)
        params = str(self.tableOptimization.item(selectedRow, 10).text()).split(' ')
        with open(self.tempSetFilePath, "w") as out:
            for line in open(self.baseSetFilePath):
                for param in params:
                    paramName,  paramValue = param.split('=')
                    if line.find(paramName + '=') > -1: line = param + '\n'
                out.write(line)
        out.close
                
    def runTestFromOptimizationResult(self):
        self.generateTempSet()
        self.setFilePath = self.tempSetFilePath
        self.on_buttonStartTest_released()
    
    def saveTestFromOptimizationResult(self):
        self.generateTempSet()
        destination = QtGui.QFileDialog.getSaveFileName(self, "Save file", "", ".set")
        shutil.copyfile (self.tempSetFilePath,  destination)
        
    def closeEvent(self, event):
        self.saveConfigFile()
        event.accept()
    
    def loadGraphic(self, type):
        scene = QtGui.QGraphicsScene()
        if type == 0: #Test Result
            #if os.path.exists(os.path.join(os.getcwd(),'results.png')): os.remove(os.path.join(os.getcwd(),'results.png'))
            plotTestResult(self.testResultsFilePath, singleSystemRegressionType, resolution = {'x': 745, 'y': 517})
            if os.path.exists(os.path.join(os.getcwd(),'results.png')):
                scene.addPixmap(QtGui.QPixmap(os.path.join(os.getcwd(),'results.png')))
        elif type == 1: #Optimizationresult
            #if os.path.exists(os.path.join(os.getcwd(),'optimization.png')): os.remove(os.path.join(os.getcwd(),'results.png'))
            plotOptimizationResult(self.optimizationResultsFilePath,  optimizationGoal = self.comboGoal.currentIndex(),  resolution = {'x': 745, 'y': 517})
            if os.path.exists(os.path.join(os.getcwd(),'optimization.png')):
                scene.addPixmap(QtGui.QPixmap(os.path.join(os.getcwd(),'optimization.png')))
        elif type == 2: #Portfolio result
            #if os.path.exists(os.path.join(os.getcwd(),'results.png')): os.remove(os.path.join(os.getcwd(),'results.png'))
            systems = []
            for i in range(self.numSystemsInPortfolio):
                systems.append(str(self.comboStrategy.itemText(self.strategyIDs[i])) + ' - ' + self.symbols[i])
            plotPortfolioTestResult(self.testResultsFilePath, resolution = {'x': 745, 'y': 517}, systemNames = systems)
            if os.path.exists(os.path.join(os.getcwd(),'results.png')): 
                scene.addPixmap(QtGui.QPixmap(os.path.join(os.getcwd(),'results.png')))
        elif type == 3: #Portfolio result
            #if os.path.exists(os.path.join(os.getcwd(),'results.png')): os.remove(os.path.join(os.getcwd(),'results.png'))
            plotMultipleTestResults(self.symbols,  resolution = {'x': 745, 'y': 517})
            if os.path.exists(os.path.join(os.getcwd(),'multipleResults.png')):
                scene.addPixmap(QtGui.QPixmap(os.path.join(os.getcwd(),'multipleResults.png')))
        self.graphicsView.setScene(scene)
        QCoreApplication.processEvents()
        
    def loadConfigFile(self):
        if not os.path.isfile(self.configFilePath):
            return
        try:
            config = readConfigFile(self.configFilePath)
            self.logSeverity = config.getint('misc',  'logSeverity')
            
            for i in range(self.comboAccountCurrency.count()):
                if self.comboAccountCurrency.itemText(i) == config.get('account', 'currency'):
                    self.comboAccountCurrency.setCurrentIndex(i)
            self.editBalance.setText(config.get('account', 'balance'))
            self.editLeverage.setText(config.get('account', 'leverage'))
            self.editContractSize.setText(config.get('account', 'contractSize'))
            self.editDigits.setText(config.get('account', 'digits'))
            self.editStopOut.setText(config.get('account', 'stopOutPercent'))
            self.editMinimumStop.setText(config.get('account', 'minimumStop'))
            self.editSpread.setText(config.get('account', 'spread'))
            self.editMinLotSize.setText(config.get('account', 'minLotSize'))
            
            self.editPair.setText(config.get('strategy', 'pair'))
            self.editDataTF.setText(config.get('strategy', 'passedTimeFrame'))
            self.comboStrategy.setCurrentIndex(config.getint('strategy', 'strategyId'))
            self.editPortfolioStrategies.setText(config.get('strategy', 'portfolioStrategies'))
            self.editSet.setText(config.get('strategy', 'set'))
            self.editRateRequirements.setText(config.get('strategy', 'rateRequirements'))
            self.editSymbolRequirements.setText(config.get('strategy', 'symbolRequirements'))
            self.editTimeFrameRequirements.setText(config.get('strategy', 'timeframeRequirements'))
            
            self.dateFrom.setDate(QtCore.QDate.fromString(config.get('strategy', 'fromDate'), "yyyy-MM-dd"))
            self.dateTo.setDate(QtCore.QDate.fromString(config.get('strategy', 'toDate'), "yyyy-MM-dd"))
            
            if config.getint('optimization', 'optimize') == 1:
                self.checkOptimize.setCheckState(QtCore.Qt.Checked)
            else:
                self.checkOptimize.setCheckState(QtCore.Qt.Unchecked)
            self.comboOptimizationType.setCurrentIndex(config.getint('optimization', 'optimizationType'))
            self.spinNumCores.setValue(config.getint('optimization', 'numCores'))
            self.comboGoal.setCurrentIndex(config.getint("optimization", "optimizationGoal"))
            
            self.editPopulation.setText(config.get('optimization', 'population'))
            self.editMaxGenerations.setText(config.get('optimization', 'maxGenerations'))
            self.editCrossoverProbability.setText(config.get('optimization', 'crossoverProbability')) 
            self.editMutationProbability.setText(config.get('optimization', 'mutationProbability')) 
            self.editMigrationProbability.setText(config.get('optimization', 'migrationProbability')) 
            self.comboEvolutionaryMode.setCurrentIndex(config.getint('optimization', 'evolutionaryMode')) 
            self.comboElitismMode.setCurrentIndex(config.getint('optimization', 'elitismMode'))  
            self.comboMutationMode.setCurrentIndex(config.getint('optimization', 'mutationMode')) 
            self.comboCrossoverMode.setCurrentIndex(config.getint('optimization', 'crossoverMode')) 
            
        except NameError as e:
            print(e)
        except AttributeError as e:
            print(e)
        except TypeError as e:
            print(e)
        except configparser.NoOptionError as e:
            print(e)
        except:
            print("Error loading parameteres from config file:", sys.exc_info()[0])
        
    def saveConfigFile(self):
        config = configparser.RawConfigParser()
        
        config.add_section('misc')
        config.set('misc', 'logSeverity', str(self.logSeverity))
        
        config.add_section('account')
        config.set('account', 'currency', self.comboAccountCurrency.currentText())
        config.set('account', 'balance', self.editBalance.text())
        config.set('account', 'leverage', self.editLeverage.text())
        config.set('account', 'contractSize', self.editContractSize.text())
        config.set('account', 'digits', self.editDigits.text())
        config.set('account', 'stopOutPercent', self.editStopOut.text())
        config.set('account', 'minimumStop', self.editMinimumStop.text())
        config.set('account', 'spread', self.editSpread.text())
        config.set('account', 'minLotSize',  self.editMinLotSize.text())
        
        config.add_section('strategy')
        config.set('strategy', 'pair', self.editPair.text())
        config.set('strategy', 'passedTimeFrame', self.editDataTF.text())
        config.set('strategy', 'strategyID', self.comboStrategy.currentIndex())
        config.set('strategy', 'fromDate', str(self.dateFrom.date().toPyDate()))
        config.set('strategy', 'toDate', str(self.dateTo.date().toPyDate()))
        config.set('strategy', 'portfolioStrategies', self.editPortfolioStrategies.text())
        config.set('strategy', 'set', self.editSet.text())
        config.set('strategy', 'rateRequirements', self.editRateRequirements.text())
        config.set('strategy', 'symbolRequirements', self.editSymbolRequirements.text())
        config.set('strategy', 'timeframeRequirements', self.editTimeFrameRequirements.text())
        
        config.add_section('optimization')
        if self.checkOptimize.isChecked():
            config.set('optimization', 'optimize', '1')
        else:
            config.set('optimization', 'optimize', '0')
        config.set('optimization', 'optimizationType', self.comboOptimizationType.currentIndex())
        config.set('optimization', 'numCores', str(self.spinNumCores.value()))
        config.set('optimization', 'optimizationGoal', str(self.comboGoal.currentIndex())) 
        
        config.set('optimization', 'population', self.editPopulation.text())
        config.set('optimization', 'maxGenerations', self.editMaxGenerations.text())
        config.set('optimization', 'crossoverProbability', self.editCrossoverProbability.text())
        config.set('optimization', 'mutationProbability', self.editMutationProbability.text())
        config.set('optimization', 'migrationProbability', self.editMigrationProbability.text())
        config.set('optimization', 'evolutionaryMode', self.comboEvolutionaryMode.currentIndex())
        config.set('optimization', 'elitismMode', self.comboElitismMode.currentIndex())
        config.set('optimization', 'mutationMode', self.comboMutationMode.currentIndex())
        config.set('optimization', 'crossoverMode', self.comboCrossoverMode.currentIndex())
        
        with open(self.configFilePath, 'wb') as configfile:
            config.write(configfile)
    
    @pyqtSignature("")
    def on_buttonStartTest_released(self):
        """
        Slot documentation goes here.
        """
           
        #Get portfolio data
        setFilePaths = []
        self.strategyIDs = []
        spreads = []
        digits = []
        passedTimeFrame = []
        # historyFilePaths = []
        rateRequirements = []
        symbolRequirements = []
        timeframeRequirements = []
        minimumStops = []
        contractSize = []
        
        for s in self.editSet.text().split(','): setFilePaths.append(s)
        for s in self.editSpread.text().split(','): spreads.append(float(s))
        for s in self.editDigits.text().split(','): digits.append(int(s))
        for s in self.editContractSize.text().split(','): contractSize.append(float(s))
        for s in self.editDataTF.text().split(','): passedTimeFrame.append(int(s))
        for s in self.editRateRequirements.text().split('|'): rateRequirements.append(s)
        for s in self.editSymbolRequirements.text().split('|'): symbolRequirements.append(s)
        for s in self.editTimeFrameRequirements.text().split('|'): timeframeRequirements.append(s)
        for s in self.editMinimumStop.text().split(','): minimumStops.append(s)
        
        symbolsArrayType = c_char_p * len(str(self.editPair.text()).split(','))
        self.symbols = symbolsArrayType()
        for index, s in enumerate(str(self.editPair.text()).split(',')): 
            self.symbols[index] = s
        #    historyFilePaths.append(os.ph
        #                            +ath.join(os.getcwd() ,  'history' ,  s + '_60.csv'))
        
        if str(self.editPortfolioStrategies.text()).strip() == '':
            self.strategyIDs.append(int(self.comboStrategy.currentIndex()))
        else:
            for s in str(self.editPortfolioStrategies.text()).split(','): self.strategyIDs.append(int(s))
        
        if len(setFilePaths) > 1:
            if len(setFilePaths) != len(digits) or len(setFilePaths) != len(self.strategyIDs) or len(self.symbols) != len(spreads) or len(self.symbols) != len(minimumStops):
                QtGui.QMessageBox.information(self, 'Error', "You must have the same number of items for minimum Stops, setFile, pair, spread, digits, and strategyID")
                return False
        else:
            if len(self.symbols) != len(digits) or len(self.symbols) != len(spreads) or len(spreads) != len(digits) != len(minimumStops):
                QtGui.QMessageBox.information(self, 'Error', "For multi-instrument tests the number of minimum stops, symbols, digits and spreads must be equal.")
                return False
        
        self.numSystemsInPortfolio = len(setFilePaths)
        self.numPairs = len(self.symbols)
        
        global barsNeeded
        
        barsNeeded = [None] * 200
        ratesNeededSymbol = [None] * 200
        ratesNeededTF = [None] * 200
        
        for i in range(0, self.numSystemsInPortfolio):
 
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
                              
            barsNeeded[self.strategyIDs[i]] = rateRequirementItems
            ratesNeededSymbol[self.strategyIDs[i]] = symbolRequirementItems
            ratesNeededTF[self.strategyIDs[i]] = timeframeRequirementItems

        self.barsNeeded = barsNeeded
        
        for i in range(self.numSystemsInPortfolio):
#            if not os.path.isfile(historyFilePaths[i]):
#                QtGui.QMessageBox.information(self, 'Error', "Can't find history file: " + historyFilePaths[i])
#                return False
            if setFilePaths[i] == '':
                QtGui.QMessageBox.information(self, 'Error', "Load a set file before starting the test")
                return False
            if not os.path.isfile(setFilePaths[i]):
                QtGui.QMessageBox.information(self, 'Error', "Can't find set file: " + setFilePaths[i])
                return False
        
        #Read set files
        sets = []
        for index, s in enumerate(setFilePaths):
            sets.append(MT4Set(s))
            if not sets[index].content:
                print("Error reading set file %s" % (s))
                return False
        
        self.tableTest.setSortingEnabled(False)
        self.tableOptimization.setSortingEnabled(False)
        
        self.running = True
        
        #Optimization 
        self.optimize = 1 if self.checkOptimize.checkState() else 0
        self.optimizationType = self.comboOptimizationType.currentIndex()
        self.numCores = self.spinNumCores.value()
        
        
        #Settings values
        optimizationParams = []
        optimizationArrays = []
        numOptimizationParams = []
        self.paramNamesArray = []
    
        TestSettingsArrayType = TestSettings * self.numSystemsInPortfolio
        testSettings = TestSettingsArrayType()
        SettingsArrayType = self.numPairs * ctypes.POINTER(c_double)
        settings = SettingsArrayType()
        for i in range(self.numSystemsInPortfolio):
            
            settings[i] = SettingsType()
            settings[i][IS_BACKTESTING]       = True
            settings[i][DISABLE_COMPOUNDING]  = float(sets[i].mainParams["DISABLE_COMPOUNDING"]['value']) if sets[i].content.has_option('main', 'DISABLE_COMPOUNDING') else 0
            settings[i][TIMED_EXIT_BARS]      = float(sets[i].mainParams["TIMED_EXIT_BARS"]['value']) if sets[i].content.has_option('main', 'TIMED_EXIT_BARS') else 0
            settings[i][ORIGINAL_EQUITY]      = float(self.editBalance.text())
            settings[i][OPERATIONAL_MODE]     = 1
            settings[i][STRATEGY_INSTANCE_ID] = float(sets[i].mainParams["STRATEGY_INSTANCE_ID"]['value']) if sets[i].content.has_option('main', 'RUN_EVERY_TICK') else 0
            settings[i][INTERNAL_STRATEGY_ID] = self.strategyIDs[i]
            settings[i][RUN_EVERY_TICK]       = float(sets[i].mainParams["RUN_EVERY_TICK"]['value']) if sets[i].content.has_option('main', 'RUN_EVERY_TICK') else 0
            settings[i][INSTANCE_MANAGEMENT]  = float(sets[i].mainParams["INSTANCE_MANAGEMENT"]['value']) if sets[i].content.has_option('main', 'INSTANCE_MANAGEMENT') else 0
            settings[i][MAX_OPEN_ORDERS]      = float(sets[i].mainParams["MAX_OPEN_ORDERS"]['value']) if sets[i].content.has_option('main', 'MAX_OPEN_ORDERS') else 1
            settings[i][TIMEFRAME]            = float(sets[i].mainParams["STRATEGY_TIMEFRAME"]['value']) if sets[i].content.has_option('main', 'STRATEGY_TIMEFRAME') else 0
            settings[i][SAVE_TICK_DATA]  = float(sets[i].mainParams["SAVE_TICK_DATA"]['value']) if sets[i].content.has_option('main', 'SAVE_TICK_DATA') else 0
            settings[i][ANALYSIS_WINDOW_SIZE] = float(sets[i].mainParams["ANALYSIS_WINDOW_SIZE"]['value']) if sets[i].content.has_option('main', 'ANALYSIS_WINDOW_SIZE') else 0
            settings[i][PARAMETER_SET_POOL]   = float(sets[i].mainParams["PARAMETER_SET_POOL"]['value']) if sets[i].content.has_option('main', 'PARAMETER_SET_POOL') else 0
            settings[i][ACCOUNT_RISK_PERCENT] = float(sets[i].mainParams["ACCOUNT_RISK_PERCENT"]['value']) if sets[i].content.has_option('main', 'ACCOUNT_RISK_PERCENT') else 0
            settings[i][MAX_DRAWDOWN_PERCENT] = float(sets[i].mainParams["MAX_DRAWDOWN_PERCENT"]['value']) if sets[i].content.has_option('main', 'MAX_DRAWDOWN_PERCENT') else 0
            settings[i][MAX_SPREAD]           = float(sets[i].mainParams["MAX_SPREAD_PIPS"]['value']) if sets[i].content.has_option('main', 'MAX_SPREAD_PIPS') else 0
            settings[i][SL_ATR_MULTIPLIER]    = float(sets[i].mainParams["SL_ATR_MULTIPLIER"]['value']) if sets[i].content.has_option('main', 'SL_ATR_MULTIPLIER') else 0
            settings[i][TP_ATR_MULTIPLIER]    = float(sets[i].mainParams["TP_ATR_MULTIPLIER"]['value']) if sets[i].content.has_option('main', 'TP_ATR_MULTIPLIER') else 0
            settings[i][ATR_AVERAGING_PERIOD] = float(sets[i].mainParams["ATR_AVERAGING_PERIOD"]['value']) if sets[i].content.has_option('main', 'ATR_AVERAGING_PERIOD') else 0
            settings[i][ORDERINFO_ARRAY_SIZE] = settings[i][MAX_OPEN_ORDERS] +1
            settings[i][ADDITIONAL_PARAM_8] = float(sets[i].additionalParams["DSL_EXIT_TYPE"]['value']) if sets[i].content.has_option('additional', 'DSL_EXIT_TYPE') else 0
            

                        
            if i == 0:
                global singleSystemRegressionType
                singleSystemRegressionType = settings[i][DISABLE_COMPOUNDING]
    
            optimizationArrays.append(sets[i].optimizationArray)
        
            self.paramNamesArray.append(paramNames)
            index=0
            for param in sets[i].content.items("additional"):
                if param[0].find(",") == -1:
                    settings[i][paramIndexes[param[0].upper()]] = float(param[1])
                    self.paramNamesArray[i][index] = param[0].upper()
                    index += 1
        
            OptimizationParamType = OptimizationParam * len(optimizationArrays[i])
            optimizationParams.append(OptimizationParamType())
            
            numOptimizationParams.append(0)
            for key, value in optimizationArrays[i].items():
                #print numOptimizationParams[i]
                #print key
                #print value
                
                optimizationParams[i][numOptimizationParams[i]].index = key
                optimizationParams[i][numOptimizationParams[i]].start = value[0]
                optimizationParams[i][numOptimizationParams[i]].step  = value[1]
                optimizationParams[i][numOptimizationParams[i]].stop  = value[2]
                
                numOptimizationParams[i] += 1
        
            testSettings[i] = TestSettings()
            testSettings[i].spread = spreads[i]
            
        
        if numOptimizationParams[0] == 0 and self.optimize:
            QtGui.QMessageBox.information(self, 'Info', 'Nothing to optimize. Check your set file')
            return True

        #Account info values
        accountInfoArrayType = self.numSystemsInPortfolio * ctypes.POINTER(c_double)
        accountInfo = accountInfoArrayType()
    
        for i in range(self.numSystemsInPortfolio):
            accountInfo[i] = accountInfoType()
            accountInfo[i][IDX_ACCOUNT_NUMBER] = 0
            accountInfo[i][IDX_BALANCE] = float(self.editBalance.text())	
            accountInfo[i][IDX_EQUITY] = accountInfo[i][IDX_BALANCE] 
            accountInfo[i][IDX_MARGIN] = 0
            #accountInfo[i][IDX_CONTRACT_SIZE] = float(self.editContractSize.text())
            accountInfo[i][IDX_CONTRACT_SIZE] = contractSize[i]            
            accountInfo[i][IDX_LEVERAGE] = float(self.editLeverage.text())
            accountInfo[i][IDX_MINIMUM_STOP] = float(minimumStops[i])
            accountInfo[i][IDX_STOPOUT_PERCENT] = float(self.editStopOut.text())
            accountInfo[i][IDX_TOTAL_OPEN_TRADE_RISK_PERCENT] = 0 #UI Values. Ignore
            accountInfo[i][IDX_LARGEST_DRAWDOWN_PERCENT] = 0 #UI Values. Ignore
    
  
        minLotSize = float(self.editMinLotSize.text())
        
        dateFrom = self.dateTo.date().toPyDate()
        
        #Load Rates
        self.fromDate = calendar.timegm(strptime(str(self.dateFrom.date().toPyDate()),"%Y-%m-%d"))
        self.toDate = calendar.timegm(strptime(str(self.dateTo.date().toPyDate()),"%Y-%m-%d"))
        for i in range(self.numSystemsInPortfolio):
            testSettings[i].fromDate = self.fromDate
            testSettings[i].toDate = self.toDate
            testSettings.spread = spreads[i]

        #passedTimeFrame = int(self.editDataTF.text())
            
        numCandles = 0
        
        historyFilePaths = []
        endingDate = []
        historyPath = "./history/"
        
        for i in range(self.numPairs):   
            historyFilePaths.append(historyPath + self.symbols[i] + '_' + str(passedTimeFrame[i]) + '.csv')
            result = checkRates(historyFilePaths[i])
            numCandles = max(result['numCandles'], numCandles)

        RatesType = Rate*numCandles
        RatesArrayType = 10 * ctypes.POINTER(Rate)
        MasterRatesArrayType = self.numPairs * ctypes.POINTER(ctypes.POINTER(Rate))
        ratesArray = MasterRatesArrayType()
        RatesInfoArrayType = self.numPairs * ctypes.POINTER(RateInfo) 
        ratesInfoArray = RatesInfoArrayType()    

        if len(list(set(historyFilePaths))) > 1:
            makeRatesCoherent(historyFilePaths)

	self.accountCurrency = str(self.comboAccountCurrency.currentText())

	for symbol in list(set(self.symbols)):
	
	    print("Generating quote files...")
	    symbolList = list(symbol)
	    baseName=symbolList[0]+symbolList[1]+symbolList[2]
            termName=symbolList[3]+symbolList[4]+symbolList[5]
	    additionalName= ''.join(symbolList[6:])
	    quoteRatePath = historyPath + symbol + '_' + str(passedTimeFrame[i]) + '.csv'
	    loadRates(quoteRatePath, numCandles, symbol, True)
	
	    if self.accountCurrency != baseName and self.accountCurrency != termName:		
	        newSymbolsList = [baseName+self.accountCurrency+additionalName, self.accountCurrency+baseName+additionalName, termName+self.accountCurrency+additionalName, self.accountCurrency+termName+additionalName]
	        for newSymbol in newSymbolsList:
	            quoteRatePath = historyPath + newSymbol + '_' + str(passedTimeFrame[i]) + '.csv'
	            if os.path.isfile(quoteRatePath):
	    	        loadRates(quoteRatePath, numCandles, newSymbol, True)

        for i in range(self.numPairs):
            if not os.path.isfile(historyFilePaths[i]):
                self.labelInfo.setText("Can't find history file: %s" % historyFilePaths[i])
                self.labelInfo.repaint()
                return True
            
            if self.numSystemsInPortfolio == 1:
                n = 0
            else:
                n = i

        
            ratesArray[i] = RatesArrayType()
            ratesInfoArray[i] = RatesInfoType()

            for j in range(0,9):
                QCoreApplication.processEvents()    
                if ratesNeededSymbol[self.strategyIDs[n]][j] != "N":
                    
                    ratesArray[i][j] = RatesType()
                    ratesInfoArray[i][j].totalBarsRequired = 0
                    
                    if ratesNeededSymbol[self.strategyIDs[n]][j] != "D":
                        #print "Loading additional rates, symbol %s..." % (ratesNeeded[self.strategyIDs[n]][j])
                        #TODO: need to check here??????
                        additionalRatePath = historyPath + ratesNeededSymbol[self.strategyIDs[n]][j] + '_' + str(passedTimeFrame[i]) + '.csv'
                        symbolUsed=ratesNeededSymbol[self.strategyIDs[n]][j]
                    else:
                        additionalRatePath = historyFilePaths[i]
                        symbolUsed=self.symbols[i]

                    self.labelInfo.setText("Loading rates for %s rates array %d system %d..." % (symbolUsed, j, i))
                    self.labelInfo.repaint()

                    #print "Loading additional rates, symbol %s..." % symbolUsed
              
                    if ratesNeededTF[self.strategyIDs[n]][j] == 0:
                    
                        #requested and passed rates sanity check
                        if int(settings[n][TIMEFRAME]) < int(passedTimeFrame[i]):
                            QtGui.QMessageBox.information(self, 'Error', "TimeFrame in data is larger than requested TimeFrame, cannot refactor rates. Aborting Simulation.")
                            return False
                
                        result = loadRates(additionalRatePath, numCandles, symbolUsed, False)
                        ratesInfoArray[i][j].requiredTimeFrame = int(settings[n][TIMEFRAME])
                    
                    else:
                    
                        #requested and passed rates sanity check
                        if int(ratesNeededTF[self.strategyIDs[n]][j]) < int(passedTimeFrame[i]):
                            QtGui.QMessageBox.information(self, 'Error', "TimeFrame in data is larger than requested TimeFrame, cannot refactor rates. Aborting Simulation.")
                            return False   
                    
                        result = loadRates(additionalRatePath, numCandles, symbolUsed, False)
                        ratesInfoArray[i][j].requiredTimeFrame = ratesNeededTF[self.strategyIDs[n]][j] 

                    ratesArray[i][j] = result['rates']
                    endingDate.append(result['endingDate'])
                    ratesInfoArray[i][j].isEnabled = 1
                    ratesInfoArray[i][j].actualTimeFrame = int(passedTimeFrame[i])
                    ratesInfoArray[i][j].totalBarsRequired = barsNeeded[self.strategyIDs[n]][j]
                    ratesInfoArray[i][j].ratesArraySize = barsNeeded[self.strategyIDs[n]][j] * 1.2 * ratesInfoArray[i][j].requiredTimeFrame/int(passedTimeFrame[i])
                    ratesInfoArray[i][j].digits = digits[i]
                    ratesInfoArray[i][j].point = 1 / (math.pow(10,ratesInfoArray[i][j].digits))
    
        #Other values
        self.accountCurrency = str(self.comboAccountCurrency.currentText())

        #Init Framework
        self.astdll.initCTesterFramework(self.asirikuyCtesterLogPath, self.logSeverity)
        
        if self.optimize:
            self.tableOptimization.setRowCount(0)
            self.buttonStopOptimization.setEnabled(True)
            self.buttonStartTest.setEnabled(False)
        else:
            self.tableTest.setRowCount(0)

        self.signalCount = 0
        self.xmlRoot = ET.Element('AsirikuyPortfolio', attrib = {'version': '1.1'})
        xmlHeader = ET.SubElement(self.xmlRoot, 'Header')
        ET.SubElement(xmlHeader, 'Deposit').text = str(settings[0][ORIGINAL_EQUITY])
        
        if self.fromDate <= ratesArray[i][0][0].time:
            ET.SubElement(xmlHeader, 'PeriodStart').text = datetime.datetime.fromtimestamp(ratesArray[i][0][0].time).isoformat()
        else:
            ET.SubElement(xmlHeader, 'PeriodStart').text = datetime.datetime.fromtimestamp(self.fromDate).isoformat()
            
        ET.SubElement(xmlHeader, 'PeriodEnd').text = datetime.datetime.fromtimestamp(endingDate[i]).isoformat()
        xmlTrades = ET.SubElement(self.xmlRoot, 'Trades')
        for i in range (self.numSystemsInPortfolio):
            xmlSystemInstance = ET.SubElement(xmlHeader, 'SystemInstance')
            xmlSystemInstance.set('id', str(i))
            ET.SubElement(xmlSystemInstance, 'AsirikuyID').text = str(self.strategyIDs[i])
            ET.SubElement(xmlSystemInstance, 'SystemName').text = strategies[self.strategyIDs[i]]
            ET.SubElement(xmlSystemInstance, 'Symbol').text = self.symbols[i]
            ET.SubElement(xmlSystemInstance, 'TimeFrame').text = str(settings[i][TIMEFRAME])
            ET.SubElement(xmlSystemInstance, 'RiskPercent').text = str(settings[i][ACCOUNT_RISK_PERCENT])
            #ET.SubElement(xmlSystemInstance, 'Deposit').text = str(settings[i][ORIGINAL_EQUITY])
            with open (setFilePaths[i], "r") as myfile: data = myfile.read()
            ET.SubElement(xmlSystemInstance, 'OtherParams').text = data
            
        if self.optimize == 1:
            self.labelInfo.setText('Running optimization...')
            self.optimizationSettings.population = int(self.editPopulation.text())
            self.optimizationSettings.crossoverProbability = float(self.editCrossoverProbability.text())
            self.optimizationSettings.mutationProbability = float(self.editMutationProbability.text())
            self.optimizationSettings.migrationProbability = float(self.editMigrationProbability.text())
            self.optimizationSettings.evolutionaryMode = self.comboEvolutionaryMode.currentIndex()
            self.optimizationSettings.elitismMode = self.comboElitismMode.currentIndex()
            self.optimizationSettings.mutationMode = self.comboMutationMode.currentIndex()
            self.optimizationSettings.crossoverMode = self.comboCrossoverMode.currentIndex()
            self.optimizationSettings.maxGenerations = int(self.editMaxGenerations.text())
            if self.checkStopIfConverged.isChecked():
                self.optimizationSettings.stopIfConverged = 1
            else:
                self.optimizationSettings.stopIfConverged = 0
            if self.checkDiscardAsymetric.isChecked():
                self.optimizationSettings.discardAssymetricSets = 1
            else:
                self.optimizationSettings.discardAssymetricSets = 0
            self.optimizationSettings.minTradesAYear = int(self.editMinTrades.text())
            self.optimizationSettings.optimizationGoal = int(self.comboGoal.currentIndex())
            
            self.optimizationThread = OptimizationThread()
            
            self.optimizationThread.optimizationResultsFilePath = self.optimizationResultsFilePath
            self.optimizationThread.astdll = self.astdll
            self.optimizationThread.astfll = self.asfdll
            self.optimizationThread.settings = settings
            self.optimizationThread.symbols = self.symbols
            self.optimizationThread.accountCurrency = self.accountCurrency
            self.optimizationThread.brokerName = self.brokerName
            self.optimizationThread.refBrokerName = self.refBrokerName
            self.optimizationThread.accountInfo = accountInfo
            self.optimizationThread.testSettings = testSettings
            self.optimizationThread.ratesInfoArray = ratesInfoArray
            self.optimizationThread.minLotSize = minLotSize
            self.optimizationThread.numCandles = numCandles
            self.optimizationThread.minimumStops= minimumStops
            self.optimizationThread.optimizationParams = optimizationParams
            self.optimizationThread.numOptimizationParams = numOptimizationParams
            self.optimizationThread.optimizationType = self.optimizationType
            self.optimizationThread.optimizationSettings = self.optimizationSettings
            self.optimizationThread.ratesArray = ratesArray
            self.optimizationThread.numCores = self.numCores
            self.optimizationThread.paramNamesArray = self.paramNamesArray
            self.optimizationThread.params = self.params
            self.optimizationThread.numPairs = self.numPairs
            
            self.connect(self.optimizationThread, SIGNAL("optimizationFinished()"),  self.optimizationFinished,  QtCore.Qt.QueuedConnection)
            self.connect(self.optimizationThread, SIGNAL("optimizationUpdate(PyQt_PyObject, PyQt_PyObject, PyQt_PyObject, PyQt_PyObject)"),  self.optimizationUpdate,  QtCore.Qt.QueuedConnection)
            self.optimizationThread.start()
            
        else:
            self.labelInfo.setText('Running test...')
            
            if self.numSystemsInPortfolio == 1: shutil.copyfile (setFilePaths[0], os.path.join(os.getcwd(),'results.set'))
            else:
                for i in range(self.numSystemsInPortfolio):
                    shutil.copyfile (setFilePaths[i], os.path.join(os.getcwd(), "result_%d.set" % (i)))

            
            self.testThread = TestThread()
            self.testThread.signalCount = self.signalCount
            self.testThread.testResultsFilePath = self.testResultsFilePath
            self.testThread.outputXMLPath = self.outputXMLPath
            self.testThread.astdll = self.astdll
            self.testThread.astfll = self.asfdll
            self.testThread.settings = settings
            self.testThread.symbols = self.symbols
            self.testThread.accountCurrency = self.accountCurrency
            self.testThread.brokerName = self.brokerName
            self.testThread.refBrokerName = self.refBrokerName
            self.testThread.accountInfo = accountInfo
            self.testThread.minLotSize = minLotSize
            self.testThread.testSettings = testSettings
            self.testThread.ratesInfoArray = ratesInfoArray
            self.testThread.numCandles = numCandles
            self.testThread.ratesArray = ratesArray
            self.testThread.minimumStops = minimumStops
            self.testThread.numSystemsInPortfolio = self.numSystemsInPortfolio
            self.testThread.numPairs = self.numPairs
            self.testThread.xmlRoot = self.xmlRoot
            self.testThread._accountInfo = accountInfoArrayType()
            self.testThread._settings = SettingsArrayType()
            self.testThread._symbols = symbolsArrayType()
            self.testThread._testSettings = TestSettingsArrayType()
            self.testThread._ratesArray = MasterRatesArrayType()
            self.testThread._ratesInfoArray = RatesInfoArrayType()
            self.connect(self.testThread, SIGNAL("testFinished(PyQt_PyObject)"),  self.testFinished,  QtCore.Qt.QueuedConnection)
            self.connect(self.testThread, SIGNAL("testUpdate(PyQt_PyObject, PyQt_PyObject, PyQt_PyObject , PyQt_PyObject, PyQt_PyObject)"),  self.testUpdate,  QtCore.Qt.QueuedConnection)
            self.testThread.start()
            QCoreApplication.processEvents()
    
    def optimizationUpdate(self, testResults, localSettings, numSettings,  iterationNumber):
        parameters = []
        for i in range(numSettings):
            parameters.append("%s=%lf" % (self.paramNamesArray[0][int(localSettings[i*2])], localSettings[i*2+1]))
        
        self.labelInfo.setText('Running optimization... Iteration: %d' % (iterationNumber))
        self.tableOptimization.insertRow(self.tableOptimization.rowCount())
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,iterationNumber)
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 0, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,testResults.totalTrades)
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 1, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,testResults.finalBalance-float(self.editBalance.text()))
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 2, item)
    
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,testResults.maxDDDepth)
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 3, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,int(testResults.maxDDLength/60/60/24))
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 4, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,testResults.pf)
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 5, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,testResults.r2)
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 6, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,testResults.ulcerIndex)
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 7, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,testResults.numShorts)
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 8, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,testResults.numLongs)
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 9, item)
        
        item = QTableWidgetItem(" ".join(parameters))
        self.tableOptimization.setItem(self.tableOptimization.rowCount()-1, 10, item)
        
        if iterationNumber < 10:
            self.tableOptimization.resizeColumnsToContents()
        
        if iterationNumber == 1 or iterationNumber % 5 == 0:
            self.loadGraphic(1)
        
    def optimizationFinished(self):
        self.running = False
        self.loadGraphic(1)
        self.labelInfo.setText('Optimization finished!')
        self.buttonStartTest.setEnabled(True)
        self.tableOptimization.setSortingEnabled(True)
        self.tableOptimization.sortItems(0)
        self.buttonStopOptimization.setEnabled(False)
            
    def testUpdate(self, id, percentageOfTestCompleted, lastTrade, currentBalance, symbol):
        self.tableTest.insertRow(self.tableTest.rowCount())
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,lastTrade.ticket)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 0, item)
        
        item = QTableWidgetItem(opType[int(lastTrade.type)])
        self.tableTest.setItem(self.tableTest.rowCount()-1, 1, item)
        
        item = QTableWidgetItem(strftime("%d/%m/%Y %H:%M", gmtime(lastTrade.openTime)))
        self.tableTest.setItem(self.tableTest.rowCount()-1, 2, item)
    
        item = QTableWidgetItem(strftime("%d/%m/%Y %H:%M", gmtime(lastTrade.closeTime)))
        self.tableTest.setItem(self.tableTest.rowCount()-1, 3, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,lastTrade.openPrice)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 4, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,lastTrade.closePrice)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 5, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,lastTrade.lots)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 6, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,lastTrade.profit)
        if lastTrade.profit > 0: item.setTextColor(QtCore.Qt.darkGreen)
        else: item.setTextColor(QtCore.Qt.darkRed)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 7, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,lastTrade.stopLoss)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 8, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,lastTrade.takeProfit)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 9, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,currentBalance)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 10, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,id)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 11, item)
        
        item = QTableWidgetItem(symbol)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 12, item)
        
        item = QTableWidgetItem()
        item.setData(QtCore.Qt.DisplayRole,lastTrade.swap)
        self.tableTest.setItem(self.tableTest.rowCount()-1, 13, item)
     
        if lastTrade.ticket < 10:
            self.tableTest.resizeColumnsToContents() 
                
            QCoreApplication.processEvents()     

    def testFinished(self,  testResult):
        self.running = False
        if self.numPairs == 1: self.tableTest.setSortingEnabled(True)
        self.tableTest.sortItems(0)
        self.labelInfo.setText('Test finished!')
        self.labelTotalTrades.setText(str(testResult.totalTrades))
        self.labelLongs.setText(str(testResult.numLongs))
        self.labelShorts.setText(str(testResult.numShorts))
        self.labelFinalBalance.setText('%.2lf' % (testResult.finalBalance))
        self.labelMaxDD.setText('%.2lf' % (testResult.maxDDDepth))
        self.labelMaxDDLength.setText(str(int(testResult.maxDDLength/60/60/24)))
        self.labelPF.setText('%.2lf' % (testResult.pf))
        self.labelR2.setText('%.2lf' % (testResult.r2))
        self.labelUlcerIndex.setText('%.2lf' % (testResult.ulcerIndex))
        if self.numSystemsInPortfolio == 1: 
            if self.numPairs > 1:
                self.loadGraphic(3)
                for i in range(self.numPairs):
                    plotTestResult(self.symbols[i] + "_results.txt")
                    csvToHTML(os.path.join(os.path.split(self.testResultsFilePath)[0], self.symbols[i] + '_results.txt'), os.path.join(os.path.split(self.testResultsFilePath)[0], self.symbols[i] + '_results.htm'), testResult, os.path.join(os.path.split(self.testResultsFilePath)[0], self.symbols[i] + '_results.png'))
            else:
                self.labelInfo.setText('Processing Graphics, please wait!')
                QCoreApplication.processEvents() 
                self.loadGraphic(0)
                csvToHTML(self.testResultsFilePath, "results.htm", testResult, "results.png")
                self.labelInfo.setText('Test finished!')
        else:
            self.labelInfo.setText('Processing Graphis, please wait!')
            QCoreApplication.processEvents() 
            self.loadGraphic(2)
            csvToHTML(self.testResultsFilePath, "results.htm", testResult, "results.png")
            self.labelInfo.setText('Test finished!')
        
    @pyqtSignature("")
    def on_buttonLoadSetFile_released(self):
        """
        Slot documentation goes here.
        """
        setPath = []
        setPath.append(str(QtGui.QFileDialog.getOpenFileName()))
        if os.path.exists(setPath[0]): 
            if str(self.editSet.text()).strip() == '': setsArray = setPath
            else: setsArray = str(self.editSet.text()).split(',') + setPath
            if len(setsArray) == 1:
                shutil.copyfile (setPath[0],  self.baseSetFilePath)
            listSets = ','.join(setsArray)
            self.editSet.setText(listSets)
            self.setFilePath = listSets
        else:
            QtGui.QMessageBox.information(self, 'Error', 'Set file doesn\'t exist')
    
    @pyqtSignature("")
    def on_buttonStopOptimization_released(self):
        """
        Slot documentation goes here.
        """
        self.labelInfo.setText('Stoping optimization...')
        self.astdll.stopOptimization.restype = c_void_p
        self.astdll.stopOptimization()
        if self.checkOptimize.isChecked() and self.comboOptimizationType.currentIndex() == 1:
            QtGui.QMessageBox.information(self, 'Info', 'The optimization will stop when the current generation is finished')
    
    @pyqtSignature("")
    def on_buttonModifySetFile_released(self):
        """
        Slot documentation goes here.
        """
        try:
            if os.name == 'nt':
                retcode = subprocess.call("start " + str(self.setFilePath), shell=True)
            else:
                retcode = subprocess.call("open " + str(self.setFilePath), shell=True)
                
        except OSError as e:
            QtGui.QMessageBox.information(self, 'Error', "Execution failed:", e) 
    
    @pyqtSignature("")
    def on_buttonAddStrategy_pressed(self):
        """
        Slot documentation goes here.
        """
        strategy = []
        strategy.append(int(self.comboStrategy.currentIndex()))
        if str(self.editPortfolioStrategies.text()).strip() == '': strategiesArray = strategy
        else: strategiesArray = str(self.editPortfolioStrategies.text()).split(',') + strategy
        listStrategies = ','.join(map(str,strategiesArray))
        self.editPortfolioStrategies.setText(listStrategies)


class TestThread(QThread):
    def __init__(self, parent=None):
        super(TestThread, self).__init__(parent)
    
    def testUpdate(self, id, percentageOfTestCompleted, lastTrade, currentBalance, symbol):
        self.testFilePath.write("%d,%s,%s,%s,%.5lf,%.5lf,%lf,%.2lf,%.5lf,%.5lf,%.2lf,%d,%s,%.2lf\n" % (
                int(lastTrade.ticket), opType[int(lastTrade.type)], strftime("%d/%m/%Y %H:%M", gmtime(lastTrade.openTime)), 
                strftime("%d/%m/%Y %H:%M", gmtime(lastTrade.closeTime)), 
                lastTrade.openPrice, lastTrade.closePrice, lastTrade.lots, lastTrade.profit, lastTrade.stopLoss, 
                lastTrade.takeProfit, currentBalance, id, symbol, lastTrade.swap
                )
                )
        self.emit(SIGNAL("testUpdate(PyQt_PyObject, PyQt_PyObject, PyQt_PyObject, PyQt_PyObject, PyQt_PyObject)"), id, percentageOfTestCompleted, lastTrade, currentBalance, symbol)
        
    def testFinished(self, testResult):
        xmlHeader = self.xmlRoot.find('Header')
        ET.SubElement(xmlHeader, 'PortfolioName').text = ''
        ET.SubElement(xmlHeader, 'NoOfTrades').text = str(self.signalCount)
        ET.ElementTree(self.xmlRoot).write(self.outputXMLPath, encoding="UTF-8", xml_declaration=True)
        insertLine(self.outputXMLPath, '<?xml-stylesheet type="text/xsl" href="include/ast.xsl"?>', 1)
        #prettyXML(self.outputXMLPath)
        self.emit(SIGNAL("testFinished(PyQt_PyObject)"), testResult)
        QCoreApplication.processEvents()
        
    def signalUpdate(self, tradeSignal):
        xmlTrades = self.xmlRoot.find('Trades')
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
        QCoreApplication.processEvents()
        self.signalCount = self.signalCount + 1
        
    def runTest(self):
        
        if (self.numSystemsInPortfolio == 1): #Simple Test

            error_c = c_char_p()
            TEST_UPDATE = CFUNCTYPE(c_void_p, c_int, c_double, OrderInfo, c_double, c_char_p)
            testUpdate_c = TEST_UPDATE(self.testUpdate)
            
            TEST_FINISHED = CFUNCTYPE(c_void_p, TestResult)
            testFinished_c = TEST_FINISHED(self.testFinished)
            
            SIGNAL_UPDATE = CFUNCTYPE(c_void_p, TradeSignal)
            signalUpdate_c = SIGNAL_UPDATE(self.signalUpdate)
            
            if self.numPairs > 1: 
                for i in range(self.numPairs): 
                    try: 
                        os.remove(os.path.join(os.path.split(self.testResultsFilePath)[0], self.symbols[i] + '_' + os.path.split(self.testResultsFilePath)[1])) 
                    except: pass
            
            for i in range(self.numPairs):
                if self.numPairs == 1: self.testFilePath = open(self.testResultsFilePath,  'w',  0)
                else: self.testFilePath = open(os.path.join(os.path.split(self.testResultsFilePath)[0], self.symbols[i] + '_' + os.path.split(self.testResultsFilePath)[1]),  'w',  0)
                self.testFilePath.write("Order Number, Order Type, Open Time, Close Time, Open Price, Close Price, Lots, Profit, SL, TP, Balance, ID, Pair, Swap\n")
                
                self._settings[0] = self.settings[0]
                self._accountInfo[0] = self.accountInfo[0]
                self.accountInfo[0][IDX_MINIMUM_STOP] = float(self.minimumStops[i])
                self._symbols[0] = self.symbols[i]
                self._testSettings[0] = self.testSettings[0]
                self._ratesArray[0] = self.ratesArray[i]
                self._ratesInfoArray[0] = self.ratesInfoArray[i]
        
                
                self.astdll.runPortfolioTest.restype = TestResult
                result = self.astdll.runPortfolioTest (
                        c_int(1),ctypes.pointer(self._settings), ctypes.pointer(self._symbols), self.accountCurrency, self.brokerName, 
                        self.refBrokerName, ctypes.pointer(self._accountInfo), ctypes.pointer(self._testSettings), ctypes.pointer(self._ratesInfoArray), 
                        c_int(self.numCandles), c_int(1), ctypes.pointer(self._ratesArray), c_double(self.minLotSize),testUpdate_c, testFinished_c, signalUpdate_c
                        )
                self.testFilePath.close()
        
        else: #Portfolio Test
            self.testFilePath = open(self.testResultsFilePath,  'w',  0)
            self.testFilePath.write("Order Number, Order Type, Open Time, Close Time, Open Price, Close Price, Lots, Profit, SL, TP, Balance, ID, Pair, Swap\n")
            error_c = c_char_p()
            
            TEST_UPDATE = CFUNCTYPE(c_void_p, c_int, c_double, OrderInfo, c_double, c_char_p)
            testUpdate_c = TEST_UPDATE(self.testUpdate)
            
            TEST_FINISHED = CFUNCTYPE(c_void_p, TestResult)
            testFinished_c = TEST_FINISHED(self.testFinished)
            
            SIGNAL_UPDATE = CFUNCTYPE(c_void_p, TradeSignal)
            signalUpdate_c = SIGNAL_UPDATE(self.signalUpdate)
            
            self.astdll.runPortfolioTest.restype = TestResult
            result = self.astdll.runPortfolioTest (
                c_int(1), ctypes.pointer(self.settings), ctypes.pointer(self.symbols), self.accountCurrency, self.brokerName, self.refBrokerName, 
                ctypes.pointer(self.accountInfo), ctypes.pointer(self.testSettings), ctypes.pointer(self.ratesInfoArray), c_int(self.numCandles), 
                c_int(self.numSystemsInPortfolio), ctypes.pointer(self.ratesArray), c_double(self.minLotSize),testUpdate_c, testFinished_c, signalUpdate_c
                )

            self.testFilePath.close()
  
    def run(self):
        self.runTest()

class OptimizationThread(QThread):
    def __init__(self, parent=None):
        super(OptimizationThread, self).__init__(parent)
    
    def optimizationUpdate(self, testResults, settings, numSettings):
        self.iterationNumber = self.iterationNumber + 1
        parameters = []
        localSettings = []

        for i in range(numSettings):
            parameters.append("%s=%lf" % (self.paramNamesArray[0][int(settings[i*2])], settings[i*2+1]))
            localSettings.append(settings[i*2])
            localSettings.append(settings[i*2+1])
        line = "%d,%s,%d,%lf,%lf,%d,%lf,%lf,%lf,%d,%d,%s\n" % (self.iterationNumber, testResults.symbol, testResults.totalTrades, testResults.finalBalance, testResults.maxDDDepth, 
                                                    int(testResults.maxDDLength/60/60/24), float(testResults.pf), testResults.r2, testResults.ulcerIndex,
                                                    testResults.numShorts, testResults.numLongs, " ".join(parameters))
        self.optimizationFilePath.write(line)
        self.emit(SIGNAL("optimizationUpdate(PyQt_PyObject, PyQt_PyObject, PyQt_PyObject, PyQt_PyObject)"), testResults, localSettings, numSettings,  self.iterationNumber)
        
    def optimizationFinished(self):
        self.emit(SIGNAL("optimizationFinished()"))
        
    def runOptimization(self):
        self.iterationNumber = 0
        self.optimizationFilePath = open(self.optimizationResultsFilePath,  'w',  0)
        self.optimizationFilePath.write("Iteration, Symbol, NumTrades, Profit, maxDD, maxDDLength, PF, R2, ulcerIndex, numShorts, numLongs, Set Parameters\n")
        error_c = c_char_p()
        OPTIMIZATION_UPDATE = CFUNCTYPE(c_void_p, TestResult, POINTER(c_double), c_int)
        optimizationUpdate_c = OPTIMIZATION_UPDATE(self.optimizationUpdate)
    
        OPTIMIZATION_FINISHED = CFUNCTYPE(c_void_p)
        optimizationFinished_c = OPTIMIZATION_FINISHED(self.optimizationFinished)
        
        self.astdll.runOptimizationMultipleSymbols.restype = c_int

        if not self.astdll.runOptimizationMultipleSymbols (
                    ctypes.pointer(self.optimizationParams[0]), c_int(self.numOptimizationParams[0]), c_int(self.optimizationType), 
                    self.optimizationSettings, c_int(self.numCores), 
                    self.settings[0], ctypes.pointer(self.symbols), self.accountCurrency, self.brokerName, self.refBrokerName, 
                    self.accountInfo[0], ctypes.pointer(self.testSettings), ctypes.pointer(self.ratesInfoArray), c_int(self.numCandles), 
                    c_int(self.numPairs), ctypes.pointer(self.ratesArray), c_double(self.minLotSize),  optimizationUpdate_c, optimizationFinished_c, 
                    byref(error_c)
                ):
                print(error_c.value)
                
        self.optimizationFilePath.close()
    
    def run(self):
        self.runOptimization()

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)
    ui = MainWindow()
    ui.show()
    sys.exit(app.exec_())
