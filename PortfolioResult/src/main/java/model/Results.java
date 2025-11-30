package model;

import java.util.Date;
import java.util.Objects;


public class Results implements Comparable<Results> {
	
	public String strategyID;
	public String orderNumber;
	public String orderType;	
	public Date openTime;
	public Date closeTime;
	public double openPrice;
	public double closePrice;
	public double lots;
	public double profit;	
	public double stopLossPrice;
	public double takeProfitPrice;
	public double balance;
	public String id;
	public String pair;
	public double swap;

	public Integer weekInYear;
	public Integer monthInYear;
	public double  pl;
	public Results(){
		
	}
	
	public Results(Results that){
		this.strategyID = that.strategyID;
		this.orderNumber = that.orderNumber;
		this.orderType = that.orderType;
		this.openTime = that.openTime;
		this.closeTime = that.closeTime;
		this.openPrice = that.openPrice;
		this.closePrice = that.closePrice;
		this.lots =that.lots;
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

	@Override
	public boolean equals(Object other) {
		if (!(other instanceof Results)) {
			return false;
		}

		Results that = (Results) other;

		// Custom equality check here.
		return this.strategyID.equals(that.strategyID) && this.orderNumber.equals(that.orderNumber)
				&& this.pair.equals(that.pair) && this.closeTime.toString().equals(that.closeTime.toString())
				&& this.orderType.equals(that.orderType);
	}

	@Override
	public int hashCode() {
		return Objects.hash(this.strategyID,this.orderNumber,
				this.pair, this.closeTime.toString(),this.orderType);
	}

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

	@Override
	public String toString() {
		return String.format("StrategyID=%s Symbol=%s OrderType=%s OpenTime=%s CloseTime=%s OpenPrice=%f ClosePrice=%f Lots=%f SL=%f TP=%f",
				strategyID,pair,orderType,openTime.toString(),closeTime.toString(),openPrice,closePrice,lots,stopLossPrice,takeProfitPrice);
	}
}
