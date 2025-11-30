package service;

import java.util.List;

import model.Statistics;

/**
 * Service for calculating and filtering portfolio statistics.
 * Provides methods for computing performance metrics and selecting optimal portfolio configurations.
 */
public interface StatisticsService {
	
	/**
	 * Calculates comprehensive statistics for the portfolio.
	 * 
	 * @param initialBalance  the starting balance
	 * @param isAdjusted      whether to use adjusted data (filtered by start date)
	 */
	void calculate(double initialBalance, boolean isAdjusted);
	
	/**
	 * Selects portfolio models based on optimization criteria.
	 * 
	 * @param isOptimized  whether to apply factor-based filtering
	 * @return list of statistics meeting the selection criteria
	 */
	List<Statistics> selectModels(boolean isOptimized);
}
