package model;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import service.DateTimeHelper;

/**
 * Strategy pattern base class for timezone adjustment logic.
 * Handles DST transitions and market hour filtering for different market types.
 */
abstract class TimeZoneAdjuster {
	
	protected abstract List<String> getHolidays();
	protected abstract List<String> getEarlyCloseDates();
	
	/**
	 * Adjusts timezone for all rates and filters out non-trading periods.
	 * 
	 * @param rateList  list of rates to adjust
	 * @return filtered and adjusted rates
	 */
	public List<Rates> adjustTimeZone(List<Rates> rateList) {
		List<String> holidays = getHolidays();
		List<String> earlyClose = getEarlyCloseDates();
		List<Rates> result = new ArrayList<>();

		for (Rates rate : rateList) {
			adjustRateDateTime(rate);

			if (shouldFilterRate(rate, holidays, earlyClose)) {
				continue;
			}

			result.add(new Rates(rate));
		}

		return result;
	}

	/**
	 * Adjusts rate datetime based on DST transitions.
	 * 
	 * @param rate  the rate to adjust
	 */
	protected void adjustRateDateTime(Rates rate) {
		int hours = getDSTAdjustmentHours(rate.datetime);
		rate.datetime = DateTimeHelper.addHours(rate.datetime, hours);
	}

	/**
	 * Determines DST adjustment hours based on date.
	 * 
	 * @param datetime  the date to check
	 * @return hours to add (2 or 3 depending on DST)
	 */
	protected abstract int getDSTAdjustmentHours(Date datetime);

	/**
	 * Checks if rate should be filtered out.
	 * 
	 * @param rate        the rate to check
	 * @param holidays    list of holiday dates
	 * @param earlyClose  list of early close dates
	 * @return true if rate should be filtered
	 */
	protected boolean shouldFilterRate(Rates rate, List<String> holidays, List<String> earlyClose) {
		if (DateTimeHelper.isWeekend(rate.datetime)) {
			return true;
		}

		String dateStr = DateTimeHelper.formatDate(rate.datetime, "dd/MM/yy");
		if (holidays.contains(dateStr)) {
			return true;
		}

		return isEarlyCloseFiltered(rate, earlyClose);
	}

	/**
	 * Checks if rate should be filtered due to early close.
	 * 
	 * @param rate        the rate to check
	 * @param earlyClose  list of early close dates
	 * @return true if should be filtered
	 */
	protected boolean isEarlyCloseFiltered(Rates rate, List<String> earlyClose) {
		return false; // Override in subclass if needed
	}

	/**
	 * Helper to check if date is before a threshold.
	 * 
	 * @param date      date to check
	 * @param threshold threshold date string
	 * @param format    date format
	 * @return true if before threshold
	 */
	protected boolean isBefore(Date date, String threshold, String format) {
		return date.before(DateTimeHelper.parseDate(threshold, format));
	}
}
