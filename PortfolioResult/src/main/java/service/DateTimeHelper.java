package service;

import java.text.Format;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.Calendar;
import java.util.Date;

/**
 * Utility class for date and time operations.
 * Provides methods for parsing, formatting, and manipulating dates.
 */
public final class DateTimeHelper {
	
	/**
	 * Private constructor to prevent instantiation of utility class.
	 */
	private DateTimeHelper() {
		throw new UnsupportedOperationException("Utility class cannot be instantiated");
	}

	/**
	 * Sets the time component of a date to the end of day (23:59:59.999).
	 * 
	 * @param date the date to modify
	 * @return a new Date at the end of the day
	 */
	public static Date atEndOfDay(Date date) {
		LocalDateTime localDateTime = dateToLocalDateTime(date);
		LocalDateTime endOfDay = localDateTime.with(LocalTime.MAX);
		return localDateTimeToDate(endOfDay);
	}

	/**
	 * Converts a Date to LocalDateTime.
	 * 
	 * @param date the date to convert
	 * @return the LocalDateTime representation
	 */
	private static LocalDateTime dateToLocalDateTime(Date date) {
		return LocalDateTime.ofInstant(date.toInstant(), ZoneId.systemDefault());
	}

	/**
	 * Converts a LocalDateTime to Date.
	 * 
	 * @param localDateTime the LocalDateTime to convert
	 * @return the Date representation
	 */
	private static Date localDateTimeToDate(LocalDateTime localDateTime) {
		return Date.from(localDateTime.atZone(ZoneId.systemDefault()).toInstant());
	}
	
	/**
	 * Parses a date string using the format "dd/MM/yyyy HH:mm".
	 * 
	 * @param date the date string to parse
	 * @return the parsed Date, or null if parsing fails
	 */
	public static Date parseDate(String date) {
		try {
			return new SimpleDateFormat("dd/MM/yyyy HH:mm").parse(date);
		} catch (ParseException e) {
			return null;
		}
	}

	/**
	 * Parses a date string using the specified format.
	 * 
	 * @param date   the date string to parse
	 * @param format the date format pattern
	 * @return the parsed Date, or null if parsing fails
	 */
	public static Date parseDate(String date, String format) {
		try {
			return new SimpleDateFormat(format).parse(date);
		} catch (ParseException e) {
			return null;
		}
	}

	/**
	 * Formats a date using the format "dd/MM/yyyy HH:mm".
	 * 
	 * @param date the date to format
	 * @return the formatted date string
	 */
	public static String formatDate(Date date) {
		Format formatter = new SimpleDateFormat("dd/MM/yyyy HH:mm");
		return formatter.format(date);
	}

	/**
	 * Formats a date using the specified format.
	 * 
	 * @param date   the date to format
	 * @param format the date format pattern
	 * @return the formatted date string
	 */
	public static String formatDate(Date date, String format) {
		Format formatter = new SimpleDateFormat(format);
		return formatter.format(date);
	}

	/**
	 * Gets the current local date-time as a timestamp string (yyyyMMddHHmmss).
	 * 
	 * @return the formatted timestamp
	 */
	public static String getCurrentLocalDateTimeStamp() {
		return LocalDateTime.now()
				.format(DateTimeFormatter.ofPattern("yyyyMMddHHmmss"));
	}

	/**
	 * Gets the week number within the year (YYYYWW format).
	 * 
	 * @param date the date
	 * @return the week number as YYYYWW integer
	 */
	public static Integer getWeekInYear(Date date) {
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		return Integer.parseInt(String.format("%d%02d", 
				cal.get(Calendar.YEAR), cal.get(Calendar.WEEK_OF_YEAR)));
	}

	/**
	 * Gets the month within the year (YYYYMM format).
	 * Note: Bug in original code - uses MONDAY instead of MONTH.
	 * 
	 * @param date the date
	 * @return the month as YYYYMM integer
	 */
	public static Integer getMonthInYear(Date date) {
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		return Integer.parseInt(String.format("%d%02d", 
				cal.get(Calendar.YEAR), cal.get(Calendar.MONTH) + 1));
	}

	/**
	 * Gets the hour of day (0-23).
	 * 
	 * @param date the date
	 * @return the hour
	 */
	public static Integer getHour(Date date) {
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		return cal.get(Calendar.HOUR_OF_DAY);
	}

	/**
	 * Adds the specified number of days to a date.
	 * 
	 * @param date the starting date
	 * @param days the number of days to add
	 * @return a new Date with the days added
	 */
	public static Date addDay(Date date, int days) {
		LocalDateTime ldt = LocalDateTime.ofInstant(date.toInstant(), ZoneId.systemDefault());
		LocalDateTime ldtPlusOne = ldt.plusDays(days);
		return Date.from(ldtPlusOne.atZone(ZoneId.systemDefault()).toInstant());
	}

	/**
	 * Adds the specified number of hours to a date.
	 * 
	 * @param date  the starting date
	 * @param hours the number of hours to add
	 * @return a new Date with the hours added
	 */
	public static Date addHours(Date date, int hours) {
		LocalDateTime ldt = LocalDateTime.ofInstant(date.toInstant(), ZoneId.systemDefault());
		LocalDateTime ldtPlusHours = ldt.plusHours(hours);
		return Date.from(ldtPlusHours.atZone(ZoneId.systemDefault()).toInstant());
	}

	/**
	 * Checks if a date falls on a weekend (Saturday or Sunday).
	 * 
	 * @param date the date to check
	 * @return true if the date is on a weekend
	 */
	public static boolean isWeekend(Date date) {
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		int dayOfWeek = cal.get(Calendar.DAY_OF_WEEK);
		return dayOfWeek == Calendar.SATURDAY || dayOfWeek == Calendar.SUNDAY;
	}
}
