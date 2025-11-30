package model;

import java.time.LocalDate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import java.util.stream.Collectors;

import service.DateTimeHelper;

public class ModelData implements IModelData {
	public static final double INITBALANCE = 100000;
	private TreeSet<Results> results = new TreeSet<>();
	private HashMap<String, Date> firstDates = new HashMap<>();

	private List<Statistics> statisticList = new ArrayList<>();
	
	private HashMap<String,Double> strategyRisk = new HashMap<>();	

	private TreeSet<Rates> rates = new TreeSet<>();
	
	private HashMap<String,DailyCheck> dailyChecks = new HashMap<>();

	private boolean isNoCashOut = false;

	private Date startDate;
	private HashMap<String,Double> factors = new HashMap<>();

	public ModelData(Map<String,Double> risks,boolean isNoCashOut) {
		initModelData(risks,isNoCashOut);
	}
	
	public ModelData() {
		clear();
	} 
	
	@Override
	public void initModelData(Map<String,Double> risks,boolean isNoCashOut) {
		clear();
		
		strategyRisk = new HashMap<String,Double>(risks);
		this.isNoCashOut = isNoCashOut;
//		strategyRisk.entrySet().stream().
//			forEach(d->optimise.put(d.getKey(),new ArrayList<Double>(
//			    Arrays.asList(0.8,1.0,1.2)))
//					);
				
	}

	@Override
	public void clear() {

		results.clear();			
		strategyRisk.clear();
		
		firstDates.clear();
		statisticList.clear();
		
		dailyChecks.clear();
		rates.clear();
		
	}

	@Override
	public boolean getNoCashOutMode() {
		return isNoCashOut;
	}

	@Override
	public List<Rates> getRates() {
		return new ArrayList<>(rates);
	}


	@Override
	public void addResult(Results result) {
		results.add(result);
	}

	private TreeSet<Results> cloneResultTreeSet(){
		TreeSet<Results> tmp = new TreeSet<>();
		for(Results r: results){
			tmp.add(new Results(r));
		}
		return tmp;
	}
	
	@Override
	public List<Results> getAllResults() {
		
		return cloneResultTreeSet().stream().collect(Collectors.toList());
	}

	@Override
	public void setStartDate(Date startDate) {
		this.startDate = startDate;
	}

	@Override
	public void setFactors(Map<String, Double> factors) {
		this.factors.putAll(factors);
	}

	@Override
	public Map<String, Double> getFactors() {
		return factors;
	}


	@Override
	public void addFirstDate(String strategyID, Date date) {
		firstDates.put(strategyID, date);
	}

	
	@Override
	public List<Results> getAdjustedData(boolean isAdjusted) {

		List<Results> adjustedResults;
		if (isAdjusted) {			
			Date maxFirstDate = firstDates.values().stream().max(Date::compareTo).get();

			if(maxFirstDate.after(startDate)) {
				startDate = maxFirstDate;
			}

			Date finalMaxFirstDate = startDate;
			adjustedResults = cloneResultTreeSet().stream().filter(m -> (m.openTime.after(finalMaxFirstDate) || m.openTime.equals(finalMaxFirstDate)))
					.collect(Collectors.toList());
		}
		else
			adjustedResults = getAllResults();

		
		
		Double balance = INITBALANCE;
		//Adjusted by risks as well
		//Adjusted by PL percentage.
		
		for (Results result : adjustedResults) {
			try{
				double leverage = balance / result.balance;
				result.lots = leverage * result.lots* strategyRisk.get(result.strategyID);
				if(isNoCashOut)
					result.profit = result.pl * balance * strategyRisk.get(result.strategyID);
				else
					result.profit = result.profit * strategyRisk.get(result.strategyID);
				balance += result.profit;	
							
				result.balance = balance;
			}
			catch(Exception e){
				System.out.println(e.getMessage());
			}
		}
		
		return adjustedResults;
	}

	@Override
	public void saveStatistics(Statistics statistics) {
		statistics.strategyRisk = new HashMap<String,Double>(this.strategyRisk);
		this.statisticList.add(statistics);
	}

	@Override
	public List<Statistics> getTopStatistics(int top) {
		
		//Apply for filter logic
				
		List<Statistics> temp = statisticList.stream().sorted(
				(o1, o2)-> Double.valueOf(o2.cagr_maxdd).compareTo( Double.valueOf(o1.cagr_maxdd))).				
                collect(Collectors.toList());
    
		return temp.stream().limit((long) ( temp.size() * top / 100)).collect(Collectors.toList());
	}
	
	@Override
	public List<Statistics> getStatistics() {
		
		
		return statisticList;
	}

	@Override
	public void addStrategy(String id, double risk) {
		strategyRisk.put(id, risk);
		
	}

	@Override
	public void addStrategyList(Map<String, Double> risks) {
		
		strategyRisk = new HashMap<String,Double>(risks);
	}

	@Override
	public Map<String, Double> getStrategyRisks() {
		return strategyRisk;
	}

	@Override
	public void addRate(Rates rate) {
		rates.add(rate);
		
	}

	@Override
	public void addDailyCheck(String daily,DailyCheck dailyCheck) {
		
		dailyChecks.put(daily,dailyCheck);
		
	}

	@Override
	public void checkDailyRates(int rateIndex) {
		
		Rates openRate = null;
		Rates closeRate = null;
				
		int i = 0;
		List<Rates> tmp = rates.stream().collect(Collectors.toList());
		for(Rates rate: tmp){
			
			if(i > 0){					
				if(rate.volume == 0 && tmp.get(i-1).volume > 0 
						&& tmp.subList(i, i+30/rateIndex).stream().allMatch(d->d.volume == 0)){					
					
					closeRate = new Rates(tmp.get(i-1));
					String key = DateTimeHelper.formatDate(closeRate.datetime,"yyyyMMdd");					
					DailyCheck check = new DailyCheck(null,closeRate.datetime);
					
					if(dailyChecks.containsKey(key))
						check.opentime = dailyChecks.get(key).opentime;
											
					addDailyCheck(key,check);
				}
				
				if(rate.volume > 0 && tmp.get(i-1).volume == 0 
						&& tmp.subList(i-30/rateIndex, i-1).stream().allMatch(d->d.volume == 0)){
					openRate = new Rates(rate);
					
					Date adjustedDate = openRate.datetime;
					if(DateTimeHelper.getHour(openRate.datetime) > 14)
						adjustedDate = DateTimeHelper.addDay(openRate.datetime, 1);
					
					
					String key = DateTimeHelper.formatDate(adjustedDate,"yyyyMMdd");					
					DailyCheck check = new DailyCheck(openRate.datetime,null);
					
					if(dailyChecks.containsKey(key))
						check.closetime = dailyChecks.get(key).closetime;
						//continue;
											
					addDailyCheck(key,check);
					
				}
			}
	
			i++;
			
		}
		
	}

	@Override
	public Map<String, DailyCheck> getDailyChecks() {
		
		return dailyChecks;
	}

	@Override
	public List<Rates> adjustTimeZone_Commodity() {
		
		List<Rates> tmp = new ArrayList<>(rates);
		List<Rates> result = new ArrayList<Rates>();
		
		List<String> earlyClose =new ArrayList<>();
		earlyClose.add("21/01/13"); //Martin Luther King Day
		earlyClose.add("18/02/13"); //Presidents Day
		earlyClose.add("27/05/13"); //Memorial Day
		earlyClose.add("04/07/13"); //independent day
		earlyClose.add("02/09/13"); //labor day
		earlyClose.add("28/11/13");
		earlyClose.add("29/11/13");
		
		earlyClose.add("20/01/14"); //Martin Luther King Day
		earlyClose.add("17/02/14"); //Presidents Day
		earlyClose.add("26/05/14"); //Memorial Day
		earlyClose.add("04/07/14"); //independent day
		earlyClose.add("01/09/14"); //labor day
		earlyClose.add("27/11/14");
		earlyClose.add("28/11/14");
		
		earlyClose.add("19/01/15"); //Martin Luther King Day
		earlyClose.add("16/02/15"); //Presidents Day
		earlyClose.add("25/05/15"); //Memorial Day
		earlyClose.add("04/07/15"); //independent day
		earlyClose.add("07/09/15"); //labor day
		earlyClose.add("26/11/15");
		earlyClose.add("27/11/15");
		
		earlyClose.add("18/01/16"); //Martin Luther King Day
		earlyClose.add("15/02/16"); //Presidents Day
		earlyClose.add("30/05/16"); //Memorial Day
		earlyClose.add("04/07/16"); //independent day
		earlyClose.add("05/09/16"); //labor day
		earlyClose.add("24/11/16");
		earlyClose.add("25/11/16");
		
		earlyClose.add("16/01/17"); //Martin Luther King Day
		earlyClose.add("20/02/17"); //Presidents Day
		earlyClose.add("29/05/17"); //Memorial Day
		earlyClose.add("04/07/17"); //independent day
		earlyClose.add("04/09/17"); //labor day
		earlyClose.add("23/11/17");
		earlyClose.add("24/11/17");

		earlyClose.add("15/01/18"); //Martin Luther King Day
		earlyClose.add("19/02/18"); //Presidents Day
		earlyClose.add("28/05/18"); //Memorial Day
		earlyClose.add("04/07/18"); //independent day
		earlyClose.add("03/09/18"); //labor day
		earlyClose.add("22/11/18");//thanks giving
		earlyClose.add("23/11/18");
		earlyClose.add("05/12/18");

		earlyClose.add("21/01/19"); //Martin Luther King Day
		earlyClose.add("18/02/19"); //Presidents Day
		earlyClose.add("27/05/19"); //Memorial Day
		earlyClose.add("04/07/19"); //independent day
		earlyClose.add("02/09/19"); //labor day
		earlyClose.add("28/11/19");//thanks giving
		earlyClose.add("29/11/19");

		List<String> holidays = new ArrayList<>();
		holidays.add("01/01/13");
		holidays.add("25/12/13");
		holidays.add("01/01/14");
		holidays.add("25/12/14");
		holidays.add("01/01/15");
		holidays.add("25/12/15");
		holidays.add("01/01/16");
		holidays.add("25/12/16");
		holidays.add("02/01/17");//New Year
		holidays.add("14/04/17");//Easter
		holidays.add("25/12/17");//Christmas
		holidays.add("01/01/18");
		holidays.add("30/03/18");
		holidays.add("25/12/18");
		holidays.add("01/01/19");
		holidays.add("19/04/19");
		holidays.add("25/12/19");
		
		for(Rates rate: tmp){
			
			
			//T+2			
			if(rate.datetime.before( DateTimeHelper.parseDate("11.03.2013 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);			
			else if(rate.datetime.before( DateTimeHelper.parseDate("03.11.2013 23:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("21.04.2014 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("02.11.2014 23:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("10.03.2015 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("01.11.2015 23:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("13.03.2016 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("06.11.2016 23:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("14.03.2017 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("05.11.2017 23:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("11.03.2018 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("04.11.2018 23:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("10.03.2019 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("03.11.2019 23:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else 
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			
			if(DateTimeHelper.isWeekend(rate.datetime))
				continue;
			

			if( DateTimeHelper.getHour(rate.datetime) <1){
				//System.out.println(DateTimeHelper.formatDate(rate.datetime));
				continue;
			}
			
			if( holidays.contains(DateTimeHelper.formatDate(rate.datetime,"dd/MM/yy"))){
				//System.out.println(DateTimeHelper.formatDate(rate.datetime));
				continue;
			}
			
			//filter holiday(new year, christmas....)
			if(earlyClose.contains(DateTimeHelper.formatDate(rate.datetime,"dd/MM/yy"))
					&& DateTimeHelper.getHour(rate.datetime) >=20){
				//System.out.println(DateTimeHelper.formatDate(rate.datetime));
				continue;			
			}
				
			
//			if(!dailyChecks.containsKey(DateTimeHelper.formatDate(rate.datetime,"yyyyMMdd"))
//					&& rate.volume == 0){
//				System.out.println(DateTimeHelper.formatDate(rate.datetime));
//				continue;			
//			}
				
			
			result.add(new Rates(rate));
		}
		
		return result;
	}

	@Override
	public List<Rates> adjustTimeZone() {
		
		List<Rates> tmp = rates.stream().collect(Collectors.toList());
		List<Rates> result = new ArrayList<Rates>();
		
		List<String> holidays = new ArrayList<>();
		holidays.add("01/01/13");
		holidays.add("25/12/13");
		holidays.add("01/01/14");
		holidays.add("25/12/14");
		holidays.add("01/01/15");
		holidays.add("25/12/15");
		holidays.add("01/01/16");
		holidays.add("25/12/16");
		holidays.add("02/01/17");//New Year
		holidays.add("14/04/17");//Easter
		holidays.add("25/12/17");//Christmas
		holidays.add("01/01/18");
		holidays.add("30/03/18");
		holidays.add("25/12/18");
		
		for(Rates rate: tmp){
			
			
			//T+2			
			if(rate.datetime.before( DateTimeHelper.parseDate("11.03.2013 21:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);			
			else if(rate.datetime.before( DateTimeHelper.parseDate("03.11.2013 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("21.04.2014 21:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("02.11.2014 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("10.03.2015 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("01.11.2015 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("13.03.2016 21:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("06.11.2016 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("14.03.2017 21:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("05.11.2017 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else if(rate.datetime.before( DateTimeHelper.parseDate("11.03.2018 21:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			else if(rate.datetime.before( DateTimeHelper.parseDate("04.11.2018 22:00", "dd.MM.yyyy HH:mm")))
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 3);
			else 
				rate.datetime = DateTimeHelper.addHours(rate.datetime, 2);
			
			if(DateTimeHelper.isWeekend(rate.datetime))
				continue;
			

			if( holidays.contains(DateTimeHelper.formatDate(rate.datetime,"dd/MM/yy")))			
				continue;			
				
			
			result.add(new Rates(rate));
		}
		
		return result;
	}

}
