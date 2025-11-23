package asirikuy;

import com.oanda.fxtrade.api.*;

public interface OandaWrapper
{
   public void onTick(String symbol, double bid, double ask);
}
