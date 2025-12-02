package model;

import java.util.Date;
import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.AllArgsConstructor;

/**
 * Represents a daily market check with open and close times.
 * Used to track daily trading sessions and identify market open/close periods.
 * 
 * @Data generates: getters, setters, toString, equals, hashCode
 * Explicitly include @NoArgsConstructor and @AllArgsConstructor for compatibility
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class DailyCheck {

	/** The opening time for the trading day */
	public Date opentime;
	
	/** The closing time for the trading day */
	public Date closetime;

	/**
	 * Copy constructor.
	 * 
	 * @param that the DailyCheck to copy
	 */
	public DailyCheck(DailyCheck that) {
		this.opentime = that.opentime;
		this.closetime = that.closetime;
	}
}
