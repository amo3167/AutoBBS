package service;

import java.io.IOException;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.temporal.ChronoUnit;
import java.util.Date;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import model.Rates;

/**
 * Implementation of {@link MT4ConversionService} for converting and merging MT4 rate data.
 * 
 * <p>This service handles format conversions, incremental merges, and validation
 * with weekend and holiday adjustments.</p>
 * 
 * @version 1.0
 * @since 2021
 */
public class MT4ConversionServiceImpl implements MT4ConversionService {
	private static final Logger logger = LogManager.getLogger(MT4ConversionServiceImpl.class);
	
	private static final int MINUTES_PER_DAY = 1440;
	private static final double DEFAULT_DISCOUNT_RATE = 0.01; // 1% minimum data threshold
	private static final double XAU_DISCOUNT_RATE = 0.1;      // 10% for precious metals
	
	private final FileService fileService;
	
	/**
	 * Creates a new MT4ConversionServiceImpl.
	 * 
	 * @param fileService File service for reading and writing rate data
	 */
	public MT4ConversionServiceImpl(FileService fileService) {
		this.fileService = fileService;
	}
	
	/**
	 * Converts MT4 rate format to NTS rate format, appending only new rates.
	 * 
	 * <p>Reads both files, identifies the last date in the NTS file, and appends
	 * all MT4 rates that are newer than that date.</p>
	 * 
	 * @param mt4RateFile Path to MT4 format rate file (source)
	 * @param ntsRateFile Path to NTS format rate file (destination)
	 * @throws IOException if file operations fail
	 */
	public void convertMT4ToNTS(String mt4RateFile, String ntsRateFile) throws IOException {
		logger.info("Converting MT4 rates from {} to NTS format: {}", mt4RateFile, ntsRateFile);
		
		List<Rates> mt4Rates = fileService.readMt4RateCSV(mt4RateFile);
		List<Rates> ntsRates = fileService.readNtsRateCSV(ntsRateFile);

		Optional<Date> lastDateTime = ntsRates.stream()
				.map(rate -> rate.datetime)
				.max(Date::compareTo);

		long addedCount = mt4Rates.stream()
				.filter(rate -> !lastDateTime.isPresent() || rate.datetime.after(lastDateTime.get()))
				.peek(ntsRates::add)
				.count();

		fileService.writeNtsRateCSV(ntsRateFile, ntsRates);
		logger.info("Converted {} new rates from MT4 to NTS format", addedCount);
	}
	
	/**
	 * Merges new MT4 rate data into an existing rate file with validation.
	 * 
	 * <p>This method validates that the new data contains a sufficient number of bars
	 * relative to the time elapsed. It accounts for weekend gaps and applies different
	 * discount rates for precious metals (XAU) versus other instruments.</p>
	 * 
	 * <p>Validation logic:</p>
	 * <ul>
	 *   <li>Calculates expected bars based on time elapsed minus weekends</li>
	 *   <li>Applies instrument-specific discount rate (1% default, 10% for XAU)</li>
	 *   <li>Merges only if actual bars >= minimum expected bars</li>
	 *   <li>Writes error file if validation fails</li>
	 * </ul>
	 * 
	 * @param mt4RateFile Path to the existing MT4 rate file to update
	 * @param newMt4RateFile Path to the file containing new rates to merge
	 * @param errorFile Path to write error messages if validation fails
	 * @param timeFrame Time frame in minutes (e.g., 60 for 1-hour bars)
	 * @throws IOException if file operations fail
	 */
	public void mergeMT4Rates(String mt4RateFile, String newMt4RateFile, String errorFile, int timeFrame) throws IOException {
		logger.info("Merging MT4 rate files: {} + {} -> {}", mt4RateFile, newMt4RateFile, mt4RateFile);
		
		List<Rates> currentRates = fileService.readMt4RateCSV(mt4RateFile);
		List<Rates> newRates = fileService.readMt4RateCSV(newMt4RateFile);
		
		Optional<Date> lastDateTime = currentRates.stream()
				.map(rate -> rate.datetime)
				.max(Date::compareTo);

		Optional<Date> lastNewDateTime = newRates.stream()
				.map(rate -> rate.datetime)
				.max(Date::compareTo);

		if (!lastDateTime.isPresent() || !lastNewDateTime.isPresent()) {
			logger.warn("Cannot merge - one or both rate files are empty");
			return;
		}
		
		if (lastNewDateTime.get().before(lastDateTime.get()) || lastNewDateTime.get().equals(lastDateTime.get())) {
			logger.info("New rates do not extend beyond existing rates - no merge needed");
			return;
		}
		
		LocalDateTime start = lastDateTime.get().toInstant().atZone(ZoneId.systemDefault()).toLocalDateTime();
		LocalDateTime end = lastNewDateTime.get().toInstant().atZone(ZoneId.systemDefault()).toLocalDateTime();
		logger.info("Merging rates from {} to {}", start, end);

		int weekendDays = countWeekendDays(start, end);
		double discountRate = mt4RateFile.contains("XAU") ? XAU_DISCOUNT_RATE : DEFAULT_DISCOUNT_RATE;
		
		// Calculate expected number of bars accounting for weekends
		long totalMinutes = ChronoUnit.MINUTES.between(start, end);
		long tradingMinutes = totalMinutes - (weekendDays * MINUTES_PER_DAY);
		double maxBars = (double) tradingMinutes / timeFrame;
		double minBars = maxBars * discountRate;

		List<Rates> addRates = newRates.stream()
				.filter(rate -> rate.datetime.after(lastDateTime.get()))
				.collect(Collectors.toList());
				
		long mergedBars = addRates.size();
		logger.info("Max bars: {}, Min bars: {}, Merged bars: {}", maxBars, minBars, mergedBars);

		if (mergedBars >= minBars || mergedBars == maxBars) {
			currentRates.addAll(addRates);
			fileService.writeMT4RateCSV(mt4RateFile, currentRates);
			logger.info("Successfully merged {} bars into {}", mergedBars, mt4RateFile);
		} else {
			double missingBars = Math.min(maxBars, minBars) - mergedBars;
			String errorMessage = String.format("Insufficient data: missing at least %.0f bars", missingBars);
			logger.error(errorMessage);
			fileService.writeToErrorFile(errorFile, errorMessage);
		}
	}
	
	/**
	 * Calculates the number of weekend days between two dates.
	 * 
	 * @param start Start date
	 * @param end End date
	 * @return Number of Saturday and Sunday days in the range
	 */
	private int countWeekendDays(LocalDateTime start, LocalDateTime end) {
		int count = 0;
		LocalDateTime current = start;
		while (end.isAfter(current)) {
			if (current.getDayOfWeek() == java.time.DayOfWeek.SATURDAY || 
			    current.getDayOfWeek() == java.time.DayOfWeek.SUNDAY) {
				count++;
			}
			current = current.plusDays(1);
		}
		return count;
	}
}
