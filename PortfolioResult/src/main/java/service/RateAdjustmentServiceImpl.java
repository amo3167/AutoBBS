package service;

import java.io.IOException;
import java.util.List;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import model.ModelDataService;
import model.Rates;

/**
 * Implementation of {@link RateAdjustmentService} for adjusting rate data based on timezone and broker-specific offsets.
 * Handles conversion and merging of MT4 rate files with timezone adjustments for various currency pairs and commodities.
 */
public class RateAdjustmentServiceImpl implements RateAdjustmentService {
	
	private static final Logger logger = LogManager.getLogger(RateAdjustmentServiceImpl.class);
	
	// Timezone offset constants (in hours)
	// Note: These constants are kept for documentation purposes but are not currently used
	// as the timezone adjustments are handled by the ModelDataService implementation
	@SuppressWarnings("unused")
	private static final int EURUSD_TZ_OFFSET = 3;
	@SuppressWarnings("unused")
	private static final int EURGBP_TZ_OFFSET = 3;
	@SuppressWarnings("unused")
	private static final int XAGUSD_TZ_OFFSET = 8;
	@SuppressWarnings("unused")
	private static final int USTEC_TZ_OFFSET = 8;
	
	private final ModelDataService model;
	private final FileService fileService;
	
	/**
	 * Creates a new RateAdjustmentServiceImpl.
	 * 
	 * @param model Model data interface for timezone adjustment operations
	 * @param fileService File service for reading and writing rate data
	 */
	public RateAdjustmentServiceImpl(ModelDataService model, FileService fileService) {
		this.model = model;
		this.fileService = fileService;
	}
	
	/**
	 * Adjusts timezone for EUR/USD 5-minute historical rate data.
	 * 
	 * <p>Reads multiple year files (2017-2021), adjusts timezone from broker time
	 * to system time, and writes consolidated output.</p>
	 * 
	 * @param baseDirectory Base directory containing EUR/USD rate files
	 * @throws IOException if file operations fail
	 */
	public void adjustEURUSD_5M(String baseDirectory) {
		try {
			logger.info("Adjusting EUR/USD 5M rates from directory: {}", baseDirectory);
			
			fileService.readHistoricalRateCSV(baseDirectory + "EURUSD_Candlestick_5_M_BID_02.01.2017-01.01.2018.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "EURUSD_Candlestick_5_M_BID_02.01.2018-01.01.2019.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "EURUSD_Candlestick_5_M_BID_02.01.2019-02.01.2020.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "EURUSD_Candlestick_5_M_BID_02.01.2020-01.01.2021.csv");

			List<Rates> adjustedRates = model.adjustTimeZone();
			fileService.writeNtsRateCSV(baseDirectory + "EURUSD_5.csv", adjustedRates);
			
			logger.info("EUR/USD 5M rates adjusted successfully. Output: EURUSD_5.csv");
		} catch (IOException | com.opencsv.exceptions.CsvValidationException e) {
			logger.error("Error adjusting EUR/USD 5M rates: {}", e.getMessage(), e);
			throw new RuntimeException("Failed to adjust EUR/USD rates", e);
		}
	}

	/**
	 * Adjusts timezone for EUR/GBP 5-minute historical rate data.
	 * 
	 * <p>Reads multiple year files (2015-2021), adjusts timezone from broker time
	 * to system time, and writes consolidated output.</p>
	 * 
	 * @param baseDirectory Base directory containing EUR/GBP rate files
	 * @throws IOException if file operations fail
	 */
	public void adjustEURGBP_5M(String baseDirectory) {
		try {
			logger.info("Adjusting EUR/GBP 5M rates from directory: {}", baseDirectory);
			
			fileService.readHistoricalRateCSV(baseDirectory + "EURGBP_Candlestick_5_M_BID_01.01.2015-01.01.2016.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "EURGBP_Candlestick_5_M_BID_01.01.2016-01.01.2018.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "EURGBP_Candlestick_5_M_BID_01.01.2018-01.01.2019.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "EURGBP_Candlestick_5_M_BID_01.01.2019-01.01.2020.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "EURGBP_Candlestick_5_M_BID_01.01.2020-01.05.2021.csv");

			List<Rates> adjustedRates = model.adjustTimeZone();
			fileService.writeNtsRateCSV(baseDirectory + "EURGBP_5.csv", adjustedRates);
			
			logger.info("EUR/GBP 5M rates adjusted successfully. Output: EURGBP_5.csv");
		} catch (IOException | com.opencsv.exceptions.CsvValidationException e) {
			logger.error("Error adjusting EUR/GBP 5M rates: {}", e.getMessage(), e);
			throw new RuntimeException("Failed to adjust EUR/GBP rates", e);
		}
	}
	
	/**
	 * Adjusts timezone for XAG/USD (Silver) 60-minute historical rate data.
	 * 
	 * <p>Reads multiple year files (2014-2019), applies commodity-specific timezone
	 * adjustments (CME market hours), and writes consolidated output.</p>
	 * 
	 * @param baseDirectory Base directory containing XAG/USD rate files
	 * @throws IOException if file operations fail
	 */
	public void adjustXAGUSD_60M(String baseDirectory) {
		try {
			logger.info("Adjusting XAG/USD 60M rates from directory: {}", baseDirectory);
			
			fileService.readHistoricalRateCSV(baseDirectory + "XAGUSD_Candlestick_1_Hour_BID_01.05.2014-01.05.2015.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "XAGUSD_Candlestick_1_Hour_BID_01.05.2015-01.05.2017.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "XAGUSD_Candlestick_1_Hour_BID_01.05.2017-01.05.2018.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "XAGUSD_Candlestick_1_Hour_BID_01.05.2018-01.05.2019.csv");

			List<Rates> adjustedRates = model.adjustTimeZone_Commodity();
			fileService.writeNtsRateCSV(baseDirectory + "XAGUSD_60.csv", adjustedRates);
			
			logger.info("XAG/USD 60M rates adjusted successfully. Output: XAGUSD_60.csv");
		} catch (IOException | com.opencsv.exceptions.CsvValidationException e) {
			logger.error("Error adjusting XAG/USD 60M rates: {}", e.getMessage(), e);
			throw new RuntimeException("Failed to adjust XAG/USD rates", e);
		}
	}

	/**
	 * Adjusts timezone for USTECH/USD (NASDAQ-100) 60-minute historical rate data.
	 * 
	 * <p>Reads multiple year files (2014-2020), applies commodity-specific timezone
	 * adjustments (US market hours), and writes consolidated output.</p>
	 * 
	 * @param baseDirectory Base directory containing USTECH rate files
	 * @throws IOException if file operations fail
	 */
	public void adjustUSTECUSD_60M(String baseDirectory) {
		try {
			logger.info("Adjusting USTECH/USD 60M rates from directory: {}", baseDirectory);
			
			fileService.readHistoricalRateCSV(baseDirectory + "USATECH.IDXUSD_Candlestick_1_Hour_BID_01.01.2014-01.04.2018.csv");
			fileService.readHistoricalRateCSV(baseDirectory + "USATECH.IDXUSD_Candlestick_1_Hour_BID_01.01.2018-01.04.2020.csv");

			List<Rates> adjustedRates = model.adjustTimeZone_Commodity();
			fileService.writeNtsRateCSV(baseDirectory + "USTECUSD_60.csv", adjustedRates);
			
			logger.info("USTECH/USD 60M rates adjusted successfully. Output: USTECUSD_60.csv");
		} catch (IOException | com.opencsv.exceptions.CsvValidationException e) {
			logger.error("Error adjusting USTECH/USD 60M rates: {}", e.getMessage(), e);
			throw new RuntimeException("Failed to adjust USTECH rates", e);
		}
	}
}
