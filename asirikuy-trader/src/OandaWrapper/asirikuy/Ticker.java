package asirikuy;

import com.oanda.fxtrade.api.*;

public class Ticker extends FXRateEvent
{
  private OandaWrapper itf;

  public Ticker(OandaWrapper pitf, String symbol){
    super(symbol);
    this.itf = pitf;
  }
  
  public void handle(FXEventInfo EI, FXEventManager EM)
  {
    FXRateEventInfo REI = (FXRateEventInfo) EI;
    this.itf.onTick(REI.getPair().toString(), REI.getTick().getBid(), REI.getTick().getAsk());
    REI = null;
  }
}
