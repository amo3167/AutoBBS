package asirikuy;

import java.util.HashMap;
import java.util.Map;

public class TradeSignal {

  public enum CommandEnum { OPEN, CLOSE, MODIFY; } 

  public CommandEnum command;

  public Map<String, Object> parameters;

  public TradeSignal  () { 
    parameters = new HashMap<String, Object>();
  }

}
