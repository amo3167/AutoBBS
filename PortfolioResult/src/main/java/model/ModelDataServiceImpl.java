package model;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import java.util.stream.Collectors;

import service.DateTimeHelper;

/**
 * Central data model for portfolio analysis, managing results, rates, and statistics.
 * Thread-safe through immutable collections returned from public methods.
 */
public class ModelDataServiceImpl implements ModelDataService {
	
	// Constants
	public static final double INITBALANCE = 100000.0;
	private static final int RATE_CHECK_WINDOW = 30; // minutes
	private static final int MARKET_OPEN_HOUR_THRESHOLD = 14;
	
	// Core data structures
	private final TreeSet<Results> results = new TreeSet<>();
	private final Map<String, Date> firstDates = new HashMap<>();
	private final List<Statistics> statisticList = new ArrayList<>();
	private final Map<String, Double> strategyRisk = new HashMap<>();
	private final TreeSet<Rates> rates = new TreeSet<>();
	private final Map<String, DailyCheck> dailyChecks = new HashMap<>();
	private final Map<String, Double> factors = new HashMap<>();
	
	// Configuration
	private boolean noCashOutMode;
	private Date startDate;

	/**
	 * Constructs ModelDataServiceImpl with strategy risks and cash-out mode.
	 * 
	 * @param risks         map of strategy IDs to risk multipliers
	 * @param isNoCashOut   true to use P&L percentage, false to use absolute profit
	 */
	public ModelDataServiceImpl(Map<String, Double> risks, boolean isNoCashOut) {
		initModelData(risks, isNoCashOut);
	}
	
	/**
	 * Constructs an empty ModelDataServiceImpl with default settings.
	 */
	public ModelDataServiceImpl() {
		clear();
	}

	@Override
	public void initModelData(Map<String, Double> risks, boolean isNoCashOut) {
		if (risks == null) {
			throw new IllegalArgumentException("Strategy risks map cannot be null");
		}
		
		clear();
		strategyRisk.putAll(risks);
		this.noCashOutMode = isNoCashOut;
	}

	@Override
	public void clear() {
		results.clear();
		strategyRisk.clear();
		firstDates.clear();
		statisticList.clear();
		dailyChecks.clear();
		rates.clear();
		factors.clear();
	}

	@Override
	public boolean getNoCashOutMode() {
		return noCashOutMode;
	}

	@Override
	public List<Rates> getRates() {
		return new ArrayList<>(rates);
	}


	@Override
	public void addResult(Results result) {
		if (result != null) {
			results.add(result);
		}
	}

	/**
	 * Creates a deep copy of the results TreeSet.
	 * 
	 * @return a new TreeSet containing copies of all results
	 */
	private TreeSet<Results> cloneResultTreeSet() {
		TreeSet<Results> copy = new TreeSet<>();
		for (Results r : results) {
			copy.add(new Results(r));
		}
		return copy;
	}
	
	@Override
	public List<Results> getAllResults() {
		return new ArrayList<>(cloneResultTreeSet());
	}

	@Override
	public void setStartDate(Date startDate) {
		this.startDate = startDate;
	}

	@Override
	public void setFactors(Map<String, Double> factors) {
		if (factors != null) {
			this.factors.putAll(factors);
		}
	}

	@Override
	public Map<String, Double> getFactors() {
		return new HashMap<>(factors);
	}


	@Override
	public void addFirstDate(String strategyID, Date date) {
		if (strategyID != null && date != null) {
			firstDates.put(strategyID, date);
		}
	}

	
	
	@Override
	public List<Results> getAdjustedData(boolean isAdjusted) {
		List<Results> adjustedResults = filterResultsByStartDate(isAdjusted);
		return applyRiskAdjustments(adjustedResults);
	}

	/**
	 * Filters results based on the adjusted start date.
	 * 
	 * @param isAdjusted  whether to apply date filtering
	 * @return filtered list of results
	 */
	private List<Results> filterResultsByStartDate(boolean isAdjusted) {
		if (!isAdjusted) {
			return getAllResults();
		}

		Date effectiveStartDate = calculateEffectiveStartDate();
		return cloneResultTreeSet().stream()
				.filter(result -> !result.closeTime.before(effectiveStartDate))
				.collect(Collectors.toList());
	}

	/**
	 * Calculates the effective start date as the latest of configured start date
	 * and the maximum first trade date across all strategies.
	 * 
	 * @return the effective start date
	 */
	private Date calculateEffectiveStartDate() {
		if (firstDates.isEmpty()) {
			return startDate;
		}

		Date maxFirstDate = firstDates.values().stream()
				.max(Date::compareTo)
				.orElse(startDate);

		return (startDate != null && maxFirstDate.before(startDate)) 
				? startDate 
				: maxFirstDate;
	}

	/**
	 * Applies risk multipliers and computes running balance for results.
	 * 
	 * @param results  list of results to adjust
	 * @return adjusted results with risk-weighted profits and balances
	 */
	private List<Results> applyRiskAdjustments(List<Results> results) {
		double balance = INITBALANCE;

		for (Results result : results) {
			try {
				Double risk = strategyRisk.get(result.strategyID);
				if (risk == null) {
					System.err.println("No risk multiplier found for strategy: " + result.strategyID);
					continue;
				}

				double leverage = balance / result.balance;
				result.lots = leverage * result.lots * risk;
				
				if (noCashOutMode) {
					result.profit = result.pl * balance * risk;
				} else {
					result.profit = result.profit * risk;
				}
				
				balance += result.profit;
				result.balance = balance;
				
			} catch (Exception e) {
				System.err.println("Error adjusting result: " + e.getMessage());
			}
		}

		return results;
	}	@Override
	public void saveStatistics(Statistics statistics) {
		if (statistics != null) {
			statistics.strategyRisk = new HashMap<>(this.strategyRisk);
			this.statisticList.add(statistics);
		}
	}

	@Override
	public List<Statistics> getTopStatistics(int topPercentage) {
		if (topPercentage <= 0 || topPercentage > 100) {
			throw new IllegalArgumentException(
					"Top percentage must be between 1 and 100, got: " + topPercentage);
		}

		List<Statistics> sortedByCAGRToDD = statisticList.stream()
				.sorted((s1, s2) -> Double.compare(s2.cagr_maxdd, s1.cagr_maxdd))
				.collect(Collectors.toList());

		long limit = Math.max(1, (long) (sortedByCAGRToDD.size() * topPercentage / 100.0));
		return sortedByCAGRToDD.stream()
				.limit(limit)
				.collect(Collectors.toList());
	}
	
	@Override
	public List<Statistics> getStatistics() {
		return new ArrayList<>(statisticList);
	}

	@Override
	public void addStrategy(String id, double risk) {
		if (id != null) {
			strategyRisk.put(id, risk);
		}
	}

	@Override
	public void addStrategyList(Map<String, Double> risks) {
		if (risks != null) {
			strategyRisk.clear();
			strategyRisk.putAll(risks);
		}
	}

	@Override
	public Map<String, Double> getStrategyRisks() {
		return new HashMap<>(strategyRisk);
	}

	@Override
	public void addRate(Rates rate) {
		if (rate != null) {
			rates.add(rate);
		}
	}

	@Override
	public void addDailyCheck(String daily, DailyCheck dailyCheck) {
		if (daily != null && dailyCheck != null) {
			dailyChecks.put(daily, dailyCheck);
		}
	}

	@Override
	public void checkDailyRates(int rateIndex) {
		if (rateIndex <= 0) {
			throw new IllegalArgumentException("Rate index must be positive, got: " + rateIndex);
		}

		List<Rates> rateList = new ArrayList<>(rates);
		int windowSize = RATE_CHECK_WINDOW / rateIndex;

		for (int i = 1; i < rateList.size(); i++) {
			detectMarketClose(rateList, i, windowSize);
			detectMarketOpen(rateList, i, windowSize);
		}
	}

	/**
	 * Detects market close events based on volume patterns.
	 * 
	 * @param rateList    list of rates
	 * @param index       current index
	 * @param windowSize  number of bars to check
	 */
	private void detectMarketClose(List<Rates> rateList, int index, int windowSize) {
		Rates current = rateList.get(index);
		Rates previous = rateList.get(index - 1);

		// Market close: volume drops to zero and stays there
		if (current.volume == 0 && previous.volume > 0) {
			int endIndex = Math.min(index + windowSize, rateList.size());
			boolean allZeroVolume = rateList.subList(index, endIndex).stream()
					.allMatch(r -> r.volume == 0);

			if (allZeroVolume) {
				String dateKey = DateTimeHelper.formatDate(previous.datetime, "yyyyMMdd");
				DailyCheck check = dailyChecks.getOrDefault(dateKey, new DailyCheck());
				check.closetime = previous.datetime;
				addDailyCheck(dateKey, check);
			}
		}
	}

	/**
	 * Detects market open events based on volume patterns.
	 * 
	 * @param rateList    list of rates
	 * @param index       current index
	 * @param windowSize  number of bars to check
	 */
	private void detectMarketOpen(List<Rates> rateList, int index, int windowSize) {
		Rates current = rateList.get(index);
		Rates previous = rateList.get(index - 1);

		// Market open: volume returns after being zero
		if (current.volume > 0 && previous.volume == 0) {
			int startIndex = Math.max(index - windowSize, 0);
			boolean allZeroVolume = rateList.subList(startIndex, index - 1).stream()
					.allMatch(r -> r.volume == 0);

			if (allZeroVolume) {
				Date adjustedDate = adjustDateForLateOpen(current.datetime);
				String dateKey = DateTimeHelper.formatDate(adjustedDate, "yyyyMMdd");
				DailyCheck check = dailyChecks.getOrDefault(dateKey, new DailyCheck());
				check.opentime = current.datetime;
				addDailyCheck(dateKey, check);
			}
		}
	}

	/**
	 * Adjusts date to next day if market opens late in the day.
	 * 
	 * @param datetime  the market open time
	 * @return adjusted date for late opens
	 */
	private Date adjustDateForLateOpen(Date datetime) {
		if (DateTimeHelper.getHour(datetime) > MARKET_OPEN_HOUR_THRESHOLD) {
			return DateTimeHelper.addDay(datetime, 1);
		}
		return datetime;
	}

	@Override
	public Map<String, DailyCheck> getDailyChecks() {
		return new HashMap<>(dailyChecks);
	}

	@Override
	public List<Rates> adjustTimeZone_Commodity() {
		TimeZoneAdjuster adjuster = new CommodityTimeZoneAdjuster();
		return adjuster.adjustTimeZone(new ArrayList<>(rates));
	}

	@Override
	public List<Rates> adjustTimeZone() {
		TimeZoneAdjuster adjuster = new StandardTimeZoneAdjuster();
		return adjuster.adjustTimeZone(new ArrayList<>(rates));
	}
}
