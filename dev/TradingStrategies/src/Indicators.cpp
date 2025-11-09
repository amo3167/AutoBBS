// Indicators.cpp
// Implementation of Indicators class

#include "Indicators.hpp"

// Include C wrapper functions from EasyTrade
extern "C" {
    #include "EasyTradeCWrapper.hpp"
}

namespace trading {

Indicators::Indicators(const StrategyContext& context)
    : context_(context) {
}

Indicators::~Indicators() {
}

double Indicators::getATR(int ratesIndex, int period, int shift) const {
    return iAtr(ratesIndex, period, shift);
}

double Indicators::getRSI(int ratesIndex, int period, int shift) const {
    return iRSI(ratesIndex, period, shift);
}

double Indicators::getMA(MAPrice priceType, int ratesIndex, int period, int shift) const {
    return iMA(static_cast<int>(priceType), ratesIndex, period, shift);
}

double Indicators::getMACD(int ratesIndex, int fastPeriod, int slowPeriod, 
                           int signalPeriod, MACDSignal signal, int shift) const {
    return iMACD(ratesIndex, fastPeriod, slowPeriod, signalPeriod, 
                 static_cast<int>(signal), shift);
}

double Indicators::getMACDAll(int ratesIndex, int fastPeriod, int slowPeriod, 
                              int signalPeriod, int shift, 
                              double* macd, double* signal, double* histogram) const {
    return iMACDAll(ratesIndex, fastPeriod, slowPeriod, signalPeriod, 
                    shift, macd, signal, histogram);
}

double Indicators::getBollingerBands(int ratesIndex, int period, double deviation, 
                                     BBandsBuffer buffer, int shift) const {
    return iBBands(ratesIndex, period, deviation, static_cast<int>(buffer), shift);
}

double Indicators::getBBandStop(int ratesIndex, int period, double deviation,
                                int* trend, double* stopPrice, int* index) const {
    return iBBandStop(ratesIndex, period, deviation, trend, stopPrice, index);
}

double Indicators::getStochastic(int ratesIndex, int period, int k, int d, 
                                 StochasticSignal signal, int shift) const {
    return iSTO(ratesIndex, period, k, d, static_cast<int>(signal), shift);
}

double Indicators::getStdDev(int ratesIndex, MAPrice priceType, int period, int shift) const {
    return iStdev(ratesIndex, static_cast<int>(priceType), period, shift);
}

double Indicators::getCCI(int ratesIndex, int period, int shift) const {
    return iCCI(ratesIndex, period, shift);
}

double Indicators::getHigh(int ratesIndex, int shift) const {
    return iHigh(ratesIndex, shift);
}

double Indicators::getLow(int ratesIndex, int shift) const {
    return iLow(ratesIndex, shift);
}

double Indicators::getOpen(int ratesIndex, int shift) const {
    return iOpen(ratesIndex, shift);
}

double Indicators::getClose(int ratesIndex, int shift) const {
    return iClose(ratesIndex, shift);
}

} // namespace trading
