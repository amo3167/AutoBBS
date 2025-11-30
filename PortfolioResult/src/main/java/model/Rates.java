package model;

import java.util.Date;
import java.util.Objects;

public class Rates implements Comparable<Rates>{
	public Date datetime;
	public double open;
	public double close;
	public double high;
	public double low;
	public double volume;
	
	public Rates(){
		
	}
	
	public Rates(Rates that){
		this.datetime = that.datetime;
		this.open = that.open;
		this.close = that.close;
		this.high = that.high;
		this.low = that.low;
		this.volume = that.volume;		
	}

	@Override
	public int compareTo(Rates obj) {
		return this.datetime.compareTo(obj.datetime);
	}
	
	@Override
	public boolean equals(Object other) {
		if (!(other instanceof Rates)) {
			return false;
		}

		Rates that = (Rates) other;

		// Custom equality check here.
		return this.open == that.open
				&& this.close == that.close
				&& this.high == that.high
				&& this.low == that.low 				
				&& this.datetime.toString().equals(that.datetime.toString())
				&& this.volume == that.volume;
	}

	@Override
	public int hashCode() {
		return Objects.hash(Double.toString(this.open),Double.toString(this.close),
				Double.toString(this.high), Double.toString(this.low), this.volume,this.datetime.toString());
	}
}
