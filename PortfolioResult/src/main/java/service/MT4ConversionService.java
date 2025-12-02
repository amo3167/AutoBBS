package service;

import java.io.IOException;

/**
 * Service interface for converting and merging MT4 rate data files.
 * 
 * <p>This service handles:</p>
 * <ul>
 *   <li>Converting MT4 rate format to NTS format</li>
 *   <li>Merging incremental MT4 rate updates into existing files</li>
 *   <li>Validating data completeness with weekend and holiday adjustments</li>
 * </ul>
 * 
 * @version 1.0
 * @since 2021
 */
public interface MT4ConversionService {
	
	/**
	 * Converts MT4 rate file format to NTS format.
	 * 
	 * <p>Reads MT4-formatted rate data and appends it incrementally to the NTS output file.
	 * Handles date parsing and format conversion for compatibility with the NTS system.</p>
	 * 
	 * @param mt4RateFile   path to the input MT4 rate file
	 * @param ntsRateFile   path to the output NTS rate file (appended to if exists)
	 * @throws IOException if file reading or writing fails
	 */
	void convertMT4ToNTS(String mt4RateFile, String ntsRateFile) throws IOException;
	
	/**
	 * Merges new MT4 rate data into an existing MT4 rate file with validation.
	 * 
	 * <p>Validates that new rates follow chronologically and checks for data completeness
	 * considering weekends and holidays. Uses instrument-specific discount rates for
	 * validation tolerance (1% for most instruments, 10% for precious metals).</p>
	 * 
	 * <p>Validation accounts for:</p>
	 * <ul>
	 *   <li>Weekend gaps (Friday close to Monday open)</li>
	 *   <li>Holiday periods with extended market closures</li>
	 *   <li>Instrument-specific price movement tolerances</li>
	 * </ul>
	 * 
	 * @param mt4RateFile      path to the existing MT4 rate file
	 * @param newMt4RateFile   path to the new MT4 rate data to merge
	 * @param errorFile        path to write validation errors (if any)
	 * @param timeFrame        candle timeframe in minutes (e.g., 5 for 5-minute candles)
	 * @throws IOException if file reading or writing fails
	 */
	void mergeMT4Rates(String mt4RateFile, String newMt4RateFile, String errorFile, int timeFrame) 
			throws IOException;
}
