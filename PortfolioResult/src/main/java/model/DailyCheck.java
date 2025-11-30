package model;

import java.util.Date;
import java.util.Objects;

public class DailyCheck {
	
	public Date opentime;
	public Date closetime;
	
	public DailyCheck(){
		
	}
	
	public DailyCheck(Date openTime,Date closeTime){		
		this.opentime = openTime;
		this.closetime = closeTime;
	}
	
	public DailyCheck(DailyCheck that){	
		this.opentime = that.opentime;
		this.closetime = that.closetime;
			
	}

	
	@Override
	public boolean equals(Object other) {
		if (!(other instanceof DailyCheck)) {
			return false;
		}

		DailyCheck that = (DailyCheck) other;

		// Custom equality check here.
		return this.opentime == that.opentime && this.closetime == that.closetime;								
				
	}

	@Override
	public int hashCode() {
		return Objects.hash(this.opentime,this.closetime);
	}
}
