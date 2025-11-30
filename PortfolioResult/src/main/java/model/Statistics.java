package model;

import java.util.HashMap;
import java.util.Map;
import java.util.OptionalDouble;

/**
 * Holds statistical metrics for portfolio performance analysis.
 * Includes returns, risk metrics, ratios, and trade statistics.
 */
public class Statistics {
	
	/** Total return percentage */
	public double totalReturn;
	
	/** Maximum drawdown percentage */
	public double max_dd;
	
	/** Maximum drawdown length in days */
	public double max_dd_length;
	
	/** Profit factor (total wins / total losses) */
	public double profit_factor;
	
	/** Compound Annual Growth Rate percentage */
	public double cagr;
	
	/** CAGR divided by maximum drawdown */
	public double cagr_maxdd;
	
	/** Sharpe ratio (risk-adjusted return) */
	public double sharp_ratio;
	
	/** Total number of trades */
	public long num_trades;
	
	/** Number of long (buy) positions */
	public long num_longs;
	
	/** Number of short (sell) positions */
	public long num_shorts;
	
	/** Average number of trades per week */
	public double trades_week;
	
	/** R-squared (coefficient of determination) */
	public double r2;
	
	/** Winning trade percentage */
	public double winning;
	
	/** Risk-reward ratio */
	public double risk_reward;
	
	/** Number of years traded */
	public double yearsTraded;

	/** Maximum winning trade amount */
	public OptionalDouble maxWinningTrade;
	
	/** Maximum losing trade amount */
	public OptionalDouble maxLosingTrade;

	/** Ulcer Index (downside risk measure) */
	public double ulcerIndex;
	
	/** Martin ratio (CAGR / Ulcer Index) */
	public double martin;

	/** Map of strategy IDs to their risk allocations */
	public Map<String, Double> strategyRisk = new HashMap<>();
	
	/**
	 * Gets strategy risk as FastUtil map for performance.

	/**
	 * Default constructor.
	 */
	public Statistics() {
	}
}
