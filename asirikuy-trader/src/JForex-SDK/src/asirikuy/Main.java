
package asirikuy;

import java.text.SimpleDateFormat;
import java.util.List;

import com.dukascopy.api.IBar;
import com.dukascopy.api.IOrder;

public class Main {
	
    public static void main(String[] args) throws Exception {
        DukascopyWrapper dcw = new DukascopyWrapper("DEMO2JEDVs", "JEDVs", "https://www.dukascopy.com/client/demo/jclient/jforex.jnlp", false);
        
        //Get candles
        List<IBar> bars = dcw.dumbStrategy.getCandles("H1", "EUR/USD", 10);
        for (int i=0; i<bars.size(); i++){
	        System.out.println(String.format(
	        	    "Previous bar (time: %s) close price=%.5f", 
	        	    new SimpleDateFormat("MM/dd/yyyy hh:mm").format(bars.get(i).getTime()), 
	        	    bars.get(i).getClose()
	        	    )
	        	  );
        }
        
        //Get orders
        for (IOrder order : dcw.dumbStrategy.engine.getOrders()) {
        	String direction;
            if (order.isLong()) direction = "buy";
            else direction = "sell";
            System.out.println(String.format(
	        	    "Order number: %s %s %s", 
	        	    order.getId(),
	        	    order.getInstrument().toString(),
	        	    direction
	        	    )
	        	  );
        }  
        
        
        //Get account info
        if (dcw.dumbStrategy.account != null){
        	System.out.println(String.format("equity = % f balance = % f", 
        			dcw.dumbStrategy.account.getEquity(), dcw.dumbStrategy.account.getCreditLine()) );
        }
        
        //Get bidask
        Thread.sleep(50);
        List<Double> bidAsk = dcw.dumbStrategy.getBidAsk("EUR/USD");    
        if (bidAsk!=null) System.out.println(String.format("EURUSD Bid %f Ask %f", bidAsk.get(0), bidAsk.get(1)));

       
        //Open trade
        //dcw.dumbStrategy.openOrder("", 0.5, "buy", "EUR/USD", 1.30, 1.40, -1, "13");
        
        //Close trade
        //dcw.dumbStrategy.closeOrder("52123990");  
    }
}
