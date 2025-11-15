
import com.oanda.fxtrade.api.*;
import asirikuy.*;

public class Test 
{ 
    public static void main (String [ ] args) 
    { 
    	FXClient fxclient;
    	long sleepTime = 3000; 

    	fxclient = API.createFXGame();
    	try {
    		fxclient.setWithRateThread(true);
    		fxclient.login("danielfp248", "pepe2000");

    		User me = fxclient.getUser();
        	System.out.println ("name: " + me.getName()); 
        }
       	catch (SessionException e) { System.exit(1); }
		catch (InvalidUserException e) { System.exit(1); }
	   	catch (InvalidPasswordException e) { System.exit(1); } 

	   	OandaWrapper wrapper = new OandaWrapper() {
	   		public void onTick(String symbol, double bid, double ask){
	   			//FXRateEventInfo REI = (FXRateEventInfo) EI;
	   			//System.out.println(REI.getTick());
	   			System.out.println(symbol + " " + bid + " " + ask);
	   		};
	   	};
	   	Ticker ticker = new Ticker(wrapper, "EUR/USD");
	   	try { fxclient.getRateTable().getEventManager().add(ticker); }
		catch (SessionException e) { System.out.println(e.getMessage()); fxclient.logout(); System.exit(1); }

		System.out.println("starting. Printing rate data...");
		try { Thread.sleep(sleepTime); }
		catch (InterruptedException e) {System.out.println(e.getMessage());}
		System.out.println("finished. Printing rate data...");

		fxclient.logout();
     } 
}
