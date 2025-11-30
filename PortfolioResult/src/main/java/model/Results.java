package model;

import java.util.Date;
import java.util.Objects;

/**
 * Represents a trading result with order details, P&L, and position information.
 * Comparable by close time, then strategy ID, then order number.
 */
public class Results implements Comparable<Results> {

	/** The strategy identifier */
	public String strategyID;
	
	/** The order number/identifier */
	public String orderNumber;
	
	/** The order type (BUY or SELL) */
	public String orderType;
	
	/** The time when the order was opened */
	public Date openTime;
	
	/** The time when the order was closed */
	public Date closeTime;
	
	/** The price at which the order was opened */
	public double openPrice;
	
	/** The price at which the order was closed */
	public double closePrice;
	
	/** The position size in lots */
	public double lots;
	
	/** The profit/loss from this trade */
	public double profit;
	
	/** The stop loss price */
	public double stopLossPrice;
	
	/** The take profit price */
	public double takeProfitPrice;
	
	/** The account balance after this trade */
	public double balance;
	
	/** The unique identifier for this result */
	public String id;
	
	/** The currency pair or trading instrument */
	public String pair;
	
	/** The swap/rollover charges */
	public double swap;

	/** The week number in the year */
	public Integer weekInYear;
	
	/** The month number in the year */
	public Integer monthInYear;
	
	/** The profit/loss percentage */
	public double pl;

	/**
	 * Default constructor.
	 */
	public Results() {
	}

	/**
	 * Copy constructor.
	 * 
	 * @param that the Results object to copy
	 */
	public Results(Results that) {
		this.strategyID = that.strategyID;
		this.orderNumber = that.orderNumber;
		this.orderType = that.orderType;
		this.openTime = that.openTime;
		this.closeTime = that.closeTime;
		this.openPrice = that.openPrice;
		this.closePrice = that.closePrice;
		this.lots = that.lots;
		this.profit = that.profit;
		this.stopLossPrice = that.stopLossPrice;
		this.takeProfitPrice = that.takeProfitPrice;
		this.balance = that.balance;
		this.id = that.id;
		this.pair = that.pair;
		this.swap = that.swap;
		this.weekInYear = that.weekInYear;
		this.monthInYear = that.monthInYear;
		this.pl = that.pl;
	}

	/**
	 * Checks equality based on strategy ID, order number, pair, close time, and order type.
	 * 
	 * @param other the object to compare with
	 * @return true if all key fields are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (!(other instanceof Results)) {
			return false;
		}

		Results that = (Results) other;
		return Objects.equals(this.strategyID, that.strategyID)
				&& Objects.equals(this.orderNumber, that.orderNumber)
				&& Objects.equals(this.pair, that.pair)
				&& Objects.equals(this.closeTime, that.closeTime)
				&& Objects.equals(this.orderType, that.orderType);
	}

	/**
	 * Generates hash code based on key identifying fields.
	 * 
	 * @return the hash code
	 */
	@Override
	public int hashCode() {
		return Objects.hash(this.strategyID, this.orderNumber,
				this.pair, this.closeTime, this.orderType);
	}

	/**
	 * Compares results by close time, then strategy ID, then order number.
	 * 
	 * @param obj the Results object to compare to
	 * @return negative, zero, or positive for ordering
	 */
	@Override
	public int compareTo(Results obj) {
		int result = this.closeTime.compareTo(obj.closeTime);

		if (result == 0) {
			result = this.strategyID.compareToIgnoreCase(obj.strategyID);
			if (result == 0) {
				return this.orderNumber.compareToIgnoreCase(obj.orderNumber);
			}
		}
		return result;
	}

	/**
	 * Returns a string representation of the trading result.
	 * 
	 * @return formatted string with key trade details
	 */
	@Override
	public String toString() {
		return String.format("StrategyID=%s Symbol=%s OrderType=%s OpenTime=%s CloseTime=%s " +
				"OpenPrice=%f ClosePrice=%f Lots=%f SL=%f TP=%f",
				strategyID, pair, orderType, openTime, closeTime, 
				openPrice, closePrice, lots, stopLossPrice, takeProfitPrice);
	}
}
