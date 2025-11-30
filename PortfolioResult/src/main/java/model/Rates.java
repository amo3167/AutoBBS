package model;

import java.util.Date;
import java.util.Objects;

/**
 * Represents market rate data (OHLCV - Open, High, Low, Close, Volume).
 * Comparable by datetime to allow chronological sorting.
 */
public class Rates implements Comparable<Rates> {

	/** The timestamp for this rate data */
	public Date datetime;
	
	/** The opening price */
	public double open;
	
	/** The closing price */
	public double close;
	
	/** The highest price during the period */
	public double high;
	
	/** The lowest price during the period */
	public double low;
	
	/** The trading volume */
	public double volume;

	/**
	 * Default constructor.
	 */
	public Rates() {
	}

	/**
	 * Copy constructor.
	 * 
	 * @param that the Rates object to copy
	 */
	public Rates(Rates that) {
		this.datetime = that.datetime;
		this.open = that.open;
		this.close = that.close;
		this.high = that.high;
		this.low = that.low;
		this.volume = that.volume;
	}

	/**
	 * Compares rates chronologically by datetime.
	 * 
	 * @param obj the Rates object to compare to
	 * @return negative, zero, or positive as this datetime is before, equal to, or after the specified datetime
	 */
	@Override
	public int compareTo(Rates obj) {
		return this.datetime.compareTo(obj.datetime);
	}

	/**
	 * Checks equality based on all OHLCV fields.
	 * 
	 * @param other the object to compare with
	 * @return true if all fields are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (!(other instanceof Rates)) {
			return false;
		}

		Rates that = (Rates) other;
		return Double.compare(this.open, that.open) == 0
				&& Double.compare(this.close, that.close) == 0
				&& Double.compare(this.high, that.high) == 0
				&& Double.compare(this.low, that.low) == 0
				&& Double.compare(this.volume, that.volume) == 0
				&& Objects.equals(this.datetime, that.datetime);
	}

	/**
	 * Generates hash code based on all OHLCV fields.
	 * 
	 * @return the hash code
	 */
	@Override
	public int hashCode() {
		return Objects.hash(this.open, this.close, this.high, 
				this.low, this.volume, this.datetime);
	}
}
