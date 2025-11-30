package model;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import service.DateTimeHelper;

/**
 * Timezone adjuster for commodity markets (CME, NYMEX, etc.) with US holidays and early closes.
 * Handles DST transitions for commodity trading hours and filters US market holidays.
 */
class CommodityTimeZoneAdjuster extends TimeZoneAdjuster {
	
	private static final int EARLY_CLOSE_HOUR = 20;
	private static final int MINIMUM_MARKET_HOUR = 1;

	@Override
	protected List<String> getHolidays() {
		List<String> holidays = new ArrayList<>();
		// US Market Holidays 2013-2019
		holidays.add("01/01/13"); holidays.add("25/12/13");
		holidays.add("01/01/14"); holidays.add("25/12/14");
		holidays.add("01/01/15"); holidays.add("25/12/15");
		holidays.add("01/01/16"); holidays.add("25/12/16");
		holidays.add("02/01/17"); holidays.add("14/04/17"); holidays.add("25/12/17");
		holidays.add("01/01/18"); holidays.add("30/03/18"); holidays.add("25/12/18");
		holidays.add("01/01/19"); holidays.add("19/04/19"); holidays.add("25/12/19");
		return holidays;
	}

	@Override
	protected List<String> getEarlyCloseDates() {
		List<String> earlyClose = new ArrayList<>();
		// US Market Early Close Days 2013-2019
		addEarlyCloseDates2013(earlyClose);
		addEarlyCloseDates2014(earlyClose);
		addEarlyCloseDates2015(earlyClose);
		addEarlyCloseDates2016(earlyClose);
		addEarlyCloseDates2017(earlyClose);
		addEarlyCloseDates2018(earlyClose);
		addEarlyCloseDates2019(earlyClose);
		return earlyClose;
	}

	private void addEarlyCloseDates2013(List<String> list) {
		list.add("21/01/13"); // MLK Day
		list.add("18/02/13"); // Presidents Day
		list.add("27/05/13"); // Memorial Day
		list.add("04/07/13"); // Independence Day
		list.add("02/09/13"); // Labor Day
		list.add("28/11/13"); list.add("29/11/13"); // Thanksgiving
	}

	private void addEarlyCloseDates2014(List<String> list) {
		list.add("20/01/14"); list.add("17/02/14"); list.add("26/05/14");
		list.add("04/07/14"); list.add("01/09/14"); 
		list.add("27/11/14"); list.add("28/11/14");
	}

	private void addEarlyCloseDates2015(List<String> list) {
		list.add("19/01/15"); list.add("16/02/15"); list.add("25/05/15");
		list.add("04/07/15"); list.add("07/09/15");
		list.add("26/11/15"); list.add("27/11/15");
	}

	private void addEarlyCloseDates2016(List<String> list) {
		list.add("18/01/16"); list.add("15/02/16"); list.add("30/05/16");
		list.add("04/07/16"); list.add("05/09/16");
		list.add("24/11/16"); list.add("25/11/16");
	}

	private void addEarlyCloseDates2017(List<String> list) {
		list.add("16/01/17"); list.add("20/02/17"); list.add("29/05/17");
		list.add("04/07/17"); list.add("04/09/17");
		list.add("23/11/17"); list.add("24/11/17");
	}

	private void addEarlyCloseDates2018(List<String> list) {
		list.add("15/01/18"); list.add("19/02/18"); list.add("28/05/18");
		list.add("04/07/18"); list.add("03/09/18");
		list.add("22/11/18"); list.add("23/11/18"); list.add("05/12/18");
	}

	private void addEarlyCloseDates2019(List<String> list) {
		list.add("21/01/19"); list.add("18/02/19"); list.add("27/05/19");
		list.add("04/07/19"); list.add("02/09/19");
		list.add("28/11/19"); list.add("29/11/19");
	}

	@Override
	protected int getDSTAdjustmentHours(Date datetime) {
		// Complex DST transition logic for commodity markets (CME hours)
		if (isBefore(datetime, "11.03.2013 22:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "03.11.2013 23:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "21.04.2014 22:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "02.11.2014 23:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "10.03.2015 22:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "01.11.2015 23:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "13.03.2016 22:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "06.11.2016 23:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "14.03.2017 22:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "05.11.2017 23:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "11.03.2018 22:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "04.11.2018 23:00", "dd.MM.yyyy HH:mm")) return 3;
		if (isBefore(datetime, "10.03.2019 22:00", "dd.MM.yyyy HH:mm")) return 2;
		if (isBefore(datetime, "03.11.2019 23:00", "dd.MM.yyyy HH:mm")) return 3;
		return 2; // Default
	}

	@Override
	protected boolean shouldFilterRate(Rates rate, List<String> holidays, List<String> earlyClose) {
		if (super.shouldFilterRate(rate, holidays, earlyClose)) {
			return true;
		}

		// Filter very early morning hours
		if (DateTimeHelper.getHour(rate.datetime) < MINIMUM_MARKET_HOUR) {
			return true;
		}

		return false;
	}

	@Override
	protected boolean isEarlyCloseFiltered(Rates rate, List<String> earlyClose) {
		String dateStr = DateTimeHelper.formatDate(rate.datetime, "dd/MM/yy");
		return earlyClose.contains(dateStr) 
				&& DateTimeHelper.getHour(rate.datetime) >= EARLY_CLOSE_HOUR;
	}
}
