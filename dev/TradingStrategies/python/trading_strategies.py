"""
Python wrapper for TradingStrategies C library

This module provides a Python-friendly interface to the TradingStrategies
C library, enabling integration with Python trading platforms.
"""

import ctypes
import os
import sys
from ctypes import Structure, POINTER, c_int, c_double, c_char_p, c_void_p, c_char
from typing import Dict, List, Optional, Tuple, Any
from dataclasses import dataclass
from enum import IntEnum
import platform

# Determine library extension based on platform
if platform.system() == "Darwin":  # macOS
    _lib_ext = ".dylib"
elif platform.system() == "Linux":
    _lib_ext = ".so"
elif platform.system() == "Windows":
    _lib_ext = ".dll"
else:
    _lib_ext = ".so"

# Try to load the library
_lib_paths = [
    os.path.join(os.path.dirname(__file__), "..", "build", f"libtrading_strategies{_lib_ext}"),
    os.path.join(os.path.dirname(__file__), f"libtrading_strategies{_lib_ext}"),
    f"libtrading_strategies{_lib_ext}",
]

_lib = None
for path in _lib_paths:
    try:
        _lib = ctypes.CDLL(path)
        break
    except OSError:
        continue

if _lib is None:
    raise ImportError(f"Could not load TradingStrategies library. Tried: {_lib_paths}")


# Enums (matching C definitions)
class AsirikuyReturnCode(IntEnum):
    SUCCESS = 0
    NULL_POINTER = 1
    INVALID_PARAMETER = 2
    INVALID_STRATEGY = 3
    # Add more as needed


class OrderType(IntEnum):
    BUY = 0
    SELL = 1
    BUY_LIMIT = 2
    SELL_LIMIT = 3
    BUY_STOP = 4
    SELL_STOP = 5


class OrderAction(IntEnum):
    OPEN = 0
    MODIFY = 1
    CLOSE = 2


# C Structures
class PythonStrategyInput(Structure):
    """Input structure for strategy execution"""
    _fields_ = [
        ('strategy_id', c_int),
        ('instance_id', c_int),
        ('symbol', c_char_p),
        ('bid', c_double),
        ('ask', c_double),
        ('current_time', c_int),
        ('account_balance', c_double),
        ('account_equity', c_double),
        ('account_currency', c_char_p),
        ('rates_count', c_int),
        ('rates_time', POINTER(c_double)),
        ('rates_open', POINTER(c_double)),
        ('rates_high', POINTER(c_double)),
        ('rates_low', POINTER(c_double)),
        ('rates_close', POINTER(c_double)),
        ('rates_volume', POINTER(c_double)),
        ('settings', POINTER(c_double)),
        ('settings_count', c_int),
        ('orders_count', c_int),
        ('order_numbers', POINTER(c_int)),
        ('order_types', POINTER(c_int)),
        ('order_open_prices', POINTER(c_double)),
        ('order_stop_loss', POINTER(c_double)),
        ('order_take_profit', POINTER(c_double)),
        ('order_statuses', POINTER(c_int)),
        ('order_timestamps', POINTER(c_int)),
    ]


class PythonStrategyOutput(Structure):
    """Output structure from strategy execution"""
    _fields_ = [
        ('return_code', c_int),
        ('signals_count', c_int),
        ('signal_types', POINTER(c_int)),
        ('signal_prices', POINTER(c_double)),
        ('signal_stop_loss', POINTER(c_double)),
        ('signal_take_profit', POINTER(c_double)),
        ('signal_actions', POINTER(c_int)),
        ('ui_values_count', c_int),
        ('ui_names', POINTER(c_char_p)),
        ('ui_values', POINTER(c_double)),
        ('status_message', c_char * 256),
        ('_internal_data', c_void_p),
    ]


# Define function signatures
_lib.trading_strategies_run.argtypes = [
    POINTER(PythonStrategyInput),
    POINTER(PythonStrategyOutput)
]
_lib.trading_strategies_run.restype = c_int

_lib.trading_strategies_free_output.argtypes = [POINTER(PythonStrategyOutput)]
_lib.trading_strategies_free_output.restype = None

_lib.trading_strategies_get_error_message.argtypes = [c_int]
_lib.trading_strategies_get_error_message.restype = c_char_p

_lib.trading_strategies_get_version.argtypes = []
_lib.trading_strategies_get_version.restype = c_char_p

_lib.trading_strategies_init.argtypes = []
_lib.trading_strategies_init.restype = c_int

_lib.trading_strategies_cleanup.argtypes = []
_lib.trading_strategies_cleanup.restype = None


@dataclass
class TradingSignal:
    """Trading signal from strategy"""
    action: str  # 'OPEN', 'MODIFY', 'CLOSE'
    order_type: str  # 'BUY', 'SELL', etc.
    price: float
    stop_loss: float
    take_profit: float


@dataclass
class MarketData:
    """Market data for strategy execution"""
    symbol: str
    bid: float
    ask: float
    current_time: int  # Unix timestamp
    rates: Dict[str, Any]  # OHLCV data by timeframe


@dataclass
class AccountInfo:
    """Account information"""
    balance: float
    equity: float
    currency: str


class TradingStrategies:
    """Python wrapper for TradingStrategies C library"""
    
    def __init__(self):
        """Initialize the TradingStrategies wrapper"""
        result = _lib.trading_strategies_init()
        if result != AsirikuyReturnCode.SUCCESS:
            raise RuntimeError(f"Failed to initialize library: {result}")
    
    def __del__(self):
        """Cleanup on destruction"""
        if _lib:
            _lib.trading_strategies_cleanup()
    
    def run_strategy(
        self,
        strategy_id: int,
        instance_id: int,
        symbol: str,
        market_data: MarketData,
        account_info: AccountInfo,
        settings: Dict[str, float],
        open_orders: List[Dict[str, Any]],
        rates_data: Dict[str, List[Dict[str, float]]]
    ) -> Tuple[int, List[TradingSignal], Dict[str, float], str]:
        """
        Run trading strategy
        
        Args:
            strategy_id: Strategy identifier (e.g., 29 for AUTOBBS)
            instance_id: Instance identifier for this strategy instance
            symbol: Trading symbol (e.g., "EURUSD")
            market_data: Current market data
            account_info: Account information
            settings: Strategy settings dictionary
            open_orders: List of open orders
            rates_data: OHLCV data by timeframe
            
        Returns:
            Tuple of (return_code, signals, ui_values, status_message)
        """
        # Prepare input structure
        input_data = self._prepare_input(
            strategy_id, instance_id, symbol, market_data,
            account_info, settings, open_orders, rates_data
        )
        
        # Prepare output structure
        output = PythonStrategyOutput()
        
        try:
            # Call C function
            result = _lib.trading_strategies_run(
                ctypes.byref(input_data),
                ctypes.byref(output)
            )
            
            # Extract results
            signals = self._extract_signals(output)
            ui_values = self._extract_ui_values(output)
            status = output.status_message.decode('utf-8', errors='ignore')
            
            return result, signals, ui_values, status
            
        finally:
            # Free allocated memory
            _lib.trading_strategies_free_output(ctypes.byref(output))
            self._free_input_arrays(input_data)
    
    def _prepare_input(
        self,
        strategy_id: int,
        instance_id: int,
        symbol: str,
        market_data: MarketData,
        account_info: AccountInfo,
        settings: Dict[str, float],
        open_orders: List[Dict[str, Any]],
        rates_data: Dict[str, List[Dict[str, float]]]
    ) -> PythonStrategyInput:
        """Convert Python data to C structure"""
        # For now, use primary timeframe (can be extended)
        primary_timeframe = list(rates_data.keys())[0] if rates_data else "M1"
        rates = rates_data.get(primary_timeframe, [])
        
        if not rates:
            raise ValueError("No rates data provided")
        
        rates_count = len(rates)
        
        # Allocate arrays for rates
        rates_time = (c_double * rates_count)()
        rates_open = (c_double * rates_count)()
        rates_high = (c_double * rates_count)()
        rates_low = (c_double * rates_count)()
        rates_close = (c_double * rates_count)()
        rates_volume = (c_double * rates_count)()
        
        for i, bar in enumerate(rates):
            rates_time[i] = bar.get('time', 0)
            rates_open[i] = bar.get('open', 0.0)
            rates_high[i] = bar.get('high', 0.0)
            rates_low[i] = bar.get('low', 0.0)
            rates_close[i] = bar.get('close', 0.0)
            rates_volume[i] = bar.get('volume', 0.0)
        
        # Convert settings to array
        # This is a simplified version - actual implementation would need
        # to map settings names to indices
        settings_list = list(settings.values())
        settings_count = len(settings_list)
        settings_array = (c_double * settings_count)(*settings_list)
        
        # Convert orders
        orders_count = len(open_orders)
        if orders_count > 0:
            order_numbers = (c_int * orders_count)()
            order_types = (c_int * orders_count)()
            order_open_prices = (c_double * orders_count)()
            order_stop_loss = (c_double * orders_count)()
            order_take_profit = (c_double * orders_count)()
            order_statuses = (c_int * orders_count)()
            order_timestamps = (c_int * orders_count)()
            
            for i, order in enumerate(open_orders):
                order_numbers[i] = order.get('number', 0)
                order_types[i] = order.get('type', 0)
                order_open_prices[i] = order.get('open_price', 0.0)
                order_stop_loss[i] = order.get('stop_loss', 0.0)
                order_take_profit[i] = order.get('take_profit', 0.0)
                order_statuses[i] = order.get('status', 0)
                order_timestamps[i] = order.get('timestamp', 0)
        else:
            order_numbers = None
            order_types = None
            order_open_prices = None
            order_stop_loss = None
            order_take_profit = None
            order_statuses = None
            order_timestamps = None
        
        # Create input structure
        input_data = PythonStrategyInput(
            strategy_id=strategy_id,
            instance_id=instance_id,
            symbol=symbol.encode('utf-8'),
            bid=market_data.bid,
            ask=market_data.ask,
            current_time=market_data.current_time,
            account_balance=account_info.balance,
            account_equity=account_info.equity,
            account_currency=account_info.currency.encode('utf-8'),
            rates_count=rates_count,
            rates_time=ctypes.cast(rates_time, POINTER(c_double)),
            rates_open=ctypes.cast(rates_open, POINTER(c_double)),
            rates_high=ctypes.cast(rates_high, POINTER(c_double)),
            rates_low=ctypes.cast(rates_low, POINTER(c_double)),
            rates_close=ctypes.cast(rates_close, POINTER(c_double)),
            rates_volume=ctypes.cast(rates_volume, POINTER(c_double)),
            settings=ctypes.cast(settings_array, POINTER(c_double)),
            settings_count=settings_count,
            orders_count=orders_count,
            order_numbers=ctypes.cast(order_numbers, POINTER(c_int)) if order_numbers else None,
            order_types=ctypes.cast(order_types, POINTER(c_int)) if order_types else None,
            order_open_prices=ctypes.cast(order_open_prices, POINTER(c_double)) if order_open_prices else None,
            order_stop_loss=ctypes.cast(order_stop_loss, POINTER(c_double)) if order_stop_loss else None,
            order_take_profit=ctypes.cast(order_take_profit, POINTER(c_double)) if order_take_profit else None,
            order_statuses=ctypes.cast(order_statuses, POINTER(c_int)) if order_statuses else None,
            order_timestamps=ctypes.cast(order_timestamps, POINTER(c_int)) if order_timestamps else None,
        )
        
        # Store arrays for cleanup
        input_data._arrays = {
            'rates_time': rates_time,
            'rates_open': rates_open,
            'rates_high': rates_high,
            'rates_low': rates_low,
            'rates_close': rates_close,
            'rates_volume': rates_volume,
            'settings': settings_array,
            'order_numbers': order_numbers,
            'order_types': order_types,
            'order_open_prices': order_open_prices,
            'order_stop_loss': order_stop_loss,
            'order_take_profit': order_take_profit,
            'order_statuses': order_statuses,
            'order_timestamps': order_timestamps,
        }
        
        return input_data
    
    def _extract_signals(self, output: PythonStrategyOutput) -> List[TradingSignal]:
        """Extract trading signals from output"""
        signals = []
        
        if output.signals_count == 0 or output.signal_types is None:
            return signals
        
        action_map = {0: 'OPEN', 1: 'MODIFY', 2: 'CLOSE'}
        type_map = {0: 'BUY', 1: 'SELL', 2: 'BUY_LIMIT', 3: 'SELL_LIMIT', 4: 'BUY_STOP', 5: 'SELL_STOP'}
        
        for i in range(output.signals_count):
            signal = TradingSignal(
                action=action_map.get(output.signal_actions[i], 'UNKNOWN'),
                order_type=type_map.get(output.signal_types[i], 'UNKNOWN'),
                price=output.signal_prices[i] if output.signal_prices else 0.0,
                stop_loss=output.signal_stop_loss[i] if output.signal_stop_loss else 0.0,
                take_profit=output.signal_take_profit[i] if output.signal_take_profit else 0.0,
            )
            signals.append(signal)
        
        return signals
    
    def _extract_ui_values(self, output: PythonStrategyOutput) -> Dict[str, float]:
        """Extract UI values from output"""
        ui_values = {}
        
        if output.ui_values_count == 0:
            return ui_values
        
        for i in range(output.ui_values_count):
            if output.ui_names and output.ui_names[i]:
                name = output.ui_names[i].decode('utf-8', errors='ignore')
                value = output.ui_values[i] if output.ui_values else 0.0
                ui_values[name] = value
        
        return ui_values
    
    def _free_input_arrays(self, input_data: PythonStrategyInput):
        """Free arrays allocated for input (Python manages this)"""
        # Arrays are automatically freed when Python objects are garbage collected
        pass
    
    @staticmethod
    def get_error_message(return_code: int) -> str:
        """Get human-readable error message"""
        msg = _lib.trading_strategies_get_error_message(return_code)
        if msg:
            return msg.decode('utf-8', errors='ignore')
        return f"Unknown error code: {return_code}"
    
    @staticmethod
    def get_version() -> str:
        """Get library version"""
        version = _lib.trading_strategies_get_version()
        if version:
            return version.decode('utf-8', errors='ignore')
        return "Unknown"


# Example usage
if __name__ == "__main__":
    # Initialize
    strategies = TradingStrategies()
    
    # Prepare data
    market_data = MarketData(
        symbol="EURUSD",
        bid=1.1000,
        ask=1.1002,
        current_time=1234567890,
        rates={}
    )
    
    account_info = AccountInfo(
        balance=10000.0,
        equity=10000.0,
        currency="USD"
    )
    
    settings = {
        "ACCOUNT_RISK_PERCENT": 2.0,
        "MAX_OPEN_ORDERS": 1.0,
        # ... more settings
    }
    
    # Run strategy
    return_code, signals, ui_values, status = strategies.run_strategy(
        strategy_id=29,  # AUTOBBS
        instance_id=0,
        symbol="EURUSD",
        market_data=market_data,
        account_info=account_info,
        settings=settings,
        open_orders=[],
        rates_data={"M1": []}  # Empty for example
    )
    
    print(f"Return code: {return_code}")
    print(f"Signals: {signals}")
    print(f"UI Values: {ui_values}")
    print(f"Status: {status}")

