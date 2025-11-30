package model;

import java.util.HashMap;
import java.util.OptionalDouble;

public class Statistics {
	public double totalReturn;
	public double max_dd;
	public double max_dd_length;
	public double profit_factor;
	public double cagr;
	public double cagr_maxdd;
	public double sharp_ratio;
	public long num_trades;
	public long num_longs;
	public long num_shorts;
	public double trades_week;
	public double r2;
	public double winning;
	public double risk_reward;
	public double yearsTraded;	
	
	public OptionalDouble maxWinningTrade;
	public OptionalDouble maxLosingTrade;
	
	public double ulcerIndex;
	public double martin;
	
	public HashMap<String,Double> strategyRisk = new HashMap<String,Double>();
	
	public Statistics(){
		
	}
}
