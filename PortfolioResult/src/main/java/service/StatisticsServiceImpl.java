package service;

import java.util.List;
import java.util.OptionalDouble;
import java.util.stream.Collectors;

import javax.annotation.Nonnull;
import javax.annotation.concurrent.ThreadSafe;

import model.ModelDataService;
import model.Results;
import model.Statistics;

/**
 * Implementation of {@link StatisticsService} for calculating portfolio statistics and performance metrics.
 * Computes returns, risk metrics, ratios, and filters strategies based on optimization criteria.
 */
@ThreadSafe
public class StatisticsServiceImpl implements StatisticsService {

	// Time conversion constants
	private static final long MILLISECONDS_PER_DAY = 1000L * 60 * 60 * 24;
	private static final long MILLISECONDS_PER_WEEK = MILLISECONDS_PER_DAY * 5; // Trading week
	private static final int DAYS_PER_YEAR = 365;
	private static final int WEEKS_PER_YEAR = 52;
	
	// Statistical constants
	private static final double SHARPE_ANNUALIZATION_FACTOR = 7.2111103; // sqrt(52)
	private static final double MAX_ULCER_INDEX = 100.0;
	private static final double PERCENTAGE_MULTIPLIER = 100.0;

	private final ModelDataService model;

	/**
	 * Constructs a StatisticsServiceImpl with the given model data.
	 * 
	 * @param model  the model data containing results and configuration
	 */
	public StatisticsServiceImpl(@Nonnull ModelDataService model) {
		if (model == null) {
			throw new IllegalArgumentException("Model cannot be null");
		}
		this.model = model;
	}

	@Override
	public void calculate(double initialBalance, boolean isAdjusted) {
		if (initialBalance <= 0) {
			throw new IllegalArgumentException("Initial balance must be positive, got: " + initialBalance);
		}

		List<Results> results = model.getAdjustedData(isAdjusted);
		if (results == null || results.isEmpty()) {
			throw new IllegalStateException("No results available for statistics calculation");
		}

		Statistics statistics = new Statistics();

		// Basic trade counts
		statistics.num_trades = results.size();
		statistics.num_longs = countTradesByType(results, "BUY");
		statistics.num_shorts = countTradesByType(results, "SELL");

		// Return calculation
		statistics.totalReturn = calculateTotalReturn(results, initialBalance);

		// Win/Loss analysis
		ProfitLossMetrics plMetrics = calculateProfitLossMetrics(results);
		statistics.winning = (plMetrics.winningTrades * PERCENTAGE_MULTIPLIER) / statistics.num_trades;
		statistics.risk_reward = plMetrics.avgWinningTrade / plMetrics.avgLosingTrade;
		statistics.maxWinningTrade = plMetrics.maxWinningTrade;
		statistics.maxLosingTrade = plMetrics.maxLosingTrade;
		statistics.profit_factor = plMetrics.totalWinning / plMetrics.totalLosing;

		// Drawdown and time-based metrics
		DrawdownMetrics ddMetrics = calculateDrawdownMetrics(results);
		statistics.max_dd = ddMetrics.maxDrawdownDepth;
		statistics.max_dd_length = ddMetrics.maxDrawdownLengthDays;

		// Time period analysis
		statistics.yearsTraded = calculateYearsTraded(results);
		statistics.trades_week = statistics.num_trades / statistics.yearsTraded / WEEKS_PER_YEAR;

		// CAGR and risk-adjusted returns
		statistics.cagr = PERCENTAGE_MULTIPLIER * (Math.pow(statistics.totalReturn, 1.0 / statistics.yearsTraded) - 1.0);
		statistics.cagr_maxdd = statistics.max_dd > 0 ? statistics.cagr / statistics.max_dd : 0;

		// R-squared (coefficient of determination)
		statistics.r2 = calculateRSquared(results, statistics.num_trades);

		// Weekly statistics (Ulcer Index, Sharpe Ratio, Martin Ratio)
		calculateWeeklyStatistics(results, initialBalance, statistics);
		
		// Save completed statistics to model
		model.saveStatistics(statistics);
	}

	/**
	 * Counts trades of a specific type.
	 */
	private long countTradesByType(List<Results> results, String orderType) {
		return results.stream()
				.filter(r -> orderType.equals(r.orderType))
				.count();
	}

	/**
	 * Calculates the total return ratio.
	 */
	private double calculateTotalReturn(List<Results> results, double initialBalance) {
		double finalBalance = results.get(results.size() - 1).balance;
		return finalBalance / initialBalance;
	}

	/**
	 * Calculates years traded from first to last trade.
	 */
	private double calculateYearsTraded(List<Results> results) {
		long firstTime = results.get(0).closeTime.getTime();
		long lastTime = results.get(results.size() - 1).closeTime.getTime();
		long millisTraded = Math.abs(lastTime - firstTime);
		return (double) millisTraded / MILLISECONDS_PER_DAY / DAYS_PER_YEAR;
	}

	/**
	 * Container for profit/loss metrics.
	 */
	private static class ProfitLossMetrics {
		double totalWinning;
		double totalLosing;
		long winningTrades;
		double avgWinningTrade;
		double avgLosingTrade;
		OptionalDouble maxWinningTrade;
		OptionalDouble maxLosingTrade;
	}

	/**
	 * Calculates comprehensive profit/loss metrics.
	 */
	private ProfitLossMetrics calculateProfitLossMetrics(List<Results> results) {
		ProfitLossMetrics metrics = new ProfitLossMetrics();

		// Winning trades
		metrics.totalWinning = results.stream()
				.filter(r -> r.profit > 0)
				.mapToDouble(r -> r.profit)
				.sum();
		metrics.winningTrades = results.stream()
				.filter(r -> r.profit > 0)
				.count();
		metrics.avgWinningTrade = results.stream()
				.filter(r -> r.profit > 0)
				.mapToDouble(r -> Math.abs(r.profit / r.balance))
				.average()
				.orElse(0.0);

		// Losing trades
		metrics.totalLosing = Math.abs(results.stream()
				.filter(r -> r.profit <= 0)
				.mapToDouble(r -> r.profit)
				.sum());
		metrics.avgLosingTrade = results.stream()
				.filter(r -> r.profit < 0)
				.mapToDouble(r -> Math.abs(r.profit / r.balance))
				.average()
				.orElse(1.0); // Avoid division by zero

		// Extremes
		metrics.maxWinningTrade = results.stream()
				.mapToDouble(r -> r.profit)
				.max();
		metrics.maxLosingTrade = results.stream()
				.mapToDouble(r -> r.profit)
				.min();

		return metrics;
	}

	/**
	 * Container for drawdown metrics.
	 */
	private static class DrawdownMetrics {
		double maxDrawdownDepth;
		double maxDrawdownLengthDays;
	}

	/**
	 * Calculates maximum drawdown depth and length.
	 */
	private DrawdownMetrics calculateDrawdownMetrics(List<Results> results) {
		DrawdownMetrics metrics = new DrawdownMetrics();
		
		double maxBalance = 0.0;
		double currentDDDepth = 0.0;
		double maxDDDepth = 0.0;
		long currentDDLength = 0;
		long maxDDLength = 0;
		long ddStartTime = results.get(0).closeTime.getTime();

		for (Results result : results) {
			if (result.balance < maxBalance) {
				// In drawdown
				currentDDDepth = ((maxBalance - result.balance) / maxBalance) * PERCENTAGE_MULTIPLIER;
				currentDDLength = result.closeTime.getTime() - ddStartTime;
			} else if (result.balance > maxBalance) {
				// New high - reset drawdown
				maxBalance = result.balance;
				currentDDDepth = 0.0;
				currentDDLength = 0;
				ddStartTime = result.closeTime.getTime();
			}

			// Track maximum drawdown
			if (currentDDDepth > maxDDDepth) {
				maxDDDepth = currentDDDepth;
			}
			if (currentDDLength > maxDDLength) {
				maxDDLength = currentDDLength;
			}
		}

		metrics.maxDrawdownDepth = maxDDDepth;
		metrics.maxDrawdownLengthDays = (double) maxDDLength / MILLISECONDS_PER_DAY;
		
		return metrics;
	}

	/**
	 * Calculates R-squared (coefficient of determination) for equity curve.
	 */
	private double calculateRSquared(List<Results> results, long numTrades) {
		double firstBalance = results.get(0).balance;
		long firstTime = results.get(0).closeTime.getTime();
		
		// Calculate linear regression slope
		double sumBalanceTime = 0.0;
		double timeSqrSum = 0.0;
		double avgBalanceLog = 0.0;

		for (Results result : results) {
			long timeDiff = result.closeTime.getTime() - firstTime;
			double logBalanceChange = Math.log(result.balance) - Math.log(firstBalance);
			
			timeSqrSum += Math.pow(timeDiff, 2);
			sumBalanceTime += timeDiff * logBalanceChange;
			avgBalanceLog += logBalanceChange / numTrades;
		}

		double linearRegressionSlope = sumBalanceTime / timeSqrSum;

		// Calculate coefficient of determination
		double sumSquaredResiduals = 0.0;
		double sumSquaredTotal = 0.0;

		for (Results result : results) {
			long timeDiff = result.closeTime.getTime() - firstTime;
			double logBalanceChange = Math.log(result.balance) - Math.log(firstBalance);
			double predicted = linearRegressionSlope * timeDiff;
			
			sumSquaredResiduals += Math.pow(predicted - logBalanceChange, 2);
			sumSquaredTotal += Math.pow(logBalanceChange - avgBalanceLog, 2);
		}

		if (sumSquaredTotal == 0 || (1 - sumSquaredResiduals / sumSquaredTotal) < 0) {
			return 0.0;
		}
		
		return 1.0 - sumSquaredResiduals / sumSquaredTotal;
	}

	/**
	 * Calculates weekly-based statistics: Ulcer Index, Sharpe Ratio, and Martin Ratio.
	 * 
	 * @param results         list of trading results
	 * @param initialBalance  starting balance
	 * @param statistics      statistics object to populate
	 */
	private void calculateWeeklyStatistics(List<Results> results, double initialBalance,
			Statistics statistics) {

		long numWeeks = 0;
		int resultIndex = 0;
		double sumSquaredDrawdowns = 0.0;
		double cumWeekReturn = 0.0;
		double cumWeekReturnSquare = 0.0;
		double maxBalance = initialBalance;
		
		long analysisTime = results.get(0).closeTime.getTime();
		double startWeekBalance = initialBalance;
		double endWeekBalance = startWeekBalance;

		int numResults = results.size();
		long lastDate = results.get(numResults - 1).closeTime.getTime();

		// Iterate week by week (5 trading days)
		while (analysisTime < lastDate) {
			numWeeks++;
			analysisTime += MILLISECONDS_PER_WEEK;

			// Find the last result in this week
			while (resultIndex < numResults && results.get(resultIndex).closeTime.getTime() < analysisTime) {
				endWeekBalance = results.get(resultIndex).balance;
				resultIndex++;
			}

			// Calculate weekly return
			double weekReturn = (endWeekBalance - startWeekBalance) / startWeekBalance;
			cumWeekReturn += weekReturn;
			cumWeekReturnSquare += weekReturn * weekReturn;
			startWeekBalance = results.get(resultIndex - 1).balance;

			// Update max balance and calculate squared drawdown
			if (endWeekBalance > maxBalance) {
				maxBalance = endWeekBalance;
			} else {
				double drawdownPercent = PERCENTAGE_MULTIPLIER * ((endWeekBalance / maxBalance) - 1.0);
				sumSquaredDrawdowns += Math.pow(drawdownPercent, 2);
			}
		}

		// Ulcer Index: cap at 100 (strategies above 100 are considered too risky)
		double ulcerIndex = Math.sqrt(sumSquaredDrawdowns / numWeeks);
		statistics.ulcerIndex = Math.min(ulcerIndex, MAX_ULCER_INDEX);

		// Sharpe Ratio (annualized)
		double meanWeekly = cumWeekReturn / numWeeks;
		double sigmaWeekly = Math.sqrt(
				(numWeeks * cumWeekReturnSquare - cumWeekReturn * cumWeekReturn) 
				/ (numWeeks * (numWeeks - 1)));
		statistics.sharp_ratio = SHARPE_ANNUALIZATION_FACTOR * (meanWeekly / sigmaWeekly);

		// Martin Ratio (CAGR / Ulcer Index)
		statistics.martin = statistics.ulcerIndex > 0 
				? statistics.cagr / statistics.ulcerIndex 
				: 0.0;
	}

	/**
	 * Selects portfolio models based on optimization criteria.
	 * 
	 * When optimized filtering is enabled, applies factor-based constraints:
	 * 1. Maximum drawdown must be within acceptable range (min_max_dd to max_max_dd)
	 * 2. Ulcer Index must be below threshold (max_ulcerIndex)
	 * 3. Results sorted by Martin Ratio (CAGR / Ulcer Index) descending
	 * 
	 * @param isOptimized  whether to apply factor-based filtering
	 * @return list of statistics meeting the selection criteria
	 */
	@Override
	public List<Statistics> selectModels(boolean isOptimized) {
		List<Statistics> statisticsList = model.getStatistics();
		
		if (!isOptimized) {
			return statisticsList;
		}

		// Apply factor-based filtering
		Double minMaxDD = model.getFactors().get("min_max_dd");
		Double maxMaxDD = model.getFactors().get("max_max_dd");
		Double maxUlcerIndex = model.getFactors().get("max_ulcerIndex");

		if (minMaxDD == null || maxMaxDD == null || maxUlcerIndex == null) {
			throw new IllegalStateException(
					"Required factors not configured: min_max_dd, max_max_dd, max_ulcerIndex");
		}

		return statisticsList.stream()
				.filter(s -> s.max_dd >= minMaxDD)
				.filter(s -> s.max_dd <= maxMaxDD)
				.filter(s -> s.ulcerIndex <= maxUlcerIndex)
				.sorted((s1, s2) -> Double.compare(s2.martin, s1.martin))
				.collect(Collectors.toList());
	}
}
