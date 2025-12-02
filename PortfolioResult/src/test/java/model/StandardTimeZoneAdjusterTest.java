package model;

import static org.junit.jupiter.api.Assertions.*;

import java.util.Date;
import java.util.List;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import service.DateTimeHelper;

/**
 * Unit tests for {@link StandardTimeZoneAdjuster}.
 * Tests DST transitions, holiday filtering, and timezone adjustments for forex markets.
 */
@DisplayName("StandardTimeZoneAdjuster Tests")
class StandardTimeZoneAdjusterTest {

	private StandardTimeZoneAdjuster adjuster;

	@BeforeEach
	void setUp() {
		adjuster = new StandardTimeZoneAdjuster();
	}

	@Test
	@DisplayName("Should adjust DST for dates before 2013 spring DST")
	void testDSTAdjustment_Before2013Spring() {
		Date date = DateTimeHelper.parseDate("01.01.2013 10:00", "dd.MM.yyyy HH:mm");
		int hours = adjuster.getDSTAdjustmentHours(date);
		assertEquals(2, hours, "Should return 2 hours for dates before 2013 spring DST");
	}

	@Test
	@DisplayName("Should adjust DST for dates between 2013 spring and fall DST")
	void testDSTAdjustment_2013SpringToFall() {
		Date date = DateTimeHelper.parseDate("15.06.2013 10:00", "dd.MM.yyyy HH:mm");
		int hours = adjuster.getDSTAdjustmentHours(date);
		assertEquals(3, hours, "Should return 3 hours during DST period");
	}

	@Test
	@DisplayName("Should adjust DST for dates after 2013 fall DST")
	void testDSTAdjustment_After2013Fall() {
		Date date = DateTimeHelper.parseDate("15.12.2013 10:00", "dd.MM.yyyy HH:mm");
		int hours = adjuster.getDSTAdjustmentHours(date);
		assertEquals(2, hours, "Should return 2 hours after fall DST");
	}

	@Test
	@DisplayName("Should return valid DST adjustment hours (2 or 3)")
	void testDSTAdjustmentReturnsValidHours() {
		Date[] testDates = {
			DateTimeHelper.parseDate("01.01.2013 10:00", "dd.MM.yyyy HH:mm"),
			DateTimeHelper.parseDate("15.06.2013 10:00", "dd.MM.yyyy HH:mm"),
			DateTimeHelper.parseDate("15.06.2014 10:00", "dd.MM.yyyy HH:mm"),
			DateTimeHelper.parseDate("15.06.2015 10:00", "dd.MM.yyyy HH:mm"),
			DateTimeHelper.parseDate("15.06.2016 10:00", "dd.MM.yyyy HH:mm"),
			DateTimeHelper.parseDate("15.06.2017 10:00", "dd.MM.yyyy HH:mm"),
			DateTimeHelper.parseDate("15.06.2018 10:00", "dd.MM.yyyy HH:mm"),
			DateTimeHelper.parseDate("15.12.2018 10:00", "dd.MM.yyyy HH:mm")
		};
		
		for (Date date : testDates) {
			int hours = adjuster.getDSTAdjustmentHours(date);
			assertTrue(hours == 2 || hours == 3, 
				"DST adjustment should return 2 or 3 hours, got " + hours + " for date " + date);
		}
	}

	@Test
	@DisplayName("Should return default 2 hours for dates after 2018")
	void testDSTAdjustment_After2018() {
		Date date = DateTimeHelper.parseDate("01.01.2020 10:00", "dd.MM.yyyy HH:mm");
		int hours = adjuster.getDSTAdjustmentHours(date);
		assertEquals(2, hours, "Should return default 2 hours for dates after 2018");
	}

	@Test
	@DisplayName("Should filter weekend rates")
	void testFilterWeekendRates() {
		Date saturday = DateTimeHelper.parseDate("06.01.2018 10:00", "dd.MM.yyyy HH:mm"); // Saturday
		Date sunday = DateTimeHelper.parseDate("07.01.2018 10:00", "dd.MM.yyyy HH:mm"); // Sunday
		
		Rates saturdayRate = createTestRate(saturday);
		Rates sundayRate = createTestRate(sunday);
		
		List<Rates> rates = List.of(saturdayRate, sundayRate);
		List<Rates> result = adjuster.adjustTimeZone(rates);
		
		assertTrue(result.isEmpty(), "Should filter out weekend rates");
	}

	@Test
	@DisplayName("Should filter holiday rates")
	void testFilterHolidayRates() {
		Date newYear = DateTimeHelper.parseDate("01.01.2018 10:00", "dd.MM.yyyy HH:mm"); // New Year's Day
		Date christmas = DateTimeHelper.parseDate("25.12.2018 10:00", "dd.MM.yyyy HH:mm"); // Christmas
		
		Rates newYearRate = createTestRate(newYear);
		Rates christmasRate = createTestRate(christmas);
		
		List<Rates> rates = List.of(newYearRate, christmasRate);
		List<Rates> result = adjuster.adjustTimeZone(rates);
		
		assertTrue(result.isEmpty(), "Should filter out holiday rates");
	}

	@Test
	@DisplayName("Should not filter non-holiday weekday rates")
	void testKeepNonHolidayWeekdayRates() {
		Date weekday = DateTimeHelper.parseDate("15.01.2018 10:00", "dd.MM.yyyy HH:mm"); // Monday
		
		Rates rate = createTestRate(weekday);
		
		List<Rates> rates = List.of(rate);
		List<Rates> result = adjuster.adjustTimeZone(rates);
		
		assertEquals(1, result.size(), "Should keep non-holiday weekday rates");
		assertNotNull(result.get(0).datetime);
	}

	@Test
	@DisplayName("Should adjust timezone for valid rates")
	void testAdjustTimezone() {
		Date originalDate = DateTimeHelper.parseDate("15.01.2018 10:00", "dd.MM.yyyy HH:mm");
		Rates rate = createTestRate(originalDate);
		
		List<Rates> rates = List.of(rate);
		List<Rates> result = adjuster.adjustTimeZone(rates);
		
		assertEquals(1, result.size());
		assertNotEquals(originalDate, result.get(0).datetime, "Datetime should be adjusted");
	}

	@Test
	@DisplayName("Should return empty list for empty input")
	void testEmptyInput() {
		List<Rates> result = adjuster.adjustTimeZone(List.of());
		assertTrue(result.isEmpty());
	}

	@Test
	@DisplayName("Should have no early close dates for forex")
	void testNoEarlyCloseDates() {
		List<String> earlyClose = adjuster.getEarlyCloseDates();
		assertTrue(earlyClose.isEmpty(), "Forex markets have no early close dates");
	}

	// Helper methods

	private Rates createTestRate(Date datetime) {
		Rates rate = new Rates();
		rate.datetime = datetime;
		rate.open = 1.2000;
		rate.close = 1.2005;
		rate.high = 1.2010;
		rate.low = 1.1990;
		rate.volume = 1000;
		return rate;
	}
}

