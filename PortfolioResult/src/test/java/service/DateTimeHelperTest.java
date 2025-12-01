package service;

import static org.junit.jupiter.api.Assertions.*;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.Calendar;
import java.util.Date;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

/**
 * Unit tests for {@link DateTimeHelper} utility class.
 * Tests date parsing, formatting, manipulation, and time calculations.
 */
@DisplayName("DateTimeHelper Tests")
class DateTimeHelperTest {

	@Test
	@DisplayName("Should prevent instantiation of utility class")
	void testUtilityClassCannotBeInstantiated() {
		try {
			// Use reflection to bypass private constructor
			java.lang.reflect.Constructor<DateTimeHelper> constructor = 
				DateTimeHelper.class.getDeclaredConstructor();
			constructor.setAccessible(true);
			Exception exception = assertThrows(Exception.class, () -> {
				constructor.newInstance();
			});
			// The exception should be wrapped in InvocationTargetException
			assertTrue(exception.getCause() instanceof UnsupportedOperationException ||
					exception instanceof UnsupportedOperationException);
		} catch (NoSuchMethodException e) {
			fail("Constructor not found: " + e.getMessage());
		}
	}

	@Test
	@DisplayName("Should parse date string with default format")
	void testParseDateDefaultFormat() {
		Date date = DateTimeHelper.parseDate("01/12/2025 14:30");
		assertNotNull(date);
		
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		assertEquals(2025, cal.get(Calendar.YEAR));
		assertEquals(Calendar.DECEMBER, cal.get(Calendar.MONTH));
		assertEquals(1, cal.get(Calendar.DAY_OF_MONTH));
		assertEquals(14, cal.get(Calendar.HOUR_OF_DAY));
		assertEquals(30, cal.get(Calendar.MINUTE));
	}

	@Test
	@DisplayName("Should parse date string with custom format")
	void testParseDateCustomFormat() {
		Date date = DateTimeHelper.parseDate("2025-12-01 14:30:00", "yyyy-MM-dd HH:mm:ss");
		assertNotNull(date);
		
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		assertEquals(2025, cal.get(Calendar.YEAR));
		assertEquals(Calendar.DECEMBER, cal.get(Calendar.MONTH));
		assertEquals(1, cal.get(Calendar.DAY_OF_MONTH));
	}

	@Test
	@DisplayName("Should return null for invalid date string")
	void testParseDateInvalidString() {
		Date date = DateTimeHelper.parseDate("invalid date");
		assertNull(date);
	}

	@Test
	@DisplayName("Should format date with default format")
	void testFormatDateDefaultFormat() {
		Date date = new Date();
		String formatted = DateTimeHelper.formatDate(date);
		assertNotNull(formatted);
		assertTrue(formatted.matches("\\d{2}/\\d{2}/\\d{4} \\d{2}:\\d{2}"));
	}

	@Test
	@DisplayName("Should format date with custom format")
	void testFormatDateCustomFormat() {
		Date date = new Date();
		String formatted = DateTimeHelper.formatDate(date, "yyyy-MM-dd");
		assertNotNull(formatted);
		assertTrue(formatted.matches("\\d{4}-\\d{2}-\\d{2}"));
	}

	@Test
	@DisplayName("Should get current local date-time stamp")
	void testGetCurrentLocalDateTimeStamp() {
		String timestamp = DateTimeHelper.getCurrentLocalDateTimeStamp();
		assertNotNull(timestamp);
		assertEquals(14, timestamp.length());
		assertTrue(timestamp.matches("\\d{14}"));
	}

	@Test
	@DisplayName("Should get week in year")
	void testGetWeekInYear() {
		// Test with a known date: January 1, 2025 (week 1)
		Calendar cal = Calendar.getInstance();
		cal.set(2025, Calendar.JANUARY, 1);
		Date date = cal.getTime();
		
		Integer week = DateTimeHelper.getWeekInYear(date);
		assertNotNull(week);
		assertTrue(week >= 202501 && week <= 202553);
	}

	@Test
	@DisplayName("Should get month in year")
	void testGetMonthInYear() {
		Calendar cal = Calendar.getInstance();
		cal.set(2025, Calendar.DECEMBER, 15);
		Date date = cal.getTime();
		
		Integer month = DateTimeHelper.getMonthInYear(date);
		assertNotNull(month);
		assertEquals(202512, month.intValue());
	}

	@Test
	@DisplayName("Should get day in year")
	void testGetDayInYear() {
		Calendar cal = Calendar.getInstance();
		cal.set(2025, Calendar.DECEMBER, 1);
		Date date = cal.getTime();
		
		Integer day = DateTimeHelper.getDayInYear(date);
		assertNotNull(day);
		assertEquals(20251201, day.intValue());
	}

	@Test
	@DisplayName("Should get hour of day")
	void testGetHour() {
		Calendar cal = Calendar.getInstance();
		cal.set(2025, Calendar.DECEMBER, 1, 14, 30);
		Date date = cal.getTime();
		
		Integer hour = DateTimeHelper.getHour(date);
		assertNotNull(hour);
		assertEquals(14, hour.intValue());
	}

	@Test
	@DisplayName("Should add days to date")
	void testAddDay() {
		Calendar cal = Calendar.getInstance();
		cal.set(2025, Calendar.DECEMBER, 1);
		Date originalDate = cal.getTime();
		
		Date newDate = DateTimeHelper.addDay(originalDate, 5);
		assertNotNull(newDate);
		
		Calendar newCal = Calendar.getInstance();
		newCal.setTime(newDate);
		assertEquals(6, newCal.get(Calendar.DAY_OF_MONTH));
	}

	@Test
	@DisplayName("Should subtract days from date")
	void testSubtractDay() {
		Calendar cal = Calendar.getInstance();
		cal.set(2025, Calendar.DECEMBER, 10);
		Date originalDate = cal.getTime();
		
		Date newDate = DateTimeHelper.addDay(originalDate, -5);
		assertNotNull(newDate);
		
		Calendar newCal = Calendar.getInstance();
		newCal.setTime(newDate);
		assertEquals(5, newCal.get(Calendar.DAY_OF_MONTH));
	}

	@Test
	@DisplayName("Should add hours to date")
	void testAddHours() {
		Calendar cal = Calendar.getInstance();
		cal.set(2025, Calendar.DECEMBER, 1, 14, 30);
		Date originalDate = cal.getTime();
		
		Date newDate = DateTimeHelper.addHours(originalDate, 3);
		assertNotNull(newDate);
		
		Calendar newCal = Calendar.getInstance();
		newCal.setTime(newDate);
		assertEquals(17, newCal.get(Calendar.HOUR_OF_DAY));
	}

	@Test
	@DisplayName("Should identify weekend dates")
	void testIsWeekend() {
		// Saturday
		Calendar satCal = Calendar.getInstance();
		satCal.set(2025, Calendar.DECEMBER, 6); // Dec 6, 2025 is a Saturday
		Date saturday = satCal.getTime();
		assertTrue(DateTimeHelper.isWeekend(saturday));
		
		// Sunday
		Calendar sunCal = Calendar.getInstance();
		sunCal.set(2025, Calendar.DECEMBER, 7); // Dec 7, 2025 is a Sunday
		Date sunday = sunCal.getTime();
		assertTrue(DateTimeHelper.isWeekend(sunday));
		
		// Monday
		Calendar monCal = Calendar.getInstance();
		monCal.set(2025, Calendar.DECEMBER, 1); // Dec 1, 2025 is a Monday
		Date monday = monCal.getTime();
		assertFalse(DateTimeHelper.isWeekend(monday));
	}

	@Test
	@DisplayName("Should set time to end of day")
	void testAtEndOfDay() {
		Calendar cal = Calendar.getInstance();
		cal.set(2025, Calendar.DECEMBER, 1, 14, 30, 0);
		cal.set(Calendar.MILLISECOND, 0);
		Date originalDate = cal.getTime();
		
		Date endOfDay = DateTimeHelper.atEndOfDay(originalDate);
		assertNotNull(endOfDay);
		
		Calendar endCal = Calendar.getInstance();
		endCal.setTime(endOfDay);
		assertEquals(23, endCal.get(Calendar.HOUR_OF_DAY));
		assertEquals(59, endCal.get(Calendar.MINUTE));
		assertEquals(59, endCal.get(Calendar.SECOND));
		assertEquals(999, endCal.get(Calendar.MILLISECOND));
	}

	@Test
	@DisplayName("Should handle date parsing round-trip")
	void testDateParsingRoundTrip() {
		String dateString = "01/12/2025 14:30";
		Date parsed = DateTimeHelper.parseDate(dateString);
		assertNotNull(parsed);
		
		String formatted = DateTimeHelper.formatDate(parsed);
		assertNotNull(formatted);
		assertEquals(dateString, formatted);
	}
}

