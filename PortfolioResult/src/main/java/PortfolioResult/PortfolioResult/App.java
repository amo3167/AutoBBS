package PortfolioResult.PortfolioResult;

import java.io.IOException;
import java.time.*;
import java.time.format.DateTimeFormatter;
import java.time.temporal.ChronoUnit;
import java.util.*;
import java.util.stream.Collectors;

import model.*;
import org.apache.log4j.Logger;

import org.joda.time.Minutes;
import service.FileService;
import service.IFileService;
import service.IStatisticsService;
import service.StatisticsService;

/**
 * Hello world!
 *
 */
public class App 
{
	private final static Logger logger = Logger.getLogger(App.class);
	
	private static IModelData model = new ModelData();	
	private static IStatisticsService statisticsService = new StatisticsService(model);
	private static IFileService fileService = new FileService(model,statisticsService);
	private static ConfigReader configReader = new ConfigReader();

    public static void main( String[] args ) throws IOException {

		Map<String,Double> sRisks = new HashMap<>();
		Map<String,Double> predefinedStrategies = new HashMap<>();
		Map<String,Double> factors = new HashMap<>();
    	/*
    	 *  optimizer
    	 *  run
    	 */

    	if(args[0].equals("run")) {
			sRisks = fileService.readPortfolioRisk(configReader.getPropValues("PortfolioRisk_Location") + args[1]);
			boolean isCheckingOpenOrder = Boolean.parseBoolean(args[2]);

			LocalDate startDate = LocalDate.of(2000,1,1);
			if(args.length == 4) {
				DateTimeFormatter dtf = DateTimeFormatter.ofPattern("yyyy-MM-dd");
				startDate = LocalDate.parse(args[3]);
			}
			model.setStartDate(Date.from(startDate.atStartOfDay(ZoneId.systemDefault()).toInstant()));

			run(sRisks, false,isCheckingOpenOrder);
		}else if (args[0].equals("runCustom")) {
			LocalDate startDate = LocalDate.of(2000,1,1);
			if(args.length == 5) {
				DateTimeFormatter dtf = DateTimeFormatter.ofPattern("yyyy-MM-dd");
				startDate = LocalDate.parse(args[4]);
			}
			model.setStartDate(Date.from(startDate.atStartOfDay(ZoneId.systemDefault()).toInstant()));
			run_custom(args[1]);
		} else if (args[0].equals("optimizer")) {
			sRisks = fileService.readPortfolioRisk(configReader.getPropValues("PortfolioRisk_Location") + args[1]);
			predefinedStrategies = fileService.readPortfolioRisk(configReader.getPropValues("PortfolioRisk_Location") + args[2]);
			factors = fileService.readPortfolioRisk(configReader.getPropValues("PortfolioRisk_Location") + args[3]);

			LocalDate startDate = LocalDate.of(2000,1,1);
			if(args.length == 5) {
				DateTimeFormatter dtf = DateTimeFormatter.ofPattern("yyyy-MM-dd");
				startDate = LocalDate.parse(args[4]);
			}
			model.setStartDate(Date.from(startDate.atStartOfDay(ZoneId.systemDefault()).toInstant()));
			model.setFactors(factors);

			run_optimizer(sRisks,predefinedStrategies);
		} else if (args[0].equals("optimizerLevel2")) {
			runOptimizerLevel2();
		}
    	else if(args[0].equals("XAGUSD_60M")){
			adjustRates_XAGUSD_60M();
		} else if(args[0].equals("USTEC_60M")){
			adjustRates_USTECUSD_60M();
		} else if(args[0].equals("EURUSD_5M")){
			adjustRates_EURUSD_5M();
		} else if(args[0].equals("EURGBP_5M")){
			adjustRates_EURGBP_5M();
		} else if(args[0].equals("MT4Rate")){
			String mt4File = configReader.getPropValues("PortfolioOpenOrders_Location") + args[1];
			String ntsFile = configReader.getPropValues("PortfolioOpenOrders_Location") + args[2];
			System.out.printf("Start to convert from %s to %s\n",mt4File,ntsFile);

			convert_mt4_rate(mt4File,ntsFile);
		} else if(args[0].equals("MT4RateMerge")){
			String mt4File = configReader.getPropValues("PortfolioOpenOrders_Location") + args[1];
			String newMt4File = configReader.getPropValues("PortfolioOpenOrders_Location") + args[2];
			String errorMt4File = configReader.getPropValues("PortfolioOpenOrders_Location") + args[3];
			System.out.printf("Start to merge from %s to %s\n",newMt4File,mt4File);

			merge_mt4_rate(mt4File,newMt4File,errorMt4File,Integer.parseInt(args[4]));
		}





    	//run_optimizer_DailyTrading();
    	
    	//run_optimizer_withoutDailyTrading();
    	
    	//checkRates();
    	//adjustRates_XAUUSD();
    	//adjustRates_XAUUSD_5M();
    	
    	//checkRates_GBPJPY_5M();
    	//adjustRates_GBPJPY_5M();
    	//adjustRates_USDJPY_5M();    	
    	//adjustRates_GBPUSD_5M();
    	
    	//adjustRates_GBPJPY_1M();
    	//adjustRates_USDJPY_1M();    	
    	//adjustRates_GBPUSD_1M();
    	
    	//adjustRates_GBPJPY_15M();
    	//adjustRates_USDJPY_15M();    	
    	//adjustRates_GBPUSD_15M();
    	
    	//adjustRates_XAUUSD_15M();
    	
    	//adjustRates_USDCAD_60M();

    }

    private static int getSkippingDays(LocalDateTime start,LocalDateTime end){
    	int i = 0;
		LocalDateTime tmp = start;
    	while(end.isAfter(tmp)) {
			if (tmp.getDayOfWeek() == DayOfWeek.SATURDAY || tmp.getDayOfWeek() == DayOfWeek.SUNDAY)
				i++;
			tmp = tmp.plusDays(1);
		}
    	return i;
	}

	private static void merge_mt4_rate(String mt4_rate_file, String new_mt4_rate_file,String error_file,int timeFrame) throws IOException{


			List<Rates> currentRates = fileService.readMt4RateCSV(mt4_rate_file);
			List<Rates> newRates = fileService.readMt4RateCSV(new_mt4_rate_file);
			Optional<Date> lastDateTime = currentRates.stream()
					.map(it->it.datetime)
					.max(Date::compareTo);

			Optional<Date> lastNewDateTime = newRates.stream()
					.map(it->it.datetime)
					.max(Date::compareTo);

			if(lastDateTime.isPresent() && lastNewDateTime.isPresent()){
				LocalDateTime start = lastDateTime.get().toInstant().atZone(ZoneId.systemDefault()).toLocalDateTime();
				LocalDateTime end = lastNewDateTime.get().toInstant().atZone(ZoneId.systemDefault()).toLocalDateTime();
				System.out.printf("lastDateTime=%s lastNewDateTime=%s\n",start.toString(),end.toString());

				if(lastNewDateTime.get().after(lastDateTime.get())){
					int weekends = getSkippingDays(start,end);
					double discountRate = 0.01;

					if(mt4_rate_file.contains("XAU")){
						discountRate = 0.1;
					}

					double maxBars = (double) (ChronoUnit.MINUTES.between(start,end) - weekends * 1440) / timeFrame;

					double minBars = maxBars * discountRate;

					List<Rates> addRates = newRates.stream()
							.filter(it -> it.datetime.after(lastDateTime.get()))
							.collect(Collectors.toList());
					long mergedBars = addRates.size();
					System.out.printf("MaxBars=%f, MinBars=%f, MergedBars =%d\n",maxBars,minBars, mergedBars);

					if(mergedBars >= minBars || mergedBars == maxBars) {
						currentRates.addAll(addRates);
						fileService.writeMT4RateCSV(mt4_rate_file,currentRates);
					}
					else {
						double missingBars = Math.min(maxBars,minBars) - mergedBars;

						fileService.writeToErrorFile(error_file,String.format("Missing at least %f bars",minBars));
					}
				}
			}


	}

    private static void convert_mt4_rate(String mt4_rate_file, String nts_rate_file) throws IOException{


		List<Rates> mt4Rates = fileService.readMt4RateCSV(mt4_rate_file);
		List<Rates> ntsRates = fileService.readNtsRateCSV(nts_rate_file);

		Optional<Date> lastDateTime = ntsRates.stream()
				.map(it->it.datetime)
				.max(Date::compareTo);

		mt4Rates.stream()
				.filter(it -> !lastDateTime.isPresent() || it.datetime.after(lastDateTime.get()))
				.forEach(ntsRates::add);

			fileService.writeNtsRateCSV(nts_rate_file,ntsRates);

	}
    
    private static void adjustRates_USDCAD_60M(){
    	try {
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDCAD 60M\\USDCAD_60_2013.csv");    		
    		
    		List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDCAD 60M\\USDCAD_60.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void adjustRates_GBPUSD_1M(){
    	try {
//    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 1M\\GBPUSD_1_2013.csv");
//    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 1M\\GBPUSD_1_2014.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 1M\\GBPUSD_1_2015.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 1M\\GBPUSD_1_2016.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 1M\\GBPUSD_1_2017.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 1M\\GBPUSD_1_2018.csv");
    		
    		List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 1M\\GBPUSD_1.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }

    private static void adjustRates_USDJPY_1M(){
    	try {    		
//    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 1M\\USDJPY_1_2013.csv");
//    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 1M\\USDJPY_1_2014.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 1M\\USDJPY_1_2015.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 1M\\USDJPY_1_2016.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 1M\\USDJPY_1_2017.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 1M\\USDJPY_1_2018.csv");
    		
			List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 1M\\USDJPY_1.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void adjustRates_GBPJPY_1M(){
    	try {
//    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 1M\\GBPJPY_1_2013.csv");
//    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 1M\\GBPJPY_1_2014.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 1M\\GBPJPY_1_2015.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 1M\\GBPJPY_1_2016.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 1M\\GBPJPY_1_2017.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 1M\\GBPJPY_1_2018.csv");
    		
    		List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 1M\\GBPJPY_1.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void adjustRates_GBPUSD_5M(){
    	try {    		
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 5M\\GBPUSD_5_2013.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 5M\\GBPUSD_5_2015.csv");
    		
			List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 5M\\GBPUSD_5.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void adjustRates_USDJPY_5M(){
    	try {    		
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 5M\\USDJPY_5_2013.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 5M\\USDJPY_5_2015.csv");
    		
			List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 5M\\USDJPY_5.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void adjustRates_GBPJPY_5M(){
    	try {
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 5M\\GBPJPY_5_2013.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 5M\\GBPJPY_5_2015.csv");
    		
    		List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 5M\\GBPJPY_5.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }

	private static void adjustRates_EURUSD_5M(){
		try {
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURUSD 5M\\EURUSD_Candlestick_5_M_BID_02.01.2017-01.01.2018.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURUSD 5M\\EURUSD_Candlestick_5_M_BID_02.01.2018-01.01.2019.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURUSD 5M\\EURUSD_Candlestick_5_M_BID_02.01.2019-02.01.2020.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURUSD 5M\\EURUSD_Candlestick_5_M_BID_02.01.2020-01.01.2021.csv");

			List<Rates> rates = model.adjustTimeZone();

			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURUSD 5M\\EURUSD_5.csv",rates);

		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
	}

	private static void adjustRates_EURGBP_5M(){
		try {
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURGBP 5M\\EURGBP_Candlestick_5_M_BID_01.01.2015-01.01.2016.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURGBP 5M\\EURGBP_Candlestick_5_M_BID_01.01.2016-01.01.2018.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURGBP 5M\\EURGBP_Candlestick_5_M_BID_01.01.2018-01.01.2019.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURGBP 5M\\EURGBP_Candlestick_5_M_BID_01.01.2019-01.01.2020.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURGBP 5M\\EURGBP_Candlestick_5_M_BID_01.01.2020-01.05.2021.csv");

			List<Rates> rates = model.adjustTimeZone();

			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\EURGBP 5M\\EURGBP_5.csv",rates);

		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
	}
    
    private static void adjustRates_GBPUSD_15M(){
    	try {    		
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 15M\\GBPUSD_15_2013_2016.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 15M\\GBPUSD_15_2016_2018.csv");
    		
			List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPUSD 15M\\GBPUSD_15.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void adjustRates_GBPJPY_15M(){
    	try {
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 15M\\GBPJPY_15_2013_2016.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 15M\\GBPJPY_15_2016_2018.csv");
    		
    		List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 15M\\GBPJPY_15.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void adjustRates_USDJPY_15M(){
    	try {    		
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 15M\\USDJPY_15_2013.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 15M\\USDJPY_15_2014.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 15M\\USDJPY_15_2015_2017.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 15M\\USDJPY_15_2018.csv");
    		
			List<Rates> rates = model.adjustTimeZone();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\USDJPY 15M\\USDJPY_15.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void adjustRates_XAUUSD_5M(){
    	try {
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 5M\\XAUUSD_5_2013_raw.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 5M\\XAUUSD_5_raw.csv");
    		
			List<Rates> rates = model.adjustTimeZone_Commodity();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 5M\\XAUUSD_5.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }

	private static void adjustRates_XAGUSD_60M(){
		try {
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\XAGUSD_60M\\XAGUSD_Candlestick_1_Hour_BID_01.05.2014-01.05.2015.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\XAGUSD_60M\\XAGUSD_Candlestick_1_Hour_BID_01.05.2015-01.05.2017.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\XAGUSD_60M\\XAGUSD_Candlestick_1_Hour_BID_01.05.2017-01.05.2018.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\XAGUSD_60M\\XAGUSD_Candlestick_1_Hour_BID_01.05.2018-01.05.2019.csv");


			List<Rates> rates = model.adjustTimeZone_Commodity();

			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\XAGUSD_60M\\XAGUSD_60.csv",rates);

		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
	}

	private static void adjustRates_USTECUSD_60M(){
		try {
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\USTECUSD_60M\\USATECH.IDXUSD_Candlestick_1_Hour_BID_01.01.2014-01.04.2018.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\USTECUSD_60M\\USATECH.IDXUSD_Candlestick_1_Hour_BID_01.01.2018-01.04.2020.csv");


			List<Rates> rates = model.adjustTimeZone_Commodity();

			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive (oceanxplorertechnology@gmail.com)\\FX\\NTS Batch\\history\\Data gap\\USTECUSD_60M\\USTECUSD_60.csv",rates);

		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
	}
    
    private static void adjustRates_XAUUSD_15M(){
    	try {
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 15M\\XAUUSD_15_2013_2016.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 15M\\XAUUSD_15_2016_2018.csv");
    		
			List<Rates> rates = model.adjustTimeZone_Commodity();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 15M\\XAUUSD_15.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void adjustRates_XAUUSD(){
    	try {
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2018.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2017.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2016.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2015.csv");
    		//fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2014.csv");
			//fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2013.csv");			
			
			List<Rates> rates = model.adjustTimeZone_Commodity();
			
			fileService.writeNtsRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1.csv",rates);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void checkRates_GBPJPY_5M(){
    	try {
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 5M\\GBPJPY_5_2013.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 5M\\GBPJPY_5_2015.csv");			
			
			//Find out timezone change period
			
			model.checkDailyRates(5);
			
			fileService.generateDailyCheckReport("C:\\Users\\amo31\\Google Drive\\FX\\Share\\GBPJPY 5M\\GBPJPY_5_dailycheck.csv");
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
    
    private static void checkRates(){
    	try {
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2018.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2017.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2016.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2015.csv");
    		fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2014.csv");
			fileService.readHistoricalRateCSV("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_2013.csv");			
			
			//Find out timezone change period
			
			model.checkDailyRates(1);
			
			fileService.generateDailyCheckReport("C:\\Users\\amo31\\Google Drive\\FX\\Share\\XAUUSD 1M\\XAUUSD_1_dailycheck.csv");
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
	private static void run(){
		Map<String,Double> sRisks = new HashMap<String,Double>();


		// Expected risk / Based risk
		// Base case: 1
		sRisks.put("841005", 0.2*1/1.0);
		//sRisks.put("842001", 0.2*1/1.0);
		sRisks.put("860006", 2.0*1.0/1.0);
		sRisks.put("860007", 1.8*1.0/1.0);
		sRisks.put("860011", 0.4*1.0/1.0);
		sRisks.put("860008", 0.2*1.0/1.0);
		sRisks.put("500002", 1.8*1.0/1.0);
		sRisks.put("900002", 1.8*1.0/1.0);
		sRisks.put("860001", 1.0*0.3/0.3);
		sRisks.put("860002", 0.2*0.3/0.3);
		sRisks.put("860013", 1.8*1.0/1.0);
		//sRisks.put("500008", 1.0*1.0/1.0);



		run(sRisks,false);
	}

    private static void run2(){
    	Map<String,Double> sRisks = new HashMap<String,Double>();
		
		// Expected risk / Based risk
    	// Base case: 1
		sRisks.put("841005", 0.2*1/1.0);
		//sRisks.put("842001", 0.2*1/1.0);
		sRisks.put("860006", 2.2*1.0/1.0);
		sRisks.put("860007", 1.4*1.0/1.0);
		sRisks.put("860011", 1.0*1.0/1.0);
		sRisks.put("860008", 0.2*1.0/1.0);
		sRisks.put("500002", 2.0*1.0/1.0);
		sRisks.put("900002", 1.8*1.0/1.0);
		sRisks.put("860001", 1.0*0.3/0.3);
		sRisks.put("860002", 0.2*0.3/0.3);

		run(sRisks,false);
    }

	private static void todayOrderChecking() throws IOException {
    	double equityMT4 = fileService.readEquityCSV(configReader.getPropValues("PortfolioOpenOrders_Location") + "Equity.csv");
		Instant previousClosedDate = ZonedDateTime.now().truncatedTo(ChronoUnit.DAYS).minusDays(1).toInstant();
		//Instant currentDate = ZonedDateTime.now().truncatedTo(ChronoUnit.DAYS).toInstant();

		List<Results> openOrdersResultsFromTesting = model.getAdjustedData(true).stream()
				.filter(d->d.profit == 0 && d.closePrice == 0)
				.collect(Collectors.toList());
		openOrdersResultsFromTesting.forEach(o->o.lots = o.lots/(o.balance/equityMT4));

		List<Results> openOrdersResultsFromMT4 = fileService.readOpenOrdersCSV(configReader.getPropValues("PortfolioOpenOrders_Location") + "OpenOrders.csv");

		ordersResultsFromTesting(openOrdersResultsFromTesting,openOrdersResultsFromMT4,"misMatchedResult",false);
		//Compare last two days orders

		List<Results> closedOrdersResultsFromTesting = model.getAdjustedData(true).stream().filter(d->d.profit != 0 && d.closeTime.toInstant().isAfter(previousClosedDate) ).collect(Collectors.toList());
		closedOrdersResultsFromTesting.forEach(o->o.lots = o.lots/(o.balance/equityMT4));

		List<Results> closedOrdersResultsFromMT4 =
				fileService.readClosedOrdersCSV(configReader.getPropValues("PortfolioOpenOrders_Location") + "ClosedOrders.csv")
				.stream().filter(d->d.closeTime.toInstant().isAfter(previousClosedDate)).collect(Collectors.toList());

		ordersResultsFromTesting(closedOrdersResultsFromTesting,closedOrdersResultsFromMT4,"misMatchedClosedResult",true);

	}

    private static void ordersResultsFromTesting(List<Results> ordersResultsFromTesting,List<Results> ordersResultsFromMT4,String fileName,boolean isClosedOrder) throws IOException {
		//Checking
		// 1. group by strategy ID
		Map<String, List<Results>> OpenOrdersGroupsFromMT4 =
				ordersResultsFromMT4.stream().collect(Collectors.groupingBy(d->d.strategyID))
						.entrySet().stream().sorted(Map.Entry.comparingByKey())
						.collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue,
								(oldValue, newValue) -> oldValue, LinkedHashMap::new));

		Map<String, List<Results>> openOrdersGroupsFromTesting =
				ordersResultsFromTesting.stream().collect(Collectors.groupingBy(d->d.strategyID))
						.entrySet().stream().sorted(Map.Entry.comparingByKey())
						.collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue,
								(oldValue, newValue) -> oldValue, LinkedHashMap::new));

		Map<String,String> misMatchedOrderGroups = new HashMap<>();
		List<String> matchedStrategy = new ArrayList<>();

		for (Map.Entry<String, List<Results>> orderGroup : OpenOrdersGroupsFromMT4.entrySet()) {
			if("0".equals(orderGroup.getKey())){
				misMatchedOrderGroups.put(orderGroup.getKey(),"Manual orders are not generated from testing!");
				continue;
			}

			if(openOrdersGroupsFromTesting.containsKey(orderGroup.getKey())){
				List<Results> testingResults = openOrdersGroupsFromTesting.get(orderGroup.getKey());
				//Comparing order group

				if(orderGroup.getValue().size() != testingResults.size()){
					misMatchedOrderGroups.put(orderGroup.getKey(),"Order size misMatched!");
					continue;
				}

				//only keep the biggest lots order

				Results maxOrderFromMT4 = orderGroup.getValue().stream().max(Comparator.comparing(r->r.lots)).orElseThrow(NoSuchElementException::new);
				Results maxOrderFromTesting = testingResults.stream().max(Comparator.comparing(r->r.lots)).orElseThrow(NoSuchElementException::new);

				if(compareOrders(maxOrderFromTesting,maxOrderFromMT4,misMatchedOrderGroups,isClosedOrder))
					matchedStrategy.add(orderGroup.getKey());

			}
			else{
				misMatchedOrderGroups.put(orderGroup.getKey(),String.format("strategy %s doesn't generated orders from testing! Details:\n%s",
						orderGroup.getKey(),
						orderGroup.getValue().stream().map(Results::toString).collect(Collectors.joining("\n"))));
			}
		}

		for (Map.Entry<String, List<Results>> orderGroup : openOrdersGroupsFromTesting.entrySet()) {
			if(matchedStrategy.contains(orderGroup.getKey()) || misMatchedOrderGroups.containsKey(orderGroup.getKey()))
				continue;
			if(!OpenOrdersGroupsFromMT4.containsKey(orderGroup.getKey())){
				misMatchedOrderGroups.put(orderGroup.getKey(),String.format("strategy %s doesn't generated orders from MT4! Details:\n%s",
						orderGroup.getKey(),
						orderGroup.getValue().stream().map(Results::toString).collect(Collectors.joining("\n"))));
			}
		}

		fileService.writeMismatchedFile(configReader.getPropValues("PortfolioResult_Location") + fileName,misMatchedOrderGroups);

	}

	private static boolean isMatchedPrice(double testingPrice, double mt4Price){
		return Math.abs(testingPrice - mt4Price) <= mt4Price * 0.01 * 0.5;
	}

	private static boolean isMatchedLots(double testingLots, double mt4Lots){
		return Math.abs(testingLots - mt4Lots) <= mt4Lots * 0.3;
	}

	private static boolean compareOrders(Results orderFromTesting, Results orderFromMT4, Map<String, String> misMatched,boolean isClosedOrder){
		if(!orderFromTesting.strategyID.equals(orderFromMT4.strategyID)) {
			misMatched.put(orderFromMT4.strategyID, "unMatched strategy ID.");
			return false;
		}

		if(!orderFromTesting.orderType.equals(orderFromMT4.orderType)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched order type: testing open price %s-> MT4 open price %s", orderFromTesting.orderType, orderFromMT4.orderType));
			return false;
		}

		if(!isMatchedPrice(orderFromTesting.openPrice,orderFromMT4.openPrice)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched open price: testing open price %.5f-> MT4 open price %.5f", orderFromTesting.openPrice, orderFromMT4.openPrice));
			return false;
		}

		if(isClosedOrder && !isMatchedPrice(orderFromTesting.closePrice,orderFromMT4.closePrice)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched close price: testing close price %.5f-> MT4 close price %.5f", orderFromTesting.closePrice, orderFromMT4.closePrice));
			return false;
		}

		//double level = orderFromTesting.balance / orderFromMT4.balance;
		if(!isMatchedLots(orderFromTesting.lots,orderFromMT4.lots)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched lots: testing lots %.3f->MT4 lots %.3f", orderFromTesting.lots, orderFromMT4.lots));
			return false;
		}

		if(!isMatchedPrice(orderFromTesting.stopLossPrice,orderFromMT4.stopLossPrice)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched stopLossPrice: testing stopLossPrice %.5f-> MT4 stopLossPrice %.5f", orderFromTesting.stopLossPrice, orderFromMT4.stopLossPrice));
			return false;
		}

		if(!isMatchedPrice(orderFromTesting.takeProfitPrice,orderFromMT4.takeProfitPrice)) {
			misMatched.put(orderFromMT4.strategyID,
					String.format("unMatched takeProfitPrice: testing takeProfitPrice %.5f-> MT4 takeProfitPrice %.5f", orderFromTesting.takeProfitPrice, orderFromMT4.takeProfitPrice));
			return false;
		}

		return true;
	}

	private static void run(Map<String,Double> sRisks,boolean fromOptimized){
    	run(sRisks,fromOptimized,false);
	}

    private static void run(Map<String,Double> sRisks,boolean fromOptimized,boolean isCheckingOpenOrder){
	
    	model.initModelData(sRisks,true);
    	
		try {
			sRisks.keySet().stream().forEach(id-> {

				try {
					fileService.readCSV(configReader.getPropValues("PortfolioResult_Location") + String.format("results_%s.txt",id),id);
				} catch (IOException e) {
					e.printStackTrace();
				}

			});

			if(!fromOptimized) {
				fileService.writeCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioResult_adjusted",true);
			}

			if(isCheckingOpenOrder){
				todayOrderChecking();
			}
			
			//generate weekly and monthly report
			fileService.generateWeelyReport(configReader.getPropValues("PortfolioResult_Location") + "portfolioWeeklyResult_adjusted",true);
			fileService.generateMonthlyReport(configReader.getPropValues("PortfolioResult_Location") + "portfolioMonthlyResult_adjusted",true);
			
			statisticsService.caculate(ModelData.INITBALANCE,true);	
			
			//Write all statiscs
			fileService.writeStatiscsListCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioStatistics_adjusted",false);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }
	private static void run_custom(String pair){
		Map<String,Double> sRisks = new HashMap<String,Double>();
    	if("US500".equals(pair)) { //it is ok for now,but not for long term.
			sRisks.put("900001", 0.5*1.0/1.0);
			sRisks.put("500001", 1.0/1.0);
			runCustom(sRisks,"400001");
		}
		else if("GBPAUD".equals(pair)) {
			sRisks.put("860012", 0.5/1.0); //it doesn't help with portfolio.
			sRisks.put("500002", 1.0/1.0);
			runCustom(sRisks,"400002");
		}
		else if("GBPJPY".equals(pair)) {
			sRisks.put("860006", 1.0/1.0);
			sRisks.put("841005", 1.0/1.0);
			sRisks.put("860002", 0.7*1.0/1.0);
			runCustom(sRisks,"400003");
		}
		else if("XAUUSD".equals(pair)) {
			sRisks.put("860007", 1.0/1.0);
			sRisks.put("842001", 0.7*1.0/1.0);
			sRisks.put("860003", 0.5*1.0/1.0);
			runCustom(sRisks,"400004");
		}
		else if("XAUUSD_5M".equals(pair)) {
			sRisks.put("860001", 0.5*1.0/1.0);
			sRisks.put("860003", 0.5*1.0/1.0);
			runCustom(sRisks,"400005");
		}
		else if("BTCUSD".equals(pair)) {
			sRisks.put("300002", 0.6*1.0/1.0);
			sRisks.put("200002", 1*1.0/1.0);
			runCustom(sRisks,"400002");
		}
		else if("Limit".equals(pair)) {
			//sRisks.put("200002", 1.0/1.0);
			sRisks.put("200003", 1.0/1.0);
			sRisks.put("200005", 1.0/1.0);
			sRisks.put("200007", 1.0/1.0);
			sRisks.put("200009", 1.0/1.0);
			runCustom(sRisks,"220000");
		}

	}

    private static void runCustom(Map<String,Double> sRisks,String newStrategyId){

    	model.initModelData(sRisks,true);
    	
		try {

			sRisks.keySet().stream().forEach(id-> {
				try {
					fileService.readCSV(configReader.getPropValues("PortfolioResult_Location") + String.format("results_%s.txt",id),id);
				} catch (IOException e) {
					e.printStackTrace();
				}
			});


			fileService.writeCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioResult_adjusted",true);
			fileService.writeNewStrategyCVS(configReader.getPropValues("PortfolioResult_Location") + "results_"+newStrategyId,true);


			//generate weekly and monthly report
			fileService.generateWeelyReport(configReader.getPropValues("PortfolioResult_Location") + "portfolioWeeklyResult_adjusted",true);
			fileService.generateMonthlyReport(configReader.getPropValues("PortfolioResult_Location") + "portfolioMonthlyResult_adjusted",true);

			statisticsService.caculate(ModelData.INITBALANCE,true);

			//Write all statiscs
			fileService.writeStatiscsListCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioStatistics_adjusted",false);

		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}
    }

	private static void runOptimizerLevel2(){
		Map<String,Double> sRisks = new HashMap<>();

		// Expected risk / Based risk
		// Base case: 1
		sRisks.put("500002", 1.0/1.0);
		sRisks.put("400003", 1.0/1.0);
		sRisks.put("400004", 1.0/1.0);
		sRisks.put("860011", 1.0/1.0);
		sRisks.put("900002", 1.0/1.0);

		List<String> strategy = sRisks.keySet().stream().sorted().collect(Collectors.toList());


		//List<Double> run = new ArrayList<>(  Arrays.asList(0.2,0.5,0.8,1.0,1.2,1.5,1.8));
		List<Double> run = new ArrayList<>(  Arrays.asList(0.2,0.4,1.0,1.4,1.8));

		model.initModelData(sRisks,true);

		try {
			sRisks.keySet().stream().forEach(id-> {
				try {
					fileService.readCSV(configReader.getPropValues("PortfolioResult_Location") + String.format("results_%s.txt",id),id);
				} catch (IOException e) {
					e.printStackTrace();
				}
			});

			fileService.writeCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioResult_adjusted.csv",true);

			//base case:
			statisticsService.caculate(ModelData.INITBALANCE,true);

			//Brutal optimizer
			sRisks.clear();

			for (int i1 = 0; i1 < run.size(); i1++) {
				sRisks.put(strategy.get(0), run.get(i1));
				for (int i2 = 0; i2 < run.size(); i2++) {
					sRisks.put(strategy.get(1), run.get(i2));
					for (int i3 = 0; i3 < run.size(); i3++) {
						sRisks.put(strategy.get(2), run.get(i3));
						for (int i4 = 0; i4 < run.size(); i4++) {
							sRisks.put(strategy.get(3), run.get(i4));
							for (int i5 = 0; i5 < run.size(); i5++) {
								sRisks.put(strategy.get(4), run.get(i5));

								model.addStrategyList(sRisks);
								statisticsService.caculate(ModelData.INITBALANCE, true);
							}

						}
					}
				}
			}


			//Write all statiscs
			fileService.writeStatiscsListCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioStatistics_optimize_adjusted.csv",true);

		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}

		List<Statistics> sList = statisticsService.selectModels(true);

		//Run the top model
		run(sList.get(0).strategyRisk,true);
	}
    
    private static void run_optimizer(Map<String,Double> sRisks,Map<String,Double> predefinedStrategies){

		List<String> strategy = sRisks.keySet().stream().sorted().filter(id->!predefinedStrategies.keySet().contains(id)).collect(Collectors.toList());
		
	
		//List<Double> run = new ArrayList<>(  Arrays.asList(0.2,0.5,0.8,1.0,1.2,1.5,1.8));
		List<Double> run = new ArrayList<>(  Arrays.asList(0.2,0.4,1.0,1.4,1.8,2.0,2.2));
		
    	model.initModelData(sRisks,true);
    	
		try {
			sRisks.keySet().stream().forEach(id-> {
				try {
					fileService.readCSV(configReader.getPropValues("PortfolioResult_Location") + String.format("results_%s.txt",id),id);
				} catch (IOException e) {
					e.printStackTrace();
				}
			});

			fileService.writeCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioResult_adjusted.csv",true);
			
			//base case:
			statisticsService.caculate(ModelData.INITBALANCE,true);	
			
			//Brutal optimizer
			sRisks.clear();			
						
			for (int i1 = 0; i1 < run.size(); i1++) {
				 sRisks.put(strategy.get(0), run.get(i1));
		            for (int i2 = 0; i2 < run.size(); i2++) {
		            	sRisks.put(strategy.get(1), run.get(i2));
		            	for (int i3 = 0; i3 < run.size(); i3++) {
		            		sRisks.put(strategy.get(2), run.get(i3));	            			
		            		for (int i4 = 0; i4 < run.size(); i4++) {
		            			sRisks.put(strategy.get(3), run.get(i4));
		            			for (int i5 = 0; i5 < run.size(); i5++) {
			            			sRisks.put(strategy.get(4), run.get(i5));
			            			for (int i6 = 0; i6 < run.size(); i6++) {
				            			sRisks.put(strategy.get(5), run.get(i6));
//											for (int i7 = 0; i7 < run.size(); i7++) {
//												sRisks.put(strategy.get(6), run.get(i7));

//												sRisks.put("841005", 0.2);
//												sRisks.put("900002", 1.8);
//												sRisks.put("860006", 2.2);
//												sRisks.put("842001", 0.0);

												sRisks.putAll(predefinedStrategies);

												model.addStrategyList(sRisks);
												statisticsService.caculate(ModelData.INITBALANCE, true);

				            			}
//			            			}
		            			}
		            		}
		            	}
		            }	           					    
		            			            
		        }
			
	
			//Write all statiscs
			fileService.writeStatiscsListCVS(configReader.getPropValues("PortfolioResult_Location") + "portfolioStatistics_optimize_adjusted.csv",true);
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}	
		
		List<Statistics> sList = statisticsService.selectModels(true);
		
		//Run the top model
		run(sList.get(0).strategyRisk,true);
    }
    
    
    private static void run_optimizer_withoutDailyTrading(){
    	Map<String,Double> sRisks = new HashMap<String,Double>();
		
		// Expected risk / Based risk
    	// Base case: 1
		sRisks.put("840005", 0.3/0.3);		
		sRisks.put("841005", 1.0/1.0);
		sRisks.put("842001", 1.0/1.0);
		sRisks.put("850001", 0.8/0.8);
	
    	model.initModelData(sRisks,true);
    	
		try {
			fileService.readCSV("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\840005.txt","840005");
			fileService.readCSV("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\841005.txt","841005");			
			fileService.readCSV("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\842001.txt","842001");
			fileService.readCSV("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\850001.txt","850001");
			
			fileService.writeCVS("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\portfolioResult_adjusted.csv",true);
			
			//base case:
			statisticsService.caculate(ModelData.INITBALANCE,true);	
			
			//Brutal optimizer
			sRisks.clear();
			
			List<Double> run = new ArrayList<Double>(  Arrays.asList(0.5,0.8,1.0,1.2,1.5));
			
			for (int i1 = 0; i1 < run.size(); i1++) {
				 sRisks.put("840005", run.get(i1));
		            for (int i2 = 0; i2 < run.size(); i2++) {
		            	sRisks.put("841005", run.get(i2));
		            	for (int i3 = 0; i3 < run.size(); i3++) {
		            		sRisks.put("842001", run.get(i3));
		            		for (int i4 = 0; i4 < run.size(); i4++) {
		            			sRisks.put("850001", run.get(i4));		            			
		            			// one run here:
		            			model.addStrategyList(sRisks);
		            			statisticsService.caculate(ModelData.INITBALANCE,true);
		            		}
		            	}
		            }
		        }
			
//			sRisks.put("840005", 0.3/0.3);		
//			sRisks.put("841005", 1.04/1.0);
//			sRisks.put("842001", 0.96/1.0);
//			sRisks.put("850001", 0.65/0.8);
//			sRisks.put("841008", 0.6/0.3);
//			sRisks.put("840007", 0.46/0.3);
//			model.addStrategyList(sRisks);
//			
//			statisticsService.caculate(ModelData.INITBALANCE,true);
			
			//Write all statiscs
			fileService.writeStatiscsListCVS("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\portfolioStatistics_adjusted.csv",true);
			
//			fileService.writeCVS("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\portfolioResult_original.csv",false);			
//			statisticsService.caculate(ModelData.INITBALANCE,false);			
//			fileService.writeStatiscsCVS("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\portfolioStatistics_original.csv");
			
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}	
    }
    
    private static void run_optimizer_DailyTrading(){
    	Map<String,Double> sRisks = new HashMap<String,Double>();
		
		// Expected risk / Based risk
    	// Base case: 1
		sRisks.put("860001", 0.3/0.3);		
		sRisks.put("860003", 0.3/0.3);
		sRisks.put("860002", 0.3/0.3);		
	
    	model.initModelData(sRisks,true);
    	
		try {
			fileService.readCSV("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\860001.txt","860001");
			fileService.readCSV("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\860003.txt","860003");			
			fileService.readCSV("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\860002.txt","860002");
			
			
			fileService.writeCVS("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\portfolioResult_adjusted.csv",true);
			
			//base case:
			statisticsService.caculate(ModelData.INITBALANCE,true);	
			
			//Brutal optimizer
			sRisks.clear();
			
			List<Double> run = new ArrayList<Double>(  Arrays.asList(0.2,0.5,0.8,1.0,1.2,1.5,1.8));
			
			for (int i1 = 0; i1 < run.size(); i1++) {
				 sRisks.put("860001", run.get(i1));
		            for (int i2 = 0; i2 < run.size(); i2++) {
		            	sRisks.put("860003", run.get(i2));
		            	for (int i3 = 0; i3 < run.size(); i3++) {
		            		sRisks.put("860002", run.get(i3));
		            		model.addStrategyList(sRisks);
		            		statisticsService.caculate(ModelData.INITBALANCE,true);		            		
		            	}
		            }
		        }
			//Write all statiscs
			fileService.writeStatiscsListCVS("C:\\Users\\amo31\\Google Drive\\FX\\Trading\\Kelpie Project\\Portfolio Test\\PortfolioResult\\portfolioStatistics_adjusted.csv",true);
			
			
		} catch (IOException e) {
			logger.error(e.getMessage(),e);
		}	
    }
    
}
