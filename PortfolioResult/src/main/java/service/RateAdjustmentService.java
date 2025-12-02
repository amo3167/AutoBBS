package service;

import java.io.IOException;

/**
 * Service interface for adjusting historical rate data timezone and format conversions.
 * 
 * <p>This service handles timezone adjustments for various currency pairs and commodities,
 * converting between different data formats (MT4, NTS) and consolidating multi-year
 * historical data files.</p>
 * 
 * <p>Supported instruments:</p>
 * <ul>
 *   <li>Forex pairs: EUR/USD, EUR/GBP (5-minute data)</li>
 *   <li>Commodities: XAG/USD (Silver), USTECH (NASDAQ-100) (60-minute data)</li>
 * </ul>
 * 
 * @version 1.0
 * @since 2021
 */
public interface RateAdjustmentService {
	
	/**
	 * Adjusts 5-minute EUR/USD rate data across multiple years.
	 * 
	 * <p>Processes historical data from 2017-2021, performing timezone adjustments
	 * and consolidating into a single output file.</p>
	 * 
	 * @param baseDirectory  the base directory containing year subdirectories with rate files
	 * @throws IOException if file reading or writing fails
	 */
	void adjustEURUSD_5M(String baseDirectory) throws IOException;
	
	/**
	 * Adjusts 5-minute EUR/GBP rate data across multiple years.
	 * 
	 * <p>Processes historical data from 2015-2021, performing timezone adjustments
	 * and consolidating into a single output file.</p>
	 * 
	 * @param baseDirectory  the base directory containing year subdirectories with rate files
	 * @throws IOException if file reading or writing fails
	 */
	void adjustEURGBP_5M(String baseDirectory) throws IOException;
	
	/**
	 * Adjusts 60-minute XAG/USD (Silver) rate data across multiple years.
	 * 
	 * <p>Processes hourly silver spot price data from 2014-2019 with CME market hours
	 * adjustments (Sunday 18:00 to Friday 17:00 EST).</p>
	 * 
	 * @param baseDirectory  the base directory containing year subdirectories with rate files
	 * @throws IOException if file reading or writing fails
	 */
	void adjustXAGUSD_60M(String baseDirectory) throws IOException;
	
	/**
	 * Adjusts 60-minute USTEC/USD (NASDAQ-100) rate data across multiple years.
	 * 
	 * <p>Processes hourly NASDAQ-100 index data from 2014-2020 with timezone
	 * and market hours adjustments.</p>
	 * 
	 * @param baseDirectory  the base directory containing year subdirectories with rate files
	 * @throws IOException if file reading or writing fails
	 */
	void adjustUSTECUSD_60M(String baseDirectory) throws IOException;
}
