package PortfolioResult.PortfolioResult;

import java.io.IOException;
import java.time.*;
import java.time.temporal.ChronoUnit;
import java.util.*;
import java.util.stream.Collectors;

import model.*;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import service.FileService;
import service.FileServiceImpl;
import service.MT4ConversionService;
import service.MT4ConversionServiceImpl;
import service.PortfolioOptimizer;
import service.PortfolioOptimizerImpl;
import service.RateAdjustmentService;
import service.RateAdjustmentServiceImpl;
import service.StatisticsService;
import service.StatisticsServiceImpl;

/**
 * Portfolio Result Analysis Application
 * 
 * <p>This is the main entry point for the AutoBBS portfolio analysis and optimization system.
 * It provides functionality for:</p>
 * <ul>
 *   <li>Running portfolio simulations with specified risk allocations</li>
 *   <li>Optimizing portfolio risk allocations using brute-force grid search</li>
 *   <li>Validating trading results against MT4 platform</li>
 *   <li>Converting and merging historical rate data between formats</li>
 *   <li>Adjusting timezone data for various currency pairs and commodities</li>
 * </ul>
 * 
 * <p><strong>Command-line modes:</strong></p>
 * <ul>
 *   <li><code>run &lt;riskFile&gt; &lt;checkOrders&gt; [startDate]</code> - Run portfolio simulation</li>
 *   <li><code>runCustom &lt;pair&gt; &lt;...params&gt; [startDate]</code> - Run custom pair configuration</li>
 *   <li><code>optimizer &lt;riskFile&gt; &lt;predefinedFile&gt; &lt;factorsFile&gt; [startDate]</code> - Optimize portfolio</li>
 *   <li><code>optimizerLevel2</code> - Run second-level optimization</li>
 *   <li><code>MT4Rate &lt;mt4File&gt; &lt;ntsFile&gt;</code> - Convert MT4 rates to NTS format</li>
 *   <li><code>MT4RateMerge &lt;mt4File&gt; &lt;newMt4File&gt; &lt;errorFile&gt; &lt;timeFrame&gt;</code> - Merge MT4 rate files</li>
 *   <li><code>XAGUSD_60M, USTEC_60M, EURUSD_5M, EURGBP_5M</code> - Adjust rates for specific pairs</li>
 * </ul>
 * 
 * @version 1.0
 * @since 2021
 */
public class App {
	private static final Logger logger = LogManager.getLogger(App.class);
	
	// Service dependencies
	private static final ModelDataService model = new ModelDataServiceImpl();
	private static final StatisticsService statisticsService = new StatisticsServiceImpl(model);
	private static final FileService fileService = new FileServiceImpl(model, statisticsService);
	private static final ConfigReader configReader = new ConfigReader();
	private static final RateAdjustmentService rateAdjustmentService = new RateAdjustmentServiceImpl(model, fileService);
	private static final MT4ConversionService mt4ConversionService = new MT4ConversionServiceImpl(fileService);
	private static final PortfolioOptimizer portfolioOptimizer = new PortfolioOptimizerImpl(model, statisticsService);
	
	// Constants
	private static final double PRICE_TOLERANCE = 0.005; // 0.5% tolerance
	private static final double LOTS_TOLERANCE = 0.3; // 30% tolerance
	private static final String MANUAL_STRATEGY_ID = "0";
    public static void main( String[] args ) throws IOException, com.opencsv.exceptions.CsvValidationException {

		Map<String,Double> sRisks = new HashMap<>();
		Map<String,Double> predefinedStrategies = new HashMap<>();
		Map<String,Double> factors = new HashMap<>();
    	/*
    	 *  optimizer
    	 *  run
    	 */

    	if(args[0].equals("run")) {
			sRisks = fileService.readPortfolioRisk(configReader.getPropValues("PortfolioRisk_Location") + args[1]);
			boolean isCheckingOpenOrder = Boolean.parseBoolean(args[2]);

			LocalDate startDate = LocalDate.of(2000,1,1);
			if(args.length == 4) {
				startDate = LocalDate.parse(args[3]);
			}
			model.setStartDate(Date.from(startDate.atStartOfDay(ZoneId.systemDefault()).toInstant()));

			run(sRisks, false,isCheckingOpenOrder);
		}else if (args[0].equals("runCustom")) {
			LocalDate startDate = LocalDate.of(2000,1,1);
			if(args.length == 5) {
				startDate = LocalDate.parse(args[4]);
			}
			model.setStartDate(Date.from(startDate.atStartOfDay(ZoneId.systemDefault()).toInstant()));
			runCustomPairStrategy(args[1]);
		} else if (args[0].equals("optimizer")) {
			sRisks = fileService.readPortfolioRisk(configReader.getPropValues("PortfolioRisk_Location") + args[1]);
			predefinedStrategies = fileService.readPortfolioRisk(configReader.getPropValues("PortfolioRisk_Location") + args[2]);
			factors = fileService.readPortfolioRisk(configReader.getPropValues("PortfolioRisk_Location") + args[3]);

			LocalDate startDate = LocalDate.of(2000,1,1);
			if(args.length == 5) {
				startDate = LocalDate.parse(args[4]);
			}
			model.setStartDate(Date.from(startDate.atStartOfDay(ZoneId.systemDefault()).toInstant()));
			model.setFactors(factors);

			run_optimizer(sRisks,predefinedStrategies);
		} else if (args[0].equals("optimizerLevel2")) {
			runOptimizerLevel2();
		}
    	else if(args[0].equals("XAGUSD_60M")){
			String baseDir = "C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\XAGUSD_60M\\";
			rateAdjustmentService.adjustXAGUSD_60M(baseDir);
		} else if(args[0].equals("USTEC_60M")){
			String baseDir = "C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\USTECUSD_60M\\";
			rateAdjustmentService.adjustUSTECUSD_60M(baseDir);
		} else if(args[0].equals("EURUSD_5M")){
			String baseDir = "C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURUSD 5M\\";
			rateAdjustmentService.adjustEURUSD_5M(baseDir);
		} else if(args[0].equals("EURGBP_5M")){
			String baseDir = "C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURGBP 5M\\";
			rateAdjustmentService.adjustEURGBP_5M(baseDir);
		} else if(args[0].equals("MT4Rate")){
			String mt4File = configReader.getPropValues("PortfolioOpenOrders_Location") + args[1];
			String ntsFile = configReader.getPropValues("PortfolioOpenOrders_Location") + args[2];
			logger.info("Converting MT4 rate file from {} to {}", mt4File, ntsFile);
			mt4ConversionService.convertMT4ToNTS(mt4File, ntsFile);
		} else if(args[0].equals("MT4RateMerge")){
			String mt4File = configReader.getPropValues("PortfolioOpenOrders_Location") + args[1];
			String newMt4File = configReader.getPropValues("PortfolioOpenOrders_Location") + args[2];
			String errorMt4File = configReader.getPropValues("PortfolioOpenOrders_Location") + args[3];
			int timeFrame = Integer.parseInt(args[4]);
			logger.info("Merging MT4 rate files: {} + {} -> {}", mt4File, newMt4File, mt4File);
			mt4ConversionService.mergeMT4Rates(mt4File, newMt4File, errorMt4File, timeFrame);
		}

    }

    /**
	 * Validates today's trading orders against MT4 platform.
	 * 
	 * <p>Compares open and closed orders from portfolio testing against actual MT4 orders.
	 * Validates order parameters including lots, prices, stop loss, and take profit.
	 * Writes mismatched orders to files for manual review.</p>
	 * 
	 * @throws IOException if file operations fail
	 */
    private static void todayOrderChecking() throws IOException {
    	double equityMT4 = fileService.readEquityCSV(configReader.getPropValues("PortfolioOpenOrders_Location") + "Equity.csv");
		Instant previousClosedDate = ZonedDateTime.now().truncatedTo(ChronoUnit.DAYS).minusDays(1).toInstant();
		//Instant currentDate = ZonedDateTime.now().truncatedTo(ChronoUnit.DAYS).toInstant();

		List<Results> openOrdersResultsFromTesting = model.getAdjustedData(true).stream()
				.filter(d->d.profit == 0 && d.closePrice == 0)
				.collect(Collectors.toList());
		openOrdersResultsFromTesting.forEach(o->o.lots = o.lots/(o.balance/equityMT4));

		List<Results> openOrdersResultsFromMT4 = fileService.readOpenOrdersCSV(configReader.getPropValues("PortfolioOpenOrders_Location") + "OpenOrders.csv");

		ordersResultsFromTesting(openOrdersResultsFromTesting,openOrdersResultsFromMT4,"misMatchedResult",false);
		//Compare last two days orders

		List<Results> closedOrdersResultsFromTesting = model.getAdjustedData(true).stream().filter(d->d.profit != 0 && d.closeTime.toInstant().isAfter(previousClosedDate) ).collect(Collectors.toList());
		closedOrdersResultsFromTesting.forEach(o->o.lots = o.lots/(o.balance/equityMT4));

		List<Results> closedOrdersResultsFromMT4 =
				fileService.readClosedOrdersCSV(configReader.getPropValues("PortfolioOpenOrders_Location") + "ClosedOrders.csv")
				.stream().filter(d->d.closeTime.toInstant().isAfter(previousClosedDate)).collect(Collectors.toList());

		ordersResultsFromTesting(closedOrdersResultsFromTesting,closedOrdersResultsFromMT4,"misMatchedClosedResult",true);

	}

	/**
	 * Compares testing results against MT4 results and identifies mismatches.
	 * 
	 * <p>Groups orders by strategy ID and compares:
	 * <ul>
	 *   <li>Order counts per strategy</li>
	 *   <li>Order parameters (lots, prices, stop loss, take profit)</li>
	 *   <li>Order types (buy/sell)</li>
	 * </ul>
	 * 
	 * <p>Manual orders (strategy ID "0") are flagged but not compared.</p>
	 * 
	 * @param ordersResultsFromTesting Orders generated by portfolio testing
	 * @param ordersResultsFromMT4 Orders reported by MT4 platform
	 * @param fileName Output file name for mismatched orders
	 * @param isClosedOrder True if comparing closed orders, false for open orders
	 * @throws IOException if file write operations fail
	 */
    private static void ordersResultsFromTesting(List<Results> ordersResultsFromTesting, List<Results> ordersResultsFromMT4, String fileName, boolean isClosedOrder) throws IOException {
		//Checking
		// 1. group by strategy ID
		Map<String, List<Results>> OpenOrdersGroupsFromMT4 =
				ordersResultsFromMT4.stream().collect(Collectors.groupingBy(d->d.strategyID))
						.entrySet().stream().sorted(Map.Entry.comparingByKey())
						.collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue,
								(oldValue, newValue) -> oldValue, LinkedHashMap::new));

		Map<String, List<Results>> openOrdersGroupsFromTesting =
				ordersResultsFromTesting.stream().collect(Collectors.groupingBy(d->d.strategyID))
						.entrySet().stream().sorted(Map.Entry.comparingByKey())
						.collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue,
								(oldValue, newValue) -> oldValue, LinkedHashMap::new));

		Map<String,String> misMatchedOrderGroups = new HashMap<>();
		List<String> matchedStrategy = new ArrayList<>();

		for (Map.Entry<String, List<Results>> orderGroup : OpenOrdersGroupsFromMT4.entrySet()) {
			if (MANUAL_STRATEGY_ID.equals(orderGroup.getKey())) {
				misMatchedOrderGroups.put(orderGroup.getKey(), "Manual orders are not generated from testing!");
				continue;
			}

			if(openOrdersGroupsFromTesting.containsKey(orderGroup.getKey())){
				List<Results> testingResults = openOrdersGroupsFromTesting.get(orderGroup.getKey());
				//Comparing order group

				if(orderGroup.getValue().size() != testingResults.size()){
					misMatchedOrderGroups.put(orderGroup.getKey(),"Order size misMatched!");
					continue;
				}

				//only keep the biggest lots order

				Results maxOrderFromMT4 = orderGroup.getValue().stream().max(Comparator.comparing(r->r.lots)).orElseThrow(NoSuchElementException::new);
				Results maxOrderFromTesting = testingResults.stream().max(Comparator.comparing(r->r.lots)).orElseThrow(NoSuchElementException::new);

				if(compareOrders(maxOrderFromTesting,maxOrderFromMT4,misMatchedOrderGroups,isClosedOrder))
					matchedStrategy.add(orderGroup.getKey());

			}
			else{
				misMatchedOrderGroups.put(orderGroup.getKey(),String.format("strategy %s doesn't generated orders from testing! Details:\n%s",
						orderGroup.getKey(),
						orderGroup.getValue().stream().map(Results::toString).collect(Collectors.joining("\n"))));
			}
		}

		for (Map.Entry<String, List<Results>> orderGroup : openOrdersGroupsFromTesting.entrySet()) {
			if(matchedStrategy.contains(orderGroup.getKey()) || misMatchedOrderGroups.containsKey(orderGroup.getKey()))
				continue;
			if(!OpenOrdersGroupsFromMT4.containsKey(orderGroup.getKey())){
				misMatchedOrderGroups.put(orderGroup.getKey(),String.format("strategy %s doesn't generated orders from MT4! Details:\n%s",
						orderGroup.getKey(),
						orderGroup.getValue().stream().map(Results::toString).collect(Collectors.joining("\n"))));
			}
		}

		fileService.writeMismatchedFile(configReader.getPropValues("PortfolioResult_Location") + fileName,misMatchedOrderGroups);

	}

	/**
	 * Checks if two prices match within tolerance.
	 * 
	 * @param testingPrice Price from testing results
	 * @param mt4Price Price from MT4
	 * @return True if prices are within PRICE_TOLERANCE (0.5%)
	 */
	private static boolean isMatchedPrice(double testingPrice, double mt4Price) {
		return Math.abs(testingPrice - mt4Price) <= mt4Price * PRICE_TOLERANCE;
	}

	/**
	 * Checks if two lot sizes match within tolerance.
	 * 
	 * @param testingLots Lot size from testing results
	 * @param mt4Lots Lot size from MT4
	 * @return True if lot sizes are within LOTS_TOLERANCE (30%)
	 */
	private static boolean isMatchedLots(double testingLots, double mt4Lots) {
		return Math.abs(testingLots - mt4Lots) <= mt4Lots * LOTS_TOLERANCE;
	}

	/**
	 * Compares individual orders from testing and MT4, recording any mismatches.
	 * 
	 * <p>Validates:
	 * <ul>
	 *   <li>Strategy ID must match</li>
	 *   <li>Order type (buy/sell) must match</li>
	 *   <li>Open price must match within tolerance</li>
	 *   <li>Close price must match within tolerance (for closed orders)</li>
	 *   <li>Lot size must match within tolerance</li>
	 *   <li>Stop loss price must match within tolerance</li>
	 *   <li>Take profit price must match within tolerance</li>
	 * </ul>
	 * 
	 * @param orderFromTesting Order from portfolio testing
	 * @param orderFromMT4 Order from MT4 platform
	 * @param misMatched Map to store mismatch descriptions
	 * @param isClosedOrder True to validate close price, false to skip
	 * @return True if all validations pass, false otherwise
	 */
	private static boolean compareOrders(Results orderFromTesting, Results orderFromMT4, Map<String, String> misMatched, boolean isClosedOrder) {
		if(!orderFromTesting.strategyID.equals(orderFromMT4.strategyID)) {
			misMatched.put(orderFromMT4.strategyID, "unMatched strategy ID.");
			return false;
		}

		if(!orderFromTesting.orderType.equals(orderFromMT4.orderType)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched order type: testing open price %s-> MT4 open price %s", orderFromTesting.orderType, orderFromMT4.orderType));
			return false;
		}

		if(!isMatchedPrice(orderFromTesting.openPrice,orderFromMT4.openPrice)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched open price: testing open price %.5f-> MT4 open price %.5f", orderFromTesting.openPrice, orderFromMT4.openPrice));
			return false;
		}

		if(isClosedOrder && !isMatchedPrice(orderFromTesting.closePrice,orderFromMT4.closePrice)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched close price: testing close price %.5f-> MT4 close price %.5f", orderFromTesting.closePrice, orderFromMT4.closePrice));
			return false;
		}

		//double level = orderFromTesting.balance / orderFromMT4.balance;
		if(!isMatchedLots(orderFromTesting.lots,orderFromMT4.lots)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched lots: testing lots %.3f->MT4 lots %.3f", orderFromTesting.lots, orderFromMT4.lots));
			return false;
		}

		if(!isMatchedPrice(orderFromTesting.stopLossPrice,orderFromMT4.stopLossPrice)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched stopLossPrice: testing stopLossPrice %.5f-> MT4 stopLossPrice %.5f", orderFromTesting.stopLossPrice, orderFromMT4.stopLossPrice));
			return false;
		}

		if(!isMatchedPrice(orderFromTesting.takeProfitPrice,orderFromMT4.takeProfitPrice)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched takeProfitPrice: testing takeProfitPrice %.5f-> MT4 takeProfitPrice %.5f", orderFromTesting.takeProfitPrice, orderFromMT4.takeProfitPrice));
			return false;
		}

		return true;
	}

	/**
	 * Convenience method that runs portfolio simulation without order checking.
	 * 
	 * @param sRisks Strategy risk allocations (strategy ID -> risk multiplier)
	 * @param fromOptimized True if results are from optimization, false otherwise
	 */
	private static void run(Map<String, Double> sRisks, boolean fromOptimized) {
    	run(sRisks, fromOptimized, false);
	}

	/**
	 * Runs portfolio simulation with specified risk allocations.
	 * 
	 * <p>Process:
	 * <ol>
	 *   <li>Initialize model with risk allocations</li>
	 *   <li>Load historical trading results for each strategy</li>
	 *   <li>Generate portfolio results (unless from optimizer)</li>
	 *   <li>Optionally validate against MT4 orders</li>
	 *   <li>Generate weekly and monthly reports</li>
	 *   <li>Calculate comprehensive statistics</li>
	 *   <li>Write results to CSV files</li>
	 * </ol>
	 * 
	 * @param sRisks Strategy risk allocations (strategy ID -> risk multiplier)
	 * @param fromOptimized True if called from optimizer (skips portfolio result generation)
	 * @param isCheckingOpenOrder True to validate orders against MT4
	 */
    private static void run(Map<String,  Double> sRisks,  boolean fromOptimized,  boolean isCheckingOpenOrder)  {
    	model.initModelData(sRisks, true);
    	
		try {
			// Load historical results for all strategies
			String resultsLocation = configReader.getPropValues("PortfolioResult_Location");
			for (String strategyId : sRisks.keySet()) {
				String resultFile = resultsLocation + String.format("results_%s.txt", strategyId);
				fileService.readCSV(resultFile, strategyId);
			}

			if (!fromOptimized) {
				fileService.writeCVS(resultsLocation + "portfolioResult_adjusted", true);
			}

			if (isCheckingOpenOrder) {
				todayOrderChecking();
			}
			
		// Generate weekly and monthly reports
		fileService.generateWeelyReport(resultsLocation + "portfolioWeeklyReport_adjusted", true);
		fileService.generateMonthlyReport(resultsLocation + "portfolioMonthlyReport_adjusted", true);
		
		statisticsService.calculate(ModelDataServiceImpl.INITBALANCE, true);			// Write statistics
			fileService.writeStatiscsListCVS(resultsLocation + "portfolioStatistics_adjusted", false);
			logger.info("Portfolio simulation completed successfully");
			
		} catch (IOException e) {
			logger.error("Portfolio simulation failed: {}", e.getMessage(), e);
			throw new RuntimeException("Failed to run portfolio simulation", e);
		}
    }

	/**
	 * Runs portfolio simulation for predefined custom trading pair configurations.
	 * 
	 * <p>This method contains hardcoded strategy combinations for specific trading pairs:
	 * US500, GBPAUD, GBPJPY, XAUUSD, XAUUSD_5M, BTCUSD, and Limit orders.</p>
	 * 
	 * <p>Note: Risk allocations are specific to historical optimization results and
	 * should be reviewed before live trading.</p>
	 * 
	 * @param pair Trading pair identifier (e.g., "US500", "GBPJPY", "XAUUSD")
	 */
	private static void runCustomPairStrategy(String pair) {
		Map<String,Double> sRisks = new HashMap<String,Double>();
    	if("US500".equals(pair)) { //it is ok for now,but not for long term.
			sRisks.put("900001", 0.5*1.0/1.0);
			sRisks.put("500001", 1.0/1.0);
			runCustom(sRisks,"400001");
		}
		else if("GBPAUD".equals(pair)) {
			sRisks.put("860012", 0.5/1.0); //it doesn't help with portfolio.
			sRisks.put("500002", 1.0/1.0);
			runCustom(sRisks,"400002");
		}
		else if("GBPJPY".equals(pair)) {
			sRisks.put("860006", 1.0/1.0);
			sRisks.put("841005", 1.0/1.0);
			sRisks.put("860002", 0.7*1.0/1.0);
			runCustom(sRisks,"400003");
		}
		else if("XAUUSD".equals(pair)) {
			sRisks.put("860007", 1.0/1.0);
			sRisks.put("842001", 0.7*1.0/1.0);
			sRisks.put("860003", 0.5*1.0/1.0);
			runCustom(sRisks,"400004");
		}
		else if("XAUUSD_5M".equals(pair)) {
			sRisks.put("860001", 0.5*1.0/1.0);
			sRisks.put("860003", 0.5*1.0/1.0);
			runCustom(sRisks,"400005");
		}
		else if("BTCUSD".equals(pair)) {
			sRisks.put("300002", 0.6*1.0/1.0);
			sRisks.put("200002", 1*1.0/1.0);
			runCustom(sRisks,"400002");
		}
		else if("Limit".equals(pair)) {
			//sRisks.put("200002", 1.0/1.0);
			sRisks.put("200003", 1.0/1.0);
			sRisks.put("200005", 1.0/1.0);
			sRisks.put("200007", 1.0/1.0);
			sRisks.put("200009", 1.0/1.0);
			runCustom(sRisks,"220000");
		}

	}

	/**
	 * Runs portfolio simulation and generates a combined strategy result file.
	 * 
	 * <p>Similar to run() but additionally exports results as a new single strategy
	 * with the specified ID. This allows treating a portfolio as a meta-strategy.</p>
	 * 
	 * @param sRisks Strategy risk allocations (strategy ID -> risk multiplier)
	 * @param newStrategyId ID for the combined portfolio strategy result file
	 */
    private static void runCustom(Map<String, Double> sRisks, String newStrategyId) {

    	model.initModelData(sRisks,true);
    	
		try {

			sRisks.keySet().stream().forEach(id-> {
				try {
					fileService.readCSV(configReader.getPropValues("PortfolioResult_Location") + String.format("results_%s.txt",id),id);
				} catch (IOException e) {
					e.printStackTrace();
				}
			});


			fileService.writeCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioResult_adjusted",true);
			fileService.writeNewStrategyCVS(configReader.getPropValues("PortfolioResult_Location") + "results_"+newStrategyId,true);


			//generate weekly and monthly report
			fileService.generateWeelyReport(configReader.getPropValues("PortfolioResult_Location") + "portfolioWeeklyReport_adjusted",true);
			fileService.generateMonthlyReport(configReader.getPropValues("PortfolioResult_Location") + "portfolioMonthlyReport_adjusted",true);

			statisticsService.calculate(ModelDataServiceImpl.INITBALANCE,true);

			//Write all statiscs
			fileService.writeStatiscsListCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioStatistics_adjusted",false);

		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }

	/**
	 * Performs 5-dimensional portfolio optimization with reduced risk multipliers.
	 * 
	 * <p>Similar to run_optimizer() but uses:
	 * <ul>
	 *   <li>5 strategies instead of 6</li>
	 *   <li>5 risk multipliers (0.2, 0.4, 1.0, 1.4, 1.8) instead of 7</li>
	 *   <li>Total combinations: 3,125 (5^5) vs 117,649 (7^6)</li>
	 * </ul>
	 * 
	 * <p>This provides faster optimization at the cost of less granularity.
	 * Used for quick iterations or when computational resources are limited.</p>
	 */
	private static void runOptimizerLevel2() {
		Map<String,Double> sRisks = new HashMap<>();

		// Expected risk / Based risk
		// Base case: 1
		sRisks.put("500002", 1.0/1.0);
		sRisks.put("400003", 1.0/1.0);
		sRisks.put("400004", 1.0/1.0);
		sRisks.put("860011", 1.0/1.0);
		sRisks.put("900002", 1.0/1.0);

		List<String> strategy = sRisks.keySet().stream().sorted().collect(Collectors.toList());


		//List<Double> run = new ArrayList<>(  Arrays.asList(0.2,0.5,0.8,1.0,1.2,1.5,1.8));
		List<Double> run = new ArrayList<>(  Arrays.asList(0.2,0.4,1.0,1.4,1.8));

		model.initModelData(sRisks,true);

		try {
			sRisks.keySet().stream().forEach(id-> {
				try {
					fileService.readCSV(configReader.getPropValues("PortfolioResult_Location") + String.format("results_%s.txt",id),id);
				} catch (IOException e) {
					e.printStackTrace();
				}
			});

			fileService.writeCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioResult_adjusted.csv",true);

			//base case:
			statisticsService.calculate(ModelDataServiceImpl.INITBALANCE,true);

			//Brutal optimizer
			sRisks.clear();

			for (int i1 = 0; i1 < run.size(); i1++) {
				sRisks.put(strategy.get(0), run.get(i1));
				for (int i2 = 0; i2 < run.size(); i2++) {
					sRisks.put(strategy.get(1), run.get(i2));
					for (int i3 = 0; i3 < run.size(); i3++) {
						sRisks.put(strategy.get(2), run.get(i3));
						for (int i4 = 0; i4 < run.size(); i4++) {
							sRisks.put(strategy.get(3), run.get(i4));
							for (int i5 = 0; i5 < run.size(); i5++) {
								sRisks.put(strategy.get(4), run.get(i5));

								model.addStrategyList(sRisks);
								statisticsService.calculate(ModelDataServiceImpl.INITBALANCE, true);
							}

						}
					}
				}
			}


			//Write all statiscs
			fileService.writeStatiscsListCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioStatistics_optimize_adjusted.csv",true);

		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}

		List<Statistics> sList = statisticsService.selectModels(true);

	//Run the top model
	run(sList.get(0).strategyRisk,true);
}

	/**
	 * Performs brute-force portfolio optimization by testing all combinations of risk multipliers
	 * for a set of trading strategies.
	 * 
	 * <p>This method executes an N-dimensional grid search over risk multipliers (0.2 to 2.2) for
	 * strategies not in the predefinedStrategies set. Predefined strategies maintain their original
	 * risk allocations throughout the optimization process.</p>
	 * 
	 * <p>Optimization process:</p>
	 * <ol>
	 *   <li>Load historical results for all strategies</li>
	 *   <li>Calculate baseline statistics with original risk allocations</li>
	 *   <li>Test all risk multiplier combinations (7^N iterations for N strategies)</li>
	 *   <li>Select best portfolio based on Martin Ratio and other criteria</li>
	 *   <li>Re-run simulation with optimal risk allocation</li>
	 * </ol>
	 * 
	 * <p>Examples of computational cost:</p>
	 * <ul>
	 *   <li>3 strategies × 7 multipliers = 343 combinations</li>
	 *   <li>4 strategies × 7 multipliers = 2,401 combinations</li>
	 *   <li>5 strategies × 7 multipliers = 16,807 combinations</li>
	 *   <li>6 strategies × 7 multipliers = 117,649 combinations</li>
	 *   <li>7 strategies × 7 multipliers = 823,543 combinations</li>
	 * </ul>
	 * 
	 * <p>Note: This is a computationally expensive operation that scales exponentially.
	 * Consider using fewer strategies or risk levels for large portfolios.</p>
	 * 
	 * @param sRisks Initial strategy risk allocations (strategy ID -> risk multiplier).
	 *               Keys represent strategy IDs, values represent base risk multipliers.
	 *               This map is modified during optimization.
	 * @param predefinedStrategies Strategies with fixed risk allocations (strategy ID -> risk multiplier).
	 *                             These strategies are not optimized but are included in every
	 *                             portfolio configuration.
	 */
	private static void run_optimizer(Map<String,Double> sRisks, Map<String,Double> predefinedStrategies) {
		// Separate strategies to optimize from predefined ones
		List<String> strategiesToOptimize = sRisks.keySet().stream()
				.sorted()
				.filter(id -> !predefinedStrategies.keySet().contains(id))
				.collect(Collectors.toList());

		// Risk multipliers to test: 0.2x, 0.4x, 1.0x, 1.4x, 1.8x, 2.0x, 2.2x
		// Represents 20% to 220% of base risk allocation
		List<Double> riskMultipliers = new ArrayList<>(Arrays.asList(0.2, 0.4, 1.0, 1.4, 1.8, 2.0, 2.2));

		if (strategiesToOptimize.isEmpty()) {
			logger.error("No strategies to optimize. All strategies are predefined.");
			throw new IllegalArgumentException("At least one strategy must be optimizable");
		}

		model.initModelData(sRisks, true);

		try {
			// Load historical trading results for each strategy
			loadStrategyResults(sRisks.keySet());

			// Write baseline portfolio results
			String portfolioResultPath = configReader.getPropValues("PortfolioResult_Location") + "portfolioResult_adjusted.csv";
			fileService.writeCVS(portfolioResultPath, true);

			// Calculate baseline statistics
			statisticsService.calculate(ModelDataServiceImpl.INITBALANCE, true);

			// Choose optimization strategy based on portfolio size
			int totalCombinations = (int) Math.pow(riskMultipliers.size(), strategiesToOptimize.size());
			logger.info("Starting portfolio optimization: {} strategies to optimize, {} predefined",
					strategiesToOptimize.size(), predefinedStrategies.size());
			logger.info("{} risk levels per strategy = {} total combinations",
					riskMultipliers.size(), totalCombinations);

			sRisks.clear();
			
			// Auto-select algorithm based on portfolio size
			if (strategiesToOptimize.size() <= 6) {
				// Small portfolio: use exhaustive grid search (guaranteed optimal)
				logger.info("Using GRID_SEARCH (exhaustive) - optimal for {} strategies", strategiesToOptimize.size());
				portfolioOptimizer.optimizePortfolio(strategiesToOptimize, riskMultipliers, predefinedStrategies);
				// Note: grid search writes results directly to model, no need to update sRisks
			} else if (strategiesToOptimize.size() <= 10) {
				// Medium portfolio: use genetic algorithm (95-98% optimal, much faster)
				int maxEvaluations = Math.min(10000, totalCombinations);
				logger.info("Using GENETIC_ALGORITHM - efficient for {} strategies (budget: {} evaluations)",
						strategiesToOptimize.size(), maxEvaluations);
				Map<String, Double> bestAllocation = portfolioOptimizer.optimize(
						strategiesToOptimize, riskMultipliers, predefinedStrategies,
						PortfolioOptimizer.OptimizationStrategy.GENETIC_ALGORITHM,
						maxEvaluations);
				sRisks.putAll(bestAllocation);
			} else {
				// Large portfolio: use coarse-to-fine hierarchical search
				int maxEvaluations = 20000;
				logger.info("Using COARSE_TO_FINE - scalable for {} strategies (budget: {} evaluations)",
						strategiesToOptimize.size(), maxEvaluations);
				Map<String, Double> bestAllocation = portfolioOptimizer.optimize(
						strategiesToOptimize, riskMultipliers, predefinedStrategies,
						PortfolioOptimizer.OptimizationStrategy.COARSE_TO_FINE,
						maxEvaluations);
				sRisks.putAll(bestAllocation);
			}

			// Write all optimization results
			String optimizedStatsPath = configReader.getPropValues("PortfolioResult_Location") + "portfolioStatistics_optimize_adjusted.csv";
			fileService.writeStatiscsListCVS(optimizedStatsPath, true);

			logger.info("Optimization complete. Results written to {}", optimizedStatsPath);

		} catch (IOException e) {
			logger.error("Error during portfolio optimization: {}", e.getMessage(), e);
			throw new RuntimeException("Portfolio optimization failed", e);
		}

		// Select and run the best performing portfolio
		List<Statistics> optimizedModels = statisticsService.selectModels(true);
		if (optimizedModels.isEmpty()) {
			logger.error("No valid portfolio configurations found during optimization");
			return;
		}

		logger.info("Running simulation with optimal portfolio configuration");
		run(optimizedModels.get(0).strategyRisk, true);
	}

		/**
		 * Loads historical trading results for all strategies.
		 *
		 * @param strategyIds Set of strategy IDs to load results for
		 * @throws IOException if any result file cannot be read
		 */
		private static void loadStrategyResults(Set<String> strategyIds) throws IOException {
			String resultsLocation = configReader.getPropValues("PortfolioResult_Location");
			for (String strategyId : strategyIds) {
				String resultFile = resultsLocation + String.format("results_%s.txt", strategyId);
				fileService.readCSV(resultFile, strategyId);
			}
		}
}