package model;

import java.util.Date;
import java.util.List;
import java.util.Map;

public interface ModelDataService {
		
	public void addResult(Results result);
	public List<Results> getAllResults();

	public void setStartDate(Date startDate);
	public void setFactors(Map<String,Double> factors);
	public Map<String,Double> getFactors();
	
	public void addFirstDate(String strategyID,Date date);
	public void clear();
		
	public void initModelData(Map<String,Double> risks,boolean isNoCashOut);
	
	public List<Results> getAdjustedData(boolean isAdjusted);
	
	public void saveStatistics(Statistics statistics);
	public List<Statistics> getTopStatistics(int top);
	public List<Statistics> getStatistics();	
	
	public void addStrategy(String id,double risk);
	
	public void addStrategyList(Map<String,Double> risks);
	
	public Map<String,Double> getStrategyRisks();
	
	public void addRate(Rates rate);
	
	public void addDailyCheck(String daily,DailyCheck dailyCheck);
	
	public void checkDailyRates(int rateIndex);
	
	public Map<String,DailyCheck> getDailyChecks();
	
	public List<Rates> adjustTimeZone_Commodity();
	public List<Rates> adjustTimeZone();

	boolean getNoCashOutMode();

	public List<Rates> getRates();
	
}
