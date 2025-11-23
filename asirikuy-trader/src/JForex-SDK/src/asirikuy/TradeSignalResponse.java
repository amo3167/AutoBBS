
package asirikuy;

public class TradeSignalResponse {

  public String identifier;
  public TradeSignal signal;
  public boolean result;
  public String message;

  public TradeSignalResponse() {
    result = false;
    message = new String("");
  }

  public boolean equals(TradeSignalResponse other) { 
    return identifier.equals(other.identifier);
  }

}
