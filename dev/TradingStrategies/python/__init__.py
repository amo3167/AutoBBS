"""
TradingStrategies Python Package

Python wrapper for the TradingStrategies C library.
"""

from .trading_strategies import (
    TradingStrategies,
    TradingSignal,
    MarketData,
    AccountInfo,
    AsirikuyReturnCode,
    OrderType,
    OrderAction,
)

__version__ = "1.0.0"
__all__ = [
    "TradingStrategies",
    "TradingSignal",
    "MarketData",
    "AccountInfo",
    "AsirikuyReturnCode",
    "OrderType",
    "OrderAction",
]

