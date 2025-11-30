package model;

import java.util.Date;
import java.util.Objects;

/**
 * Represents a daily market check with open and close times.
 * Used to track daily trading sessions and identify market open/close periods.
 */
public class DailyCheck {

	/** The opening time for the trading day */
	public Date opentime;
	
	/** The closing time for the trading day */
	public Date closetime;

	/**
	 * Default constructor.
	 */
	public DailyCheck() {
	}

	/**
	 * Constructs a DailyCheck with specified open and close times.
	 * 
	 * @param openTime  the market open time
	 * @param closeTime the market close time
	 */
	public DailyCheck(Date openTime, Date closeTime) {
		this.opentime = openTime;
		this.closetime = closeTime;
	}

	/**
	 * Copy constructor.
	 * 
	 * @param that the DailyCheck to copy
	 */
	public DailyCheck(DailyCheck that) {
		this.opentime = that.opentime;
		this.closetime = that.closetime;
	}

	
	/**
	 * Checks equality based on open and close times.
	 * 
	 * @param other the object to compare with
	 * @return true if the open and close times are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (!(other instanceof DailyCheck)) {
			return false;
		}

		DailyCheck that = (DailyCheck) other;
		return Objects.equals(this.opentime, that.opentime) 
			&& Objects.equals(this.closetime, that.closetime);
	}

	/**
	 * Generates hash code based on open and close times.
	 * 
	 * @return the hash code
	 */
	@Override
	public int hashCode() {
		return Objects.hash(this.opentime, this.closetime);
	}
}
