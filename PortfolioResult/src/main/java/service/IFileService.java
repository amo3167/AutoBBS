package service;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import model.Rates;
import model.Results;
import model.Statistics;

public interface IFileService {

	public Map<String,Double> readPortfolioRisk(String filePath);
	public double readEquityCSV(String filePath);

	public List<Results> readOpenOrdersCSV(String filePath);
	public List<Results> readClosedOrdersCSV(String filePath);
	public void writeMismatchedFile(String filePath,Map<String,String> misMatched) throws IOException;

	public void readCSV(String filePath,String strategyID);
	public void writeCVS(String filePath,boolean isAdjusted) throws IOException;
	public void writeNewStrategyCVS(String filePath, boolean isAdjusted) throws IOException;

	public void generateWeelyReport(String filePath,boolean isAdjusted) throws IOException;
	public void generateMonthlyReport(String filePath,boolean isAdjusted) throws IOException;
	
	public void writeStatiscsCVS(String filePath,Statistics statistics) throws IOException;
	public void writeStatiscsListCVS(String filePath,boolean isOptimized) throws IOException;
	
	public void readHistoricalRateCSV(String filePath)throws IOException;
	public List<Rates> readMt4RateCSV(String filePath) throws IOException;
	public List<Rates> readNtsRateCSV(String filePath) throws IOException;

	public void writeMT4RateCSV(String filePath, List<Rates> rates);
	public void writeNtsRateCSV(String filePath, List<Rates> rates)throws IOException;
	public void generateDailyCheckReport(String filePath)throws IOException;

	public void writeToErrorFile(String filePath,String data) throws IOException;
}
