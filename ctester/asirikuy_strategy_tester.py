#!/usr/bin/env python3
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
import datetime, ctypes, os, platform, configparser, math, csv, calendar, argparse,  sys,  signal, shutil, threading
import xml.etree.ElementTree as ET
from include.asirikuy import *

# Global lock for thread-safe optimizationUpdate callback
optimizationUpdateLock = threading.Lock()

def main():
    print("[DEBUG] ===== Starting main() =====")
    main_start_time = datetime.datetime.now()
    global iterationNumber, lines, numSystemsInPortfolio, numPairs, setFilePaths
    global fromDate, toDate
    global astdll
    print("[DEBUG] Globals initialized")
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

    print("[DEBUG] Loading library...")
    system = platform.system()
    if (system == "Windows"):
        astdll = loadLibrary('CTesterFrameworkAPI')
    elif (system == "Linux"):
        astdll = loadLibrary('libCTesterFrameworkAPI.so')
    elif (system == "Darwin"):
        astdll = loadLibrary('libCTesterFrameworkAPI.dylib')
    else:
        print("No shared library loading support for OS %s" % (system))
        return False
    print("[DEBUG] Library loaded successfully")

    print("[DEBUG] Importing graphics module...")
    from include.graphics import plotTestResult, plotMultipleTestResults, plotOptimizationResult, plotPortfolioTestResult
    print("[DEBUG] Graphics module imported")

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
    # Note: outputOptimizationFile and outputFile will be updated later to use instance-specific folder
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
    print(f"[DEBUG] Reading config file: {configFilePath}")
    global config
    config = readConfigFile(configFilePath)
    if not config:
        print("Error reading config file %s" % (configFilePath))
        return False
    print("[DEBUG] Config file read successfully")

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

    # Support both "set" and "setFile" for backward compatibility
    set_key = "setFile" if config.has_option("strategy", "setFile") else "set"
    for s in config.get("strategy", set_key).split(','):
        if os.path.exists(s):
            setFilePaths.append(s)
        else:
            setFilePaths.append("./sets/%s" % (s))
    for index,s in enumerate(config.get("strategy", "pair").split(',')):
        symbols[index] = s.encode('utf-8')  # Python 3: encode string to bytes
    
    # Read set files early to extract instance IDs for instance-specific log folders
    sets = []
    instanceIds = []
    for index, s in enumerate(setFilePaths):
        print(f"[DEBUG] Reading set file {index} early for instance ID: {s}")
        sets.append(MT4Set(s))
        if not sets[index].content:
            print("Error reading set file %s" % (s))
            return False
        # Extract instance ID from set file
        instanceId = 0
        if sets[index].content.has_option('main', 'STRATEGY_INSTANCE_ID'):
            instanceId = int(float(sets[index].mainParams["STRATEGY_INSTANCE_ID"]['value']))
        instanceIds.append(instanceId)
        print(f"[DEBUG] Set file {index} instance ID: {instanceId}")
    
    # Determine if this is an optimization run
    optimize = config.getboolean("optimization", "optimize") if config.has_option("optimization", "optimize") else False
    
    # Create instance-specific log folder if we have instance IDs
    # Format: tmp/{SYMBOL}_{INSTANCE_ID}_optimize/log/ for optimization, tmp/{SYMBOL}_{INSTANCE_ID}/log/ for backtest
    instanceLogFolder = None
    instanceConfigPath = None
    if len(instanceIds) > 0 and instanceIds[0] > 0:
        symbol_str = symbols[0].decode('utf-8') if isinstance(symbols[0], bytes) else symbols[0]
        if optimize:
            instanceLogFolder = f"./tmp/{symbol_str}_{instanceIds[0]}_optimize/log"
            instanceTmpFolder = f"./tmp/{symbol_str}_{instanceIds[0]}_optimize"
        else:
            instanceLogFolder = f"./tmp/{symbol_str}_{instanceIds[0]}/log"
            instanceTmpFolder = f"./tmp/{symbol_str}_{instanceIds[0]}"
        os.makedirs(instanceLogFolder, exist_ok=True)
        print(f"[DEBUG] Created instance-specific log folder: {instanceLogFolder}")
        
        # Update CTester log path
        asirikuyCtesterLogPath = os.path.join(instanceLogFolder, 'AsirikuyCTester.log')
        if execUnderMPI and rank > 0:
            asirikuyCtesterLogPath = os.path.join(instanceLogFolder, f'AsirikuyCTester_{rank}.log')
        print(f"[DEBUG] Updated CTester log path to: {asirikuyCtesterLogPath}")
        
        # Create instance-specific AsirikuyConfig.xml with updated log folder
        if sets[0].content.has_option('main', 'FRAMEWORK_CONFIG'):
            originalConfigPath = sets[0].mainParams["FRAMEWORK_CONFIG"]['value']
            # Handle relative paths
            if not os.path.isabs(originalConfigPath):
                originalConfigPath = os.path.join(os.getcwd(), originalConfigPath.lstrip('./'))
            
            if os.path.exists(originalConfigPath):
                import xml.etree.ElementTree as ET
                # Read original config
                tree = ET.parse(originalConfigPath)
                root = tree.getroot()
                
                # Update log folder path
                logging_elem = root.find('Logging')
                if logging_elem is not None:
                    folder_elem = logging_elem.find('Folder')
                    if folder_elem is not None:
                        # Use absolute path for log folder
                        abs_log_folder = os.path.abspath(instanceLogFolder)
                        folder_elem.text = abs_log_folder
                        print(f"[DEBUG] Updated AsirikuyConfig.xml log folder to: {abs_log_folder}")
                    
                    # Update MaxSeverity based on logseverity from config file (if available)
                    # This prevents DEBUG logs from filling up disk space during optimization
                    if config.has_option("misc", "logSeverity") or config.has_option("misc", "logseverity"):
                        try:
                            log_severity_key = "logSeverity" if config.has_option("misc", "logSeverity") else "logseverity"
                            log_severity_str = config.get("misc", log_severity_key).split(';')[0].strip()
                            log_severity = int(log_severity_str)
                            max_severity_elem = logging_elem.find('MaxSeverity')
                            if max_severity_elem is not None:
                                max_severity_elem.text = str(log_severity)
                                print(f"[DEBUG] Updated AsirikuyConfig.xml MaxSeverity to: {log_severity} (from config logseverity)")
                            else:
                                # Create MaxSeverity element if it doesn't exist
                                max_severity_elem = ET.SubElement(logging_elem, 'MaxSeverity')
                                max_severity_elem.text = str(log_severity)
                                print(f"[DEBUG] Created AsirikuyConfig.xml MaxSeverity: {log_severity} (from config logseverity)")
                        except (ValueError, KeyError) as e:
                            print(f"[DEBUG] Could not parse logseverity from config, keeping original MaxSeverity: {e}")
                
                # Update TempFileFolderPath to use instance-specific tmp folder
                # This controls where state files, OrderInfo files, etc. are written
                temp_file_folder_elem = root.find('TempFileFolderPath')
                if temp_file_folder_elem is not None:
                    abs_tmp_folder = os.path.abspath(instanceTmpFolder)
                    temp_file_folder_elem.text = abs_tmp_folder
                    print(f"[DEBUG] Updated AsirikuyConfig.xml TempFileFolderPath to: {abs_tmp_folder}")
                
                # Save instance-specific config
                instanceConfigPath = os.path.join(instanceTmpFolder, 'AsirikuyConfig.xml')
                os.makedirs(instanceTmpFolder, exist_ok=True)
                tree.write(instanceConfigPath, encoding='utf-8', xml_declaration=True)
                print(f"[DEBUG] Created instance-specific config: {instanceConfigPath}")
                
                # Update set file's FRAMEWORK_CONFIG reference
                sets[0].mainParams["FRAMEWORK_CONFIG"]['value'] = instanceConfigPath
                print(f"[DEBUG] Updated set file FRAMEWORK_CONFIG to: {instanceConfigPath}")
                
                # Update output file paths to use instance-specific folder
                # For optimization files
                if optimize:
                    # Extract just the filename if a path was provided
                    optFileBase = os.path.basename(outputOptimizationFile) if os.path.dirname(outputOptimizationFile) else outputOptimizationFile
                    outputOptimizationFile = os.path.join(instanceTmpFolder, optFileBase)
                    print(f"[DEBUG] Updated outputOptimizationFile to: {outputOptimizationFile}")
                # For test result files
                # Extract just the filename if a path was provided
                testFileBase = os.path.basename(outputFile) if os.path.dirname(outputFile) else outputFile
                outputFile = os.path.join(instanceTmpFolder, testFileBase)
                outputXMLPath = outputFile + '.xml'
                print(f"[DEBUG] Updated outputFile to: {outputFile}")
                print(f"[DEBUG] Updated outputXMLPath to: {outputXMLPath}")

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
            print(numSystemsInPortfolio)
            print(len(digits))
            print(len(strategyIDs))
            print(len(spreads))
            print(len(rateRequirements))


            print("You must have the same number of items for minimum stops, requirement arrays, setFile, pair, spread, digits, and strategyID")
            return False
    else:
        if len(symbols) != len(digits) or len(symbols) != len(spreads) or len(spreads) != len(digits) or len(minimumStops) != len(digits):
            print("For multi-instrument tests the number of symbols, minimum stop values, digits and spreads must be equal.")
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
    # Handle inline comments in config values (Python 3 configparser is stricter)
    log_severity_str = config.get("misc", "logSeverity").split(';')[0].strip()  # Remove inline comment
    astdll.initCTesterFramework(asirikuyCtesterLogPath.encode('utf-8'), int(log_severity_str))

    # Sets are already read earlier for instance ID extraction
    # Print optimization array info for debugging
    for index in range(len(sets)):
        if hasattr(sets[index], 'optimizationArray'):
            print(f"[DEBUG] Set file {index} optimizationArray: {sets[index].optimizationArray}")
        else:
            print(f"[DEBUG] Set file {index} has no optimizationArray attribute")
    # Handle inline comments in config values (Python 3 configparser is stricter)
    optimization_type_str = config.get("optimization", "optimizationType").split(';')[0].strip()
    optimizationType    = int(optimization_type_str)
    # Handle case-insensitive config key (numCores vs numcores)
    # Note: ConfigParser is case-insensitive by default, but we check both for safety
    try:
        num_cores_str = config.get("optimization", "numCores").split(';')[0].strip()
        print("[DEBUG] Read numCores from config (numCores): %s" % num_cores_str, flush=True)
    except (configparser.NoOptionError, KeyError):
        try:
            num_cores_str = config.get("optimization", "numcores").split(';')[0].strip()
            print("[DEBUG] Read numCores from config (numcores): %s" % num_cores_str, flush=True)
        except (configparser.NoOptionError, KeyError):
            num_cores_str = "1"  # Default to 1 if not found
            print("[DEBUG] WARNING: numCores not found in config, defaulting to 1", flush=True)
    numCores = int(num_cores_str)
    print("[DEBUG] Final numCores value: %d" % numCores, flush=True)

    #Config values
    accountCurrency = config.get("account", "currency")
    
    # Read brokerName and refBrokerName from config, with fallback defaults
    # IMPORTANT: Encode to bytes to ensure the string buffer persists
    # ctypes creates temporary buffers for Python strings, which can be freed
    # Encoding to bytes and keeping a reference ensures the buffer stays alive
    try:
        brokerName = config.get("account", "brokerName").encode('utf-8')
    except:
        brokerName = "Pepperstone Group Limited".encode('utf-8')
    
    try:
        refBrokerName = config.get("account", "refBrokerName").encode('utf-8')
    except:
        refBrokerName = brokerName  # Use same as brokerName if not specified
    
    
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
        
        print(f"[DEBUG] System {i}: BEFORE loading additional params - settings[{i}][3] (AUTOBBS_RISK_CAP) = {settings[i][3]}")
        print(f"[DEBUG] System {i}: optimizationArray BEFORE append: {sets[i].optimizationArray}")
           
        optimizationArrays.append(sets[i].optimizationArray)
        print(f"[DEBUG] System {i}: optimizationArray length = {len(sets[i].optimizationArray)}")
        print(f"[DEBUG] System {i}: optimizationArray contents = {sets[i].optimizationArray}")

        paramNamesArray.append(paramNames)
        index = 0
        print(f"[DEBUG] System {i}: Starting to load parameters from 'additional' section...")
        for param in sets[i].content.items("additional"):
            if param[0].find(",") == -1:
                param_key = param[0].upper()
                print(f"[DEBUG] System {i}: Found param in additional section: {param_key} = {param[1]}")
                if param_key in paramIndexes:
                    param_index = paramIndexes[param_key]
                    old_value = settings[i][param_index]
                    settings[i][param_index] = float(param[1])
                    print(f"[DEBUG] System {i}: LOADED {param_key} -> settings[{i}][{param_index}] = {param[1]} (was {old_value})")
                    if param_key == "AUTOBBS_RISK_CAP":
                        print(f"[DEBUG] System {i}: *** AUTOBBS_RISK_CAP loaded: settings[{i}][3] = {settings[i][3]} ***")
                    paramNamesArray[i][index] = param_key
                    index += 1
                else:
                    print(f"[DEBUG] System {i}: SKIPPED {param_key} - not in paramIndexes")
                # Skip unknown parameters (they may be for other strategies)
        
        print(f"[DEBUG] System {i}: AFTER loading additional params - settings[{i}][3] (AUTOBBS_RISK_CAP) = {settings[i][3]}")


        OptimizationParamType = OptimizationParam * len(optimizationArrays[i])
        optimizationParams.append(OptimizationParamType())

        numOptimizationParams.append(0)
        print(f"[DEBUG] System {i}: Processing {len(optimizationArrays[i])} optimization parameters")
        for key, value in list(optimizationArrays[i].items()):
            print(f"[DEBUG] System {i}: Adding param - key={key} (paramIndex), value={value}, optimizationIndex={numOptimizationParams[i]}")
            if key == 3:
                print(f"[DEBUG] System {i}: *** Found AUTOBBS_RISK_CAP in optimizationArray: key=3, range=({value[0]}, {value[1]}, {value[2]}) ***")
            optimizationParams[i][numOptimizationParams[i]].index = key
            optimizationParams[i][numOptimizationParams[i]].start = value[0]
            optimizationParams[i][numOptimizationParams[i]].step  = value[1]
            optimizationParams[i][numOptimizationParams[i]].stop  = value[2]
            numOptimizationParams[i] += 1
        print(f"[DEBUG] System {i}: Total optimization params = {numOptimizationParams[i]}")
        print(f"[DEBUG] System {i}: Final settings[{i}][3] (AUTOBBS_RISK_CAP) = {settings[i][3]}")

        testSettings[i] = TestSettings()
        testSettings[i].spread = spreads[i]
        testSettings[i].is_calculate_expectancy = is_calculate_expectancy

    print(f"[DEBUG] After processing all systems:", flush=True)
    print(f"[DEBUG] optimize = {optimize}", flush=True)
    print(f"[DEBUG] numOptimizationParams[0] = {numOptimizationParams[0]}", flush=True)
    import sys
    sys.stdout.flush()
    if numOptimizationParams[0] == 0 and optimize:
        print("[DEBUG] ERROR: Nothing to optimize. Check your set file.", flush=True)
        print("[DEBUG] This means no parameters with ',F = 1' were found in the set file.", flush=True)
        sys.stdout.flush()
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
    # Handle inline comments and date format (dd/mm/yy or YYYY-MM-DD)
    from_date_str = config.get("strategy", "fromDate").split(';')[0].strip()
    to_date_str = config.get("strategy", "toDate").split(';')[0].strip()
    
    # Try dd/mm/yy format first (legacy format), then YYYY-MM-DD
    try:
        fromDate = calendar.timegm(strptime(from_date_str, "%d/%m/%y"))
    except ValueError:
        fromDate = calendar.timegm(strptime(from_date_str, "%Y-%m-%d"))
    
    try:
        toDate = calendar.timegm(strptime(to_date_str, "%d/%m/%y"))
    except ValueError:
        toDate = calendar.timegm(strptime(to_date_str, "%Y-%m-%d"))
    for i in range(numPairs):
        testSettings[i].fromDate = fromDate
        testSettings[i].toDate = toDate
        testSettings.spread = spreads[i]

    numCandles = 0

    historyFilePaths = []
    endingDate = []
    
    
    for i in range(numPairs):
        # Python 3: symbols[i] is bytes, decode to string for concatenation
        symbol_str = symbols[i].decode('utf-8') if isinstance(symbols[i], bytes) else symbols[i]
        historyFilePaths.append(historyPath + symbol_str + '_' + str(passedTimeFrame[i]) + '.csv')
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

        print("Generating quote files...")
        # Python 3: symbol is bytes, decode to string for string operations
        symbol_str = symbol.decode('utf-8') if isinstance(symbol, bytes) else symbol
        symbolList = list(symbol_str)
        baseName=symbolList[0]+symbolList[1]+symbolList[2]
        termName=symbolList[3]+symbolList[4]+symbolList[5]
        additionalName= ''.join(symbolList[6:])
        quoteRatePath = historyPath + symbol_str + '_' + str(passedTimeFrame[i]) + '.csv'
        loadRates(quoteRatePath, numCandles, symbol, True)

        if accountCurrency != baseName and accountCurrency != termName:
            newSymbolsList = [baseName+accountCurrency+additionalName, accountCurrency+baseName+additionalName, termName+accountCurrency+additionalName, accountCurrency+termName+additionalName]
            for newSymbol in newSymbolsList:
                quoteRatePath = historyPath + newSymbol + '_' + str(passedTimeFrame[i]) + '.csv'
                if os.path.isfile(quoteRatePath):
                    # Convert newSymbol to bytes for loadRates if needed
                    newSymbol_bytes = newSymbol.encode('utf-8') if isinstance(symbol, bytes) else newSymbol
                    loadRates(quoteRatePath, numCandles, newSymbol_bytes, True)

    for i in range(numPairs):
        if not os.path.isfile(historyFilePaths[i]):
            print("Can't find history file: %s" % historyFilePaths[i])
            return False
        print("Loading rates for %s..." % (symbols[i]))

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
                    print("Loading additional rates, symbol %s..." % (ratesNeededSymbol[strategyIDs[n]][j]))
                    additionalRatePath = historyPath + ratesNeededSymbol[strategyIDs[n]][j] + '_' + str(passedTimeFrame[i]) + '.csv'
                    symbolUsed=ratesNeededSymbol[strategyIDs[n]][j]
                else:
                    additionalRatePath = historyFilePaths[i]
                    symbolUsed=symbols[i]

                if ratesNeededTF[strategyIDs[n]][j] == 0:

                    #requested and passed rates sanity check
                    if int(settings[n][TIMEFRAME]) < int(passedTimeFrame[i]):
                        print("passedTimeFrame needs to be below required time frame for proper refactoring.")
                        return False

                    result = loadRates(additionalRatePath, numCandles, symbolUsed, False)
                    ratesInfoArray[i][j].requiredTimeFrame = int(settings[n][TIMEFRAME])

                else:

                    #requested and passed rates sanity check
                    if int(ratesNeededTF[strategyIDs[n]][j]) < int(passedTimeFrame[i]):
                        print("passedTimeFrame needs to be below required time frame for proper refactoring.")
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
        print("[DEBUG] Optimization mode enabled", flush=True)
        print("[DEBUG] numOptimizationParams[0] =", numOptimizationParams[0], flush=True)
        print("[DEBUG] optimizationType =", optimizationType, flush=True)
        # CRITICAL: Verify numCores value before printing
        print("[DEBUG] numCores variable type:", type(numCores), flush=True)
        print("[DEBUG] numCores variable value:", numCores, flush=True)
        print("[DEBUG] numCores =", numCores, flush=True)
        print("[DEBUG] numPairs =", numPairs, flush=True)
        print("[DEBUG] numCandles =", numCandles, flush=True)
        import sys
        sys.stdout.flush()
        
        # Debug: Print optimization parameters
        if numOptimizationParams[0] > 0:
            print("[DEBUG] Optimization parameters:", flush=True)
            for i in range(numOptimizationParams[0]):
                param = optimizationParams[0][i]
                print(f"  [DEBUG] Param {i}: index={param.index}, start={param.start}, step={param.step}, stop={param.stop}", flush=True)
        else:
            print("[DEBUG] WARNING: No optimization parameters found!", flush=True)
        import sys
        sys.stdout.flush()
        
        if execUnderMPI == False or (execUnderMPI == True and rank == 1):
            f = open(outputOptimizationFile + ".csv", 'w')
            header = "Iteration, Symbol, NumTrades, Profit, maxDD, maxDDLength, PF, R2, ulcerIndex, Sharpe, CAGR, CAGR to Max DD, numShorts, numLongs, Set Parameters\n"
            f.write(header)
            f.flush()  # CRITICAL: Flush immediately to ensure header is written
            print("[DEBUG] Opened optimization output file:", outputOptimizationFile + ".csv")
            print("[DEBUG] Wrote header, file handle:", f)
            import os as os_module
            print("[DEBUG] File exists:", os_module.path.exists(outputOptimizationFile + ".csv"))
            print("[DEBUG] File size:", os_module.path.getsize(outputOptimizationFile + ".csv") if os_module.path.exists(outputOptimizationFile + ".csv") else "N/A")

        OPTIMIZATION_UPDATE = CFUNCTYPE(c_void_p, TestResult, POINTER(c_double), c_int)
        optimizationUpdate_c = OPTIMIZATION_UPDATE(optimizationUpdate)
        print("[DEBUG] Created optimizationUpdate callback:", optimizationUpdate_c)

        OPTIMIZATION_FINISHED = CFUNCTYPE(c_void_p)
        optimizationFinished_c = OPTIMIZATION_FINISHED(optimizationFinished)
        print("[DEBUG] Created optimizationFinished callback:", optimizationFinished_c)

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
        
        print("[DEBUG] Optimization settings configured:")
        print(f"  [DEBUG] population={optimizationSettings.population}, maxGenerations={optimizationSettings.maxGenerations}")
        print(f"  [DEBUG] optimizationGoal={optimizationSettings.optimizationGoal}")
        print("[DEBUG] About to call runOptimizationMultipleSymbols...", flush=True)
        print("[DEBUG] optimizationParams[0] pointer:", ctypes.pointer(optimizationParams[0]), flush=True)
        print("[DEBUG] settings[0] pointer:", settings[0], flush=True)
        print("[DEBUG] symbols pointer:", ctypes.pointer(symbols), flush=True)
        print("[DEBUG] accountCurrency:", accountCurrency, flush=True)
        print("[DEBUG] brokerName:", brokerName, flush=True)
        print("[DEBUG] refBrokerName:", refBrokerName, flush=True)
        
        # Parameter validation
        print("[DEBUG] Validating parameters...", flush=True)
        if optimizationParams[0] is None:
            print("[DEBUG] ERROR: optimizationParams[0] is None!", flush=True)
            return False
        if settings[0] is None:
            print("[DEBUG] ERROR: settings[0] is None!", flush=True)
            return False
        if symbols is None or symbols[0] is None:
            print("[DEBUG] ERROR: symbols is None!", flush=True)
            return False
        if accountInfo[0] is None:
            print("[DEBUG] ERROR: accountInfo[0] is None!", flush=True)
            return False
        if testSettings is None or testSettings[0] is None:
            print("[DEBUG] ERROR: testSettings is None!", flush=True)
            return False
        if ratesInfoArray is None or ratesInfoArray[0] is None:
            print("[DEBUG] ERROR: ratesInfoArray is None!", flush=True)
            return False
        if ratesArray is None or ratesArray[0] is None:
            print("[DEBUG] ERROR: ratesArray is None!", flush=True)
            return False
        if optimizationUpdate_c is None:
            print("[DEBUG] ERROR: optimizationUpdate_c callback is None!", flush=True)
            return False
        if optimizationFinished_c is None:
            print("[DEBUG] ERROR: optimizationFinished_c callback is None!", flush=True)
            return False
        print("[DEBUG] All parameters validated successfully", flush=True)
        
        import sys
        sys.stdout.flush()

        print("[DEBUG] Calling runOptimizationMultipleSymbols NOW...", flush=True)
        # CRITICAL: Verify numCores value right before passing to C function
        print("[DEBUG] RIGHT BEFORE C CALL: numCores = %d (type: %s)" % (numCores, type(numCores).__name__), flush=True)
        print("[DEBUG] RIGHT BEFORE C CALL: c_int(numCores) = %d" % c_int(numCores).value, flush=True)
        print(f"[DEBUG] RIGHT BEFORE C CALL: settings[0][3] (AUTOBBS_RISK_CAP) = {settings[0][3]}", flush=True)
        print(f"[DEBUG] RIGHT BEFORE C CALL: numOptimizationParams[0] = {numOptimizationParams[0]}", flush=True)
        if numOptimizationParams[0] > 0:
            for p in range(numOptimizationParams[0]):
                param_idx = optimizationParams[0][p].index
                print(f"[DEBUG] RIGHT BEFORE C CALL: optimizationParams[0][{p}]: index={param_idx}, start={optimizationParams[0][p].start}, step={optimizationParams[0][p].step}, stop={optimizationParams[0][p].stop}", flush=True)
                if param_idx == 3:
                    print(f"[DEBUG] *** RIGHT BEFORE C CALL: Found AUTOBBS_RISK_CAP in optimizationParams[0][{p}]: index=3, range=({optimizationParams[0][p].start}, {optimizationParams[0][p].step}, {optimizationParams[0][p].stop}) ***", flush=True)
        sys.stdout.flush()
        # Write to file to ensure we capture the call
        with open("debug_optimization.txt", "w") as dbg:
            dbg.write("About to call runOptimizationMultipleSymbols\n")
            dbg.flush()
        try:
            result = astdll.runOptimizationMultipleSymbols (
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
            )
            print("[DEBUG] runOptimizationMultipleSymbols returned:", result, flush=True)
            print("[DEBUG] error_c.value:", error_c.value if error_c.value else "None", flush=True)
            sys.stdout.flush()
            # Write to file
            with open("debug_optimization.txt", "a") as dbg:
                dbg.write(f"Function returned: {result}\n")
                dbg.write(f"Error: {error_c.value if error_c.value else 'None'}\n")
                dbg.flush()
        except Exception as e:
            print(f"[DEBUG] EXCEPTION in runOptimizationMultipleSymbols: {e}", flush=True)
            import traceback
            traceback.print_exc()
            sys.stdout.flush()
            # Write exception to file
            with open("debug_optimization.txt", "a") as dbg:
                dbg.write(f"EXCEPTION: {e}\n")
                traceback.print_exc(file=dbg)
                dbg.flush()
            result = 0
        
        # Always write this
        with open("debug_optimization.txt", "a") as dbg:
            dbg.write("AFTER function call - checking result\n")
            dbg.flush()
        print("[DEBUG] AFTER runOptimizationMultipleSymbols call - checking result...", flush=True)
        sys.stdout.flush()
        
        if not result:
            print("Error executing framework: " + str(error_c.value))
        else:
            if execUnderMPI == True:
                lines = comm.gather(lines, root = 0)
                if rank == 0:
                    print("Waiting results from other MPI instances")
                    for i in range(size):
                        print("Getting data from instance %d" % (i))
                        f.write(lines[i])
                    print("Optimization finished!")

        elapsed = (time() - start)

        f.close()

    else:
        print("Running test...")
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
                # Encode accountCurrency to bytes for C function (keep string version for Python operations)
                accountCurrencyBytes = accountCurrency.encode('utf-8')
                result = astdll.runPortfolioTest (
                    c_int(1),ctypes.pointer(_settings), ctypes.pointer(_symbols), accountCurrencyBytes, brokerName, refBrokerName,
                    ctypes.pointer(_accountInfo), ctypes.pointer(_testSettings), ctypes.pointer(_ratesInfoArray), c_int(numCandles),
                    c_int(1), ctypes.pointer(_ratesArray), c_double(minLotSize), testUpdate_c, None, signalUpdate_c
                )
                if result.testId == 0:
                    print("Error executing framework: " + str(error_c.value))
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
            # Encode accountCurrency to bytes for C function (keep string version for Python operations)
            accountCurrencyBytes = accountCurrency.encode('utf-8')
            result = astdll.runPortfolioTest (
                c_int(1), ctypes.pointer(settings), ctypes.pointer(symbols), accountCurrencyBytes, brokerName, refBrokerName, ctypes.pointer(accountInfo),
                ctypes.pointer(testSettings), ctypes.pointer(ratesInfoArray), c_int(numCandles), c_int(numSystemsInPortfolio), ctypes.pointer(ratesArray), c_double(minLotSize),
                testUpdate_c, None, signalUpdate_c
            )
            print("finished executing")
            if result.testId == 0:
                print("Error executing framework: " + str(error_c.value))
            else:
                print("finished trying to execute testFinished")
                testFinished(result)
            f.close()
            systems = []
            for i in range(numSystemsInPortfolio):
                systems.append(strategies[strategyIDs[i]] + ' - ' + symbols[i])
            plotPortfolioTestResult(outputFile + ".txt", systemNames = systems)
            csvToHTML(outputFile + ".txt", outputFile + ".htm", result, outputFile + ".png")

        elapsed = (time() - start)

    if execUnderMPI == False or (execUnderMPI == True and rank ==0):
        print("Time elapsed:", str(datetime.timedelta(seconds = elapsed)))

    print("total wall time: {}".format((datetime.datetime.now()-main_start_time).total_seconds()))


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
        print("No shared library loading support for OS %s" % (system))
        return False

    ASKFrameworkVersion = getASKFrameworkVersion(asfdll)
    ASTFrameworkVersion = getASTFrameworkVersion(astdll)

    print("AsirikuyFrameworkAPI v"+ASKFrameworkVersion)
    print("CTesterFrameworkAPI v"+ASTFrameworkVersion)
    print("New Strategy Tester v" + VERSION)
    quit()


def optimizationUpdate(testResults, settings, numSettings):
    global f, paramNames, iterationNumber, execUnderMPI, rank, lines, optimizationUpdateLock
    
    try:
        # CRITICAL: Flush immediately to ensure output appears
        print("[DEBUG] optimizationUpdate CALLED: testResults.totalTrades=%d, finalBalance=%lf" % (testResults.totalTrades, testResults.finalBalance), flush=True)
        sys.stdout.flush()
        
        parameters = []
        lines = ""

        if testResults.maxDDDepth != 0:
            ratio = testResults.cagr/testResults.maxDDDepth
        else:
            ratio = 0

        for i in range(numSettings):
            parameters.append("%s=%lf" % (paramNames[int(settings[i*2])], settings[i*2+1]))

        # Thread-safe: Use lock to protect shared state (iterationNumber and file writes)
        # All file I/O and shared variable access must be inside the lock
        with optimizationUpdateLock:
            if execUnderMPI == False:
                iterationNumber+=1
                print("Iteration %d finished" % (iterationNumber), flush=True)
                sys.stdout.flush()
                
                line = "%d,%s,%d,%lf,%lf,%d,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%s\n" % (iterationNumber, testResults.symbol, testResults.totalTrades, testResults.finalBalance-initialBalance, testResults.maxDDDepth,
                                                                                   int(testResults.maxDDLength/60/60/24), float(testResults.pf), testResults.r2, testResults.ulcerIndex, testResults.sharpe, testResults.cagr, ratio,
                                                                                   testResults.numShorts, testResults.numLongs, " ".join(parameters))
                print("[DEBUG] optimizationUpdate: Writing line to CSV: %s" % line.strip(), flush=True)
                sys.stdout.flush()
                f.write(line)
                f.flush()  # Ensure data is written immediately
                print("[DEBUG] optimizationUpdate: Line written and flushed (iteration %d)" % iterationNumber, flush=True)
                sys.stdout.flush()
            else:
                # MPI mode - line construction (file write happens later in MPI code)
                line = "%d,%s,%d,%lf,%lf,%d,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%s\n" % (iterationNumber, testResults.symbol, testResults.totalTrades,testResults.finalBalance-initialBalance, testResults.maxDDDepth,
                                                                                   int(testResults.maxDDLength/60/60/24), float(testResults.pf), testResults.r2, testResults.ulcerIndex, testResults.sharpe, testResults.cagr, ratio,
                                                                                   testResults.numShorts, testResults.numLongs, " ".join(parameters))
        
        # MPI-specific code (outside lock, but MPI handles its own synchronization)
        if execUnderMPI == True:
            if rank == 1:
                iterationNumber+=1
                iterationString = "%d," % (iterationNumber)
                lines = iterationString + line
                print("Iteration %d finished, %d" % (iterationNumber, rank), flush=True)
                for i in range(2, size):
                    iterationNumber+=1
                    iterationString = "%d," % (iterationNumber)
                    lines = lines + iterationString + comm.recv(source=i, tag=11)
                    print("Iteration %d finished, %d" % (iterationNumber, rank), flush=True)
                f.write(lines)
                f.flush()
            else:
                comm.send(line, dest=1, tag=11)
    except Exception as e:
        # CRITICAL: Catch and log any exceptions to prevent silent failures
        print("[ERROR] optimizationUpdate callback failed with exception: %s" % str(e), flush=True)
        import traceback
        traceback.print_exc()
        sys.stdout.flush()
        sys.stderr.flush()

def optimizationFinished():
    if execUnderMPI == False:
        print("Optimization finished!!")

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

    print(' ')

    with open(configFilePath, 'r') as fin:
        print(fin.read())

    print(' ')
    print('Test finished!!')
    print('Total trades: %d' % (testResult.totalTrades))
    print('Longs: %d Shorts: %d' % (testResult.numLongs, testResult.numShorts))
    print('Final balance: %.2lf$' % (testResult.finalBalance))
    print('Max DD: %.2lf$' % (testResult.maxDDDepth))
    print('Max DD Length: %d' % (int(testResult.maxDDLength/60/60/24)))
    print('PF: %.2lf' % (testResult.pf))
    print('R2: %.2lf' % (testResult.r2))
    print('Ulcer Index: %.2lf' % (testResult.ulcerIndex))

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
            print('Stopping optimization...')
            sys.stdout.flush()
            astdll.stopOptimization()
        elif config.getint("optimization", "optimizationType") == OPTI_GENETIC:
            print('The optimization will stop when then current generation is finished')
            sys.stdout.flush()
            astdll.stopOptimization()

##################################
###           MAIN           ####
##################################

if __name__ == "__main__": main()
