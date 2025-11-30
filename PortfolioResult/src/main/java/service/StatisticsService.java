package service;

import java.util.ArrayList;
import java.util.List;
import java.util.OptionalDouble;
import java.util.stream.Collectors;

import model.IModelData;
import model.Results;
import model.Statistics;

public class StatisticsService implements IStatisticsService {

	private IModelData model;

	public StatisticsService(IModelData model) {
		this.model = model;
	}

	@Override
	public void caculate(double initialBalance, boolean isAdjusted) {

		Statistics statistics = new Statistics();

		List<Results> results = model.getAdjustedData(isAdjusted);

		statistics.num_trades = results.size();
		statistics.num_longs = results.stream().filter(m -> m.orderType.equals("BUY")).count();
		statistics.num_shorts = results.stream().filter(m -> m.orderType.equals("SELL")).count();

		statistics.totalReturn = results.get(results.size() - 1).balance / initialBalance;

		double totalWinning = results.stream().filter(m -> m.profit > 0).mapToDouble(d -> d.profit).sum();
		long totalWinningTrades = results.stream().filter(m -> m.profit > 0).count();
		OptionalDouble averageWinningTrade = results.stream().filter(m -> m.profit > 0)
				.mapToDouble(d -> Math.abs(d.profit / d.balance)).average();

		double totalLosing = Math.abs(results.stream().filter(m -> m.profit <= 0).mapToDouble(d -> d.profit).sum());
		
		OptionalDouble averageLosingTrade = results.stream().filter(m -> m.profit < 0)
				.mapToDouble(d -> Math.abs(d.profit / d.balance)).average();

		statistics.winning = (double) totalWinningTrades / statistics.num_trades * 100.0;
		statistics.risk_reward = averageWinningTrade.getAsDouble() / averageLosingTrade.getAsDouble();

		statistics.maxWinningTrade = results.stream().mapToDouble(d -> d.profit).max();
		statistics.maxLosingTrade = results.stream().mapToDouble(d -> d.profit).min();

		statistics.profit_factor = totalWinning / totalLosing;

		// Max_DD
		double maxBalance = 0.0;
		double maxDDDepthTemp = 0.0;
		double maxDDDepth = 0.0;
		long maxDDLengthTemp = 0;
		long maxDDLength = 0;

		long ddStartTime = results.get(0).closeTime.getTime();

		double sumBalanceTime = 0.0;
		double timeSqrSum = 0.0;
		double avgBalanceLog = 0.0;
		double linearRegressionSlope;		

		for (Results result : results) {

			timeSqrSum += Math.pow((double) result.closeTime.getTime() - results.get(0).closeTime.getTime(), 2);

			// calculations needed for linear regression
			sumBalanceTime += (result.closeTime.getTime() - results.get(0).closeTime.getTime())
					* (Math.log(result.balance) - Math.log(results.get(0).balance));
			
			avgBalanceLog += (Math.log(result.balance) - Math.log(results.get(0).balance)) / statistics.num_trades;

			if (result.balance < maxBalance) {
				maxDDDepthTemp = ((maxBalance - result.balance) / maxBalance) * 100;
				maxDDLengthTemp = result.closeTime.getTime() - ddStartTime;
			}

			if (result.balance > maxBalance) {
				maxBalance = result.balance;
				maxDDDepthTemp = 0.0;
				maxDDLengthTemp = 0;
				ddStartTime = result.closeTime.getTime();
			}

			if (maxDDDepthTemp > maxDDDepth) {
				maxDDDepth = maxDDDepthTemp;
			}

			if (maxDDLengthTemp > maxDDLength) {
				maxDDLength = maxDDLengthTemp;
			}
		}

		statistics.max_dd = maxDDDepth;
		statistics.max_dd_length = (double) maxDDLength / (1000 * 60 * 60 * 24);

		statistics.yearsTraded = (double) Math
				.abs((results.get(0).closeTime.getTime() - results.get(results.size() - 1).closeTime.getTime()))
				/ (1000 * 3600 * 24) / 365;

		statistics.cagr = 100 * (Math.pow(statistics.totalReturn, 1 / statistics.yearsTraded) - 1);
		statistics.cagr_maxdd = statistics.cagr / statistics.max_dd;

		linearRegressionSlope = (sumBalanceTime) / timeSqrSum;
		

		double yPs = 0.0;
		double yRs = 0.0;

		List<Double> regressionResiduals = new ArrayList<>();

		// determination coefficient calculation
		for (int j = 0; j < statistics.num_trades; j++) {
			regressionResiduals
					.add(linearRegressionSlope * (results.get(j).closeTime.getTime() - results.get(0).closeTime.getTime())
							- (Math.log(results.get(j).balance) - Math.log(results.get(0).balance)));
			yPs += Math.pow(
					(linearRegressionSlope * (results.get(j).closeTime.getTime() - results.get(0).closeTime.getTime())
							- (Math.log(results.get(j).balance) - Math.log(results.get(0).balance))),
					2);
			yRs += Math.pow(((Math.log(results.get(j).balance) - Math.log(results.get(0).balance)) - avgBalanceLog), 2);
		}

		if ((yRs == 0) || (1 - yPs / yRs) < 0)
			statistics.r2 = 0;
		else
			statistics.r2 = (1 - yPs / yRs);

		statistics.trades_week = statistics.num_trades / statistics.yearsTraded / 52;

		calculateWeeklyStatistics(results,initialBalance,statistics);
		model.saveStatistics(statistics);
	}

	private void calculateWeeklyStatistics(List<Results> results, double initialBalance,
			Statistics statistics) {

		/* Ulcer Index, Sharpe and martin ratio calculations */

		long n = 0;
		int j = 0;		
		double sumSqrt = 0;
		double cumWeekReturn = 0;
		double cumWeekReturnSquare = 0;
		double maxBalance = initialBalance;

		double meanWeekly;
		double sigmaWeekly;
		double weekReturn;
		
		long analysisTime = results.get(0).closeTime.getTime();
		double startWeekBalance = initialBalance;
		double lastWeekBalance = startWeekBalance;

		int statisticsSize = results.size();
		
		long lastDate = results.get(statisticsSize-1).closeTime.getTime();

		while (analysisTime < lastDate) {
			n++;
			analysisTime += 1000*3600*24*5;

			while (j < statisticsSize && results.get(j).closeTime.getTime() < analysisTime) {
				lastWeekBalance = results.get(j).balance;				
				j++;
			}

			weekReturn = (lastWeekBalance - startWeekBalance) / startWeekBalance;

			cumWeekReturn += weekReturn;
			cumWeekReturnSquare += weekReturn * weekReturn;
			startWeekBalance = results.get(j - 1).balance;

			if (lastWeekBalance > maxBalance) {
				maxBalance = lastWeekBalance;
			} else {
				sumSqrt += Math.pow((100 * ((lastWeekBalance / maxBalance) - 1)), 2);
			}
		}

		/*
		 * if above 100 make the UlcerIndex 100 (all strategies above 100 are
		 * useless)
		 */
		if (Math.sqrt(sumSqrt / n) > 100)
			statistics.ulcerIndex = 100;
		else
			statistics.ulcerIndex = Math.sqrt(sumSqrt / n);

		meanWeekly = cumWeekReturn / n;
		sigmaWeekly = Math.sqrt((n * cumWeekReturnSquare - cumWeekReturn * cumWeekReturn) / (n * (n - 1)));

		statistics.sharp_ratio = 7.2111103 * (meanWeekly / sigmaWeekly);
		statistics.martin = statistics.cagr / statistics.ulcerIndex;

	}

	/*
	 * 1. max_dd is between 7 - 15
	 * 2. cagr is top 40%
	 * 3. ulcerIndex is lower than 2.4 or lower 50%	 * 3.
	 * 4. order by martin
	 * (non-Javadoc)
	 * @see service.IStatisticsService#selectModels()
	 */
	@Override
	public List<Statistics> selectModels(boolean isOptimized) {
		
		List<Statistics> sList = model.getStatistics();
		if(isOptimized){

			return  sList.stream().filter(o-> o.max_dd>= model.getFactors().get("min_max_dd")
					&& o.max_dd <= model.getFactors().get("max_max_dd")
					&& o.ulcerIndex <= model.getFactors().get("max_ulcerIndex"))
					.sorted((o1, o2)-> Double.compare(o2.martin, o1.martin))
					.collect(Collectors.toList());

//			List<Statistics> temp = sList.stream().filter(o-> o.max_dd>= 7 && o.max_dd <= model.getFactors().get("max_dd"))
//					.sorted((o1, o2)-> Double.compare(o2.cagr_maxdd, o1.cagr_maxdd))
//					.limit((long) ( sList.size() * 40 / 100)).collect(Collectors.toList());
//
//			List<Statistics> temp2 = temp.stream()
//					.filter(o-> o.ulcerIndex <= model.getFactors().get("ulcerIndex"))
//					.sorted((o1, o2)-> Double.compare(o1.ulcerIndex, o2.ulcerIndex))
//					.limit((long) ( temp.size() * 50 / 100))
//					.sorted((o1, o2)-> Double.compare(o2.martin, o1.martin))
//					.collect(Collectors.toList());

//			return temp2;
		}
		return sList;
		

		
		
		
		
	}
}
