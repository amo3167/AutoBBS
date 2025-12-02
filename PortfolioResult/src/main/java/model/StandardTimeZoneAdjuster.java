package model;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * Timezone adjuster for standard forex markets.
 * Handles DST transitions for forex trading hours (nearly 24/5 operation).
 */
class StandardTimeZoneAdjuster extends TimeZoneAdjuster {

	@Override
	protected List<String> getHolidays() {
		List<String> holidays = new ArrayList<>();
		holidays.add("01/01/13"); holidays.add("25/12/13");
		holidays.add("01/01/14"); holidays.add("25/12/14");
		holidays.add("01/01/15"); holidays.add("25/12/15");
		holidays.add("01/01/16"); holidays.add("25/12/16");
		holidays.add("02/01/17"); holidays.add("14/04/17"); holidays.add("25/12/17");
		holidays.add("01/01/18"); holidays.add("30/03/18"); holidays.add("25/12/18");
		return holidays;
	}

	@Override
	protected List<String> getEarlyCloseDates() {
		return new ArrayList<>(); // No early close for forex
	}

	@Override
	protected int getDSTAdjustmentHours(Date datetime) {
		// DST transitions for forex markets
		if (isBefore(datetime, "11.03.2013 21:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "03.11.2013 22:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "21.04.2014 21:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "02.11.2014 22:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "10.03.2015 22:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "01.11.2015 22:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "13.03.2016 21:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "06.11.2016 22:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "14.03.2017 21:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "05.11.2017 22:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "11.03.2018 21:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "04.11.2018 22:00", "dd.MM.yyyy HH:mm")) return 3;
		return 2; // Default
	}
}
