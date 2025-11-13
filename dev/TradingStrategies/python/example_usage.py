"""
Example usage of TradingStrategies Python wrapper

This demonstrates how to use the TradingStrategies library from Python
with a broker REST API integration.
"""

from trading_strategies import TradingStrategies, MarketData, AccountInfo, TradingSignal
from typing import List, Dict, Any
import time


class BrokerAPI:
    """Mock broker API for demonstration"""
    
    def get_market_data(self, symbol: str) -> Dict[str, Any]:
        """Get current market data from broker"""
        # In real implementation, this would call broker REST API
        return {
            "symbol": symbol,
            "bid": 1.1000,
            "ask": 1.1002,
            "timestamp": int(time.time())
        }
    
    def get_rates(self, symbol: str, timeframe: str, count: int) -> List[Dict[str, float]]:
        """Get historical rates from broker"""
        # In real implementation, this would call broker REST API
        # Example: GET /api/v1/rates?symbol=EURUSD&timeframe=M1&count=1000
        return [
            {
                "time": int(time.time()) - i * 60,
                "open": 1.1000 + i * 0.0001,
                "high": 1.1005 + i * 0.0001,
                "low": 1.0995 + i * 0.0001,
                "close": 1.1002 + i * 0.0001,
                "volume": 1000.0
            }
            for i in range(count)
        ]
    
    def get_account_info(self) -> Dict[str, Any]:
        """Get account information from broker"""
        # In real implementation, this would call broker REST API
        return {
            "balance": 10000.0,
            "equity": 10000.0,
            "currency": "USD"
        }
    
    def get_open_orders(self, symbol: str) -> List[Dict[str, Any]]:
        """Get open orders from broker"""
        # In real implementation, this would call broker REST API
        return []
    
    def place_order(self, signal: TradingSignal, symbol: str) -> Dict[str, Any]:
        """Place order via broker REST API"""
        # In real implementation, this would call broker REST API
        # Example: POST /api/v1/orders
        print(f"Placing order: {signal.action} {signal.order_type} at {signal.price}")
        return {"order_id": "12345", "status": "pending"}


class TradingPlatform:
    """Trading platform integrating TradingStrategies with broker API"""
    
    def __init__(self, broker_api: BrokerAPI):
        self.broker = broker_api
        self.strategies = TradingStrategies()
        self.symbol = "EURUSD"
        self.strategy_id = 29  # AUTOBBS
        self.instance_id = 0
    
    def run_strategy_cycle(self):
        """Run one cycle of strategy execution"""
        # 1. Get market data from broker
        market_data_dict = self.broker.get_market_data(self.symbol)
        rates_data = {
            "M1": self.broker.get_rates(self.symbol, "M1", 1000),
            "M5": self.broker.get_rates(self.symbol, "M5", 200),
            "M15": self.broker.get_rates(self.symbol, "M15", 100),
            "H1": self.broker.get_rates(self.symbol, "H1", 24),
            "H4": self.broker.get_rates(self.symbol, "H4", 6),
            "D1": self.broker.get_rates(self.symbol, "D1", 30),
        }
        
        # 2. Get account info
        account_dict = self.broker.get_account_info()
        
        # 3. Get open orders
        open_orders = self.broker.get_open_orders(self.symbol)
        
        # 4. Prepare data structures
        market_data = MarketData(
            symbol=market_data_dict["symbol"],
            bid=market_data_dict["bid"],
            ask=market_data_dict["ask"],
            current_time=market_data_dict["timestamp"],
            rates=rates_data
        )
        
        account_info = AccountInfo(
            balance=account_dict["balance"],
            equity=account_dict["equity"],
            currency=account_dict["currency"]
        )
        
        # 5. Strategy settings
        settings = {
            "ACCOUNT_RISK_PERCENT": 2.0,
            "MAX_OPEN_ORDERS": 1.0,
            "IS_BACKTESTING": 0.0,
            "OPERATIONAL_MODE": 1.0,  # MODE_TRADE
            "TIMEFRAME": 1.0,  # M1
            "MAX_SPREAD": 0.0003,
            "SL_ATR_MULTIPLIER": 2.0,
            "TP_ATR_MULTIPLIER": 3.0,
            # ... more settings as needed
        }
        
        # 6. Run strategy
        return_code, signals, ui_values, status = self.strategies.run_strategy(
            strategy_id=self.strategy_id,
            instance_id=self.instance_id,
            symbol=self.symbol,
            market_data=market_data,
            account_info=account_info,
            settings=settings,
            open_orders=open_orders,
            rates_data=rates_data
        )
        
        # 7. Check return code
        if return_code != 0:  # SUCCESS
            error_msg = TradingStrategies.get_error_message(return_code)
            print(f"Strategy error: {error_msg}")
            return
        
        # 8. Process signals
        for signal in signals:
            if signal.action == "OPEN":
                # Place new order
                result = self.broker.place_order(signal, self.symbol)
                print(f"Order placed: {result}")
            elif signal.action == "MODIFY":
                # Modify existing order
                print(f"Modify order: {signal}")
            elif signal.action == "CLOSE":
                # Close existing order
                print(f"Close order: {signal}")
        
        # 9. Log UI values (for monitoring)
        if ui_values:
            print(f"UI Values: {ui_values}")
        
        print(f"Status: {status}")
    
    def run_continuous(self, interval_seconds: int = 60):
        """Run strategy continuously"""
        print(f"Starting trading platform for {self.symbol}")
        print(f"Strategy ID: {self.strategy_id}")
        print(f"Update interval: {interval_seconds} seconds")
        
        try:
            while True:
                self.run_strategy_cycle()
                time.sleep(interval_seconds)
        except KeyboardInterrupt:
            print("\nStopping trading platform...")


if __name__ == "__main__":
    # Create broker API (replace with real broker API implementation)
    broker = BrokerAPI()
    
    # Create trading platform
    platform = TradingPlatform(broker)
    
    # Run one cycle for demonstration
    print("Running single strategy cycle...")
    platform.run_strategy_cycle()
    
    # Uncomment to run continuously:
    # platform.run_continuous(interval_seconds=60)

