package model;

import java.util.Date;
import lombok.EqualsAndHashCode;
import lombok.NoArgsConstructor;
import lombok.AllArgsConstructor;

/**
 * Represents market rate data (OHLCV - Open, High, Low, Close, Volume).
 * Comparable by datetime to allow chronological sorting.
 */
@EqualsAndHashCode
@NoArgsConstructor
@AllArgsConstructor
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
}
