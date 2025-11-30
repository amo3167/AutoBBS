package service;

import java.util.List;

import model.Statistics;

public interface IStatisticsService {
	public void caculate(double initialBalance,boolean isAdjusted);
	
	public List<Statistics> selectModels(boolean isOptimized);
}
