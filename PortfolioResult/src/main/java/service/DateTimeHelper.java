package service;

import java.text.Format;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.Calendar;
import java.util.Date;

import org.joda.time.DateTime;

public class DateTimeHelper {
	private DateTimeHelper(){
	
	}
	public static Date atEndOfDay(Date date) {
	    LocalDateTime localDateTime = dateToLocalDateTime(date);
	    LocalDateTime endOfDay = localDateTime.with(LocalTime.MAX);
	    return localDateTimeToDate(endOfDay);
	}

	private static LocalDateTime dateToLocalDateTime(Date date) {
	    return LocalDateTime.ofInstant(date.toInstant(), ZoneId.systemDefault());
	}

	private static Date localDateTimeToDate(LocalDateTime localDateTime) {
	    return Date.from(localDateTime.atZone(ZoneId.systemDefault()).toInstant());
	}
	
	public static Date parseDate(String date) {
	     try {
	         return new SimpleDateFormat("dd/MM/yyyy HH:mm").parse(date);
	     } catch (ParseException e) {
	         return null;
	     }
	  }
	
	public static Date parseDate(String date,String format) {
	     try {
	         return new SimpleDateFormat(format).parse(date);
	     } catch (ParseException e) {
	         return null;
	     }
	  }
	
	public static String formatDate(Date date){
		Format formatter = new SimpleDateFormat("dd/MM/yyyy HH:mm");
		return formatter.format(date);
	}
	
	public static String formatDate(Date date,String format){
		Format formatter = new SimpleDateFormat(format);
		return formatter.format(date);
	}
	
	public static String getCurrentLocalDateTimeStamp() {
	    return LocalDateTime.now()
	       .format(DateTimeFormatter.ofPattern("yyyyMMddHHmmss"));
	}
	
	public static Integer getWeekInYear(Date date){
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		
		return Integer.parseInt(String.format("%d%02d", cal.get(Calendar.YEAR),cal.get(Calendar.WEEK_OF_YEAR)));
	}
	
	public static Integer getMonthInYear(Date date){
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		
		return Integer.parseInt(String.format("%d%02d", cal.get(Calendar.YEAR),cal.get(Calendar.MONDAY)+1));
	}
	
	public static Integer getHour(Date date){
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		
		return cal.get(Calendar.HOUR_OF_DAY );
	}
	
	public static Date addDay(Date date,int days){		
		DateTime dtOrg = new DateTime(date);
		DateTime dtPlusOne = dtOrg.plusDays(1);
		return dtPlusOne.toDate();
	}
	
	public static Date addHours(Date date,int hours){		
		DateTime dtOrg = new DateTime(date);
		DateTime dtPlusOne = dtOrg.plusHours(hours);
		return dtPlusOne.toDate();
	}
	
	public static boolean isWeekend(Date date){		
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		
		if( cal.get(Calendar.DAY_OF_WEEK ) == Calendar.SATURDAY ||  cal.get(Calendar.DAY_OF_WEEK ) == Calendar.SUNDAY)
			return true;
		return false;
		
	}
}
