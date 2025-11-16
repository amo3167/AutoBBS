/*
 * MACD Daily Strategy Module
 * 
 * Provides MACD Daily strategy execution function with extensive symbol-specific
 * configurations and advanced features. This strategy uses daily MACD indicators,
 * volume analysis, and various filters to determine entry and exit signals.
 * 
 * Features:
 * - Symbol-specific MACD parameters (fast/slow/signal periods)
 * - Volume confirmation (CMF Volume, Volume Gap, standard volume)
 * - Weekly baseline filtering
 * - BeiLi (divergence) detection
 * - Maximum level risk control
 * - End-of-day entry support
 * - ATR-based stop loss management
 * - Advanced stop loss management using hourly close prices
 * - ASI (Accumulation Swing Index) indicator support
 * - Late entry prevention
 * - Next day bar filtering
 * - Special stop loss handling modes
 * - Enhanced logging with detailed status messages
 * - Flexible baseline calculations (MA20 or MA50 depending on symbol)
 * - Sunday trading restrictions for crypto symbols
 * 
 * Supported symbols include: BTCUSD, ETHUSD, SpotCrudeUSD, XAGUSD, XTIUSD,
 * XAUUSD, XAUEUR, GBPJPY, GBPCHF, AUDJPY, EURJPY, CADJPY, GBPAUD, GBPUSD,
 * AUDNZD, and default (EURUSD).
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "InstanceStates.h"
#include "strategies/autobbs/trend/macd/MACDDailyStrategy.h"
#include "strategies/autobbs/trend/macd/MACDOrderSplitting.h"

/* Strategy configuration constants */
#define SPLIT_TRADE_MODE_MACD_DAILY 24     /* Split trade mode for MACD Daily strategy */
#define TP_MODE_DAILY_ATR 3                /* Take profit mode: daily ATR */
#define TP_MODE_STANDARD 1                 /* Standard trade mode */

/* Default MACD parameters (standard) */
#define MACD_FAST_PERIOD_STANDARD 12       /* Standard fast MA period */
#define MACD_SLOW_PERIOD_STANDARD 26       /* Standard slow MA period */
#define MACD_SIGNAL_STANDARD 9              /* Standard signal MA period */

/* Short-term MACD parameters (used for most symbols) */
#define MACD_FAST_PERIOD_SHORT 5           /* Short-term fast MA period */
#define MACD_SLOW_PERIOD_SHORT 10          /* Short-term slow MA period */
#define MACD_SIGNAL_SHORT 5                /* Short-term signal MA period */

/* Time constants */
#define START_HOUR_DEFAULT 1               /* Default start hour for trading */
#define END_OF_DAY_HOUR 23                 /* Hour for end-of-day entry check */
#define MACD_LIMIT_DIVISOR 2               /* Divisor for MACD limit at EOD (level / 2) */

/* ATR and price gap constants */
#define ATR_PERIOD_FOR_CHECK 5             /* ATR period for volatility check */
#define PRICE_GAP_THRESHOLD_FACTOR 0.2     /* Factor for price gap threshold (20% of daily ATR) */
#define ATR_PRICE_PERCENTAGE_FACTOR 0.01   /* ATR as percentage of price (1%) */
#define ATR_PRICE_MULTIPLIER 0.55          /* Additional multiplier for ATR price check */

/* MA baseline period */
#define MA_BASELINE_PERIOD_20 20           /* MA period for 20-day baseline */
#define MA_BASELINE_PERIOD_50 50           /* MA period for 50-day baseline */

/* Weekly baseline calculation */
#define WEEKLY_SR_LEVELS_LONG 26           /* Number of bars for long-term weekly SR levels */
#define WEEKLY_SR_LEVELS_SHORT 9           /* Number of bars for short-term weekly SR levels */

/* Historical MACD bars to check */
#define HISTORICAL_MACD_BARS 5             /* Number of historical MACD bars to check */

/**
 * @brief Symbol-specific configuration structure for MACD Daily strategy.
 * 
 * This structure holds all symbol-specific parameters and flags that control
 * the behavior of the MACD Daily strategy for different trading instruments.
 */
typedef struct {
	/* MACD entry thresholds and levels */
	double level;                          /* Entry threshold level (0 = no threshold) */
	double maxLevel;                       /* Maximum MACD level before blocking entry */
	double histLevel;                      /* Historical MACD level for late entry check */
	double nextMACDRange;                  /* Next day bar MACD range filter (0 = disabled) */
	
	/* MACD parameters */
	int fastMAPeriod;                      /* Fast moving average period */
	int slowMAPeriod;                      /* Slow moving average period */
	int signalMAPeriod;                    /* Signal moving average period */
	
	/* Risk management */
	double stopLossMultiplier;             /* Stop loss multiplier (e.g., 1.8 for 1.8 * ATR) */
	double maxRisk;                        /* Maximum risk multiplier */
	int range;                             /* BeiLi turning point range check */
	
	/* Baseline configuration */
	int baselinePeriod;                    /* Moving average period for baseline (20 or 50) */
	BOOL useWeeklyBaseLine;                /* Enable weekly baseline filtering */
	
	/* Filter flags */
	BOOL isVolumeControl;                  /* Enable volume control filter */
	BOOL isEnableBeiLi;                    /* Enable BeiLi (divergence) detection */
	BOOL isEnableSlow;                     /* Require slow line to be above/below zero */
	BOOL isEnableATR;                      /* Enable ATR volatility filter */
	BOOL isEnableCMFVolume;                /* Enable CMF volume filter */
	BOOL isEnableCMFVolumeGap;              /* Enable CMF volume gap filter */
	BOOL isEnableMaxLevelBuy;              /* Enable max level check for buy signals */
	BOOL isEnableMaxLevelSell;             /* Enable max level check for sell signals */
	BOOL isEnableMaxLevel;                 /* Enable general max level check */
	
	/* Volume risk flags */
	BOOL isVolumeControlRisk;              /* Use volume control for risk adjustment */
	BOOL isCMFVolumeRisk;                  /* Use CMF volume for risk adjustment */
	BOOL isCMFVolumeGapRisk;               /* Use CMF volume gap for risk adjustment */
	BOOL isAllVolumeRisk;                  /* Require all volume conditions for risk adjustment */
	
	/* Advanced features */
	BOOL isDailyOnly;                      /* Only trade on new daily bar */
	BOOL isEnableEntryEOD;                 /* Enable end-of-day entry (23:00) */
	BOOL isEnableLate;                    /* Enable late entry prevention */
	BOOL isEnableNextdayBar;               /* Enable next day bar filtering */
	BOOL isEnableNoStopLoss;               /* Special no-stop-loss mode for adding positions */
	BOOL isEnableMaxLevelRiskControl;     /* Enable max level risk control */
	
	/* Order management */
	BOOL stopMovingBackSL;                 /* Allow stop loss to move backward */
	int shiftPreDayBar;                    /* Shift index for previous day bar check */
	int startHour;                         /* Start hour for trading (default: 1) */
	
	/* Lot size configuration */
	double minLotSize;                     /* Minimum lot size (0 = use default) */
	double volumeStep;                     /* Volume step size (0 = use default) */
	
	/* Special logic flags */
	BOOL skipSunday;                       /* Skip trading on Sunday (for crypto) */
	BOOL useDynamicLevel;                  /* Calculate level dynamically based on price */
	BOOL useTimeBasedMACD;                 /* Use time-based MACD parameters (for crypto) */
} MACDSymbolConfig;

/**
 * @brief Initializes symbol-specific configuration for MACD Daily strategy.
 * 
 * This function configures all symbol-specific parameters based on the trading
 * symbol. Each symbol has unique characteristics that require different MACD
 * parameters, filters, and risk management settings.
 * 
 * Symbol-Specific Trading Logic:
 * 
 * BTCUSD/ETHUSD (Cryptocurrencies):
 * - Dynamic level calculation: 0.5% of current price
 * - Dynamic max level: 5% of current price
 * - Time-based MACD: Uses 5/10/5 before Jan 18, 2021, then 7/14/7
 * - Uses MA50 baseline for trend confirmation
 * - Stop loss: 1.8 * daily ATR
 * - Max risk: 1.5x
 * - No volume control (crypto markets are 24/7)
 * - Sunday trading restriction
 * - End-of-day entry enabled
 * - Range: 5 (tighter BeiLi check)
 * 
 * SpotCrudeUSD/XTIUSD (Oil/Commodities):
 * - Fixed level: 0.35
 * - Dynamic max level: 1% of current price
 * - MACD: 5/10/5 (short-term)
 * - Uses weekly baseline for trend confirmation
 * - Stop loss: 1.8 * daily ATR
 * - Max risk: 1.5x
 * - Weekly baseline filtering enabled
 * - End-of-day entry enabled
 * - Lot size: 0.01 for SpotCrudeUSD, 0.5 for XTIUSD
 * - Range: 10
 * 
 * XAUUSD (Gold):
 * - Fixed level: 2.0
 * - Dynamic max level: min(10, (price-1500)/300 + 10)
 * - MACD: 5/10/5 (short-term)
 * - Uses MA50 baseline
 * - Max risk: 2.0x
 * - CMF volume gap risk adjustment enabled
 * - No end-of-day entry
 * - Range: 10
 * 
 * XAGUSD (Silver):
 * - Fixed level: 0.05
 * - Dynamic max level: min(0.2, 0.4*(price-15)/5 + 0.2)
 * - MACD: 5/10/5 (short-term)
 * - Uses MA50 baseline
 * - ATR range: 1% of price
 * - Next day bar filtering enabled
 * - Max risk: 1.5x
 * - Range: 10
 * 
 * XAUEUR (Gold in EUR):
 * - Fixed level: 2.0
 * - Fixed max level: 8.0
 * - MACD: 5/10/5 (short-term)
 * - Max level checks enabled for both buy and sell
 * - Range: default (10000)
 * 
 * GBPJPY (Major Forex):
 * - Fixed level: 0.1
 * - Fixed max level: 0.9
 * - MACD: 5/10/5 (short-term)
 * - Volume control enabled (requires volume1 > volume2)
 * - ATR filter enabled
 * - CMF volume filter enabled
 * - Next day bar filtering enabled (nextMACDRange = 0.2)
 * - Max risk: 2.0x
 * - End-of-day entry enabled
 * - Range: 10
 * 
 * GBPCHF (Forex):
 * - Fixed level: 0.002
 * - Fixed max level: 0.008
 * - MACD: 5/10/5 (short-term)
 * - BeiLi detection disabled
 * - Minimal filters (no volume, no ATR)
 * - Range: default (10000)
 * 
 * AUDJPY/EURJPY/CADJPY (JPY Pairs):
 * - Fixed level: 0.1
 * - Fixed max level: 0.5 (AUDJPY) or 0.9 (EURJPY/CADJPY)
 * - MACD: 5/10/5 (short-term)
 * - EURJPY/CADJPY: CMF volume gap filter enabled
 * - CADJPY: ATR filter enabled
 * - Range: default (10000)
 * 
 * GBPAUD/GBPUSD (GBP Pairs):
 * - Fixed level: 0.001
 * - Fixed max level: 0.008 (GBPAUD) or 0.007 (GBPUSD)
 * - MACD: 5/10/5 (short-term)
 * - Uses MA50 baseline
 * - Max risk: 1.5x
 * - GBPUSD: Next day bar filtering enabled
 * - Range: 10
 * 
 * AUDNZD (Forex):
 * - Fixed level: 0.0025
 * - No max level (0)
 * - MACD: 12/26/9 (standard parameters)
 * - Standard configuration
 * 
 * EURUSD (Default):
 * - Fixed level: 0 (no threshold)
 * - Fixed max level: 0.005
 * - MACD: 5/10/5 (short-term)
 * - Slow line filter enabled
 * - Daily only trading
 * 
 * @param pConfig Configuration structure to populate
 * @param pParams Strategy parameters containing symbol and price information
 * @param pIndicators Strategy indicators (for setting lot sizes, risk cap, etc.)
 * @param pBase_Indicators Base indicators (for ATR calculations)
 * @param timeInfo Current time information for time-based logic
 * @param dailyBaseLine Pointer to daily baseline (may be modified)
 * @param atrRange Pointer to ATR range (may be modified)
 */
static void initializeSymbolConfig(MACDSymbolConfig* pConfig, StrategyParams* pParams, 
	Indicators* pIndicators, Base_Indicators* pBase_Indicators, 
	struct tm* timeInfo, double* dailyBaseLine, double* atrRange)
{
	/* Initialize with default values */
	memset(pConfig, 0, sizeof(MACDSymbolConfig));
	
	pConfig->fastMAPeriod = MACD_FAST_PERIOD_SHORT;
	pConfig->slowMAPeriod = MACD_SLOW_PERIOD_SHORT;
	pConfig->signalMAPeriod = MACD_SIGNAL_SHORT;
	pConfig->stopLossMultiplier = 1.0;
	pConfig->maxRisk = 2.0;
	pConfig->range = 10000;
	pConfig->baselinePeriod = MA_BASELINE_PERIOD_20;
	pConfig->isVolumeControl = TRUE;
	pConfig->isEnableBeiLi = TRUE;
	pConfig->isEnableSlow = TRUE;
	pConfig->isEnableATR = TRUE;
	pConfig->isDailyOnly = TRUE;
	pConfig->stopMovingBackSL = FALSE;
	pConfig->shiftPreDayBar = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	pConfig->startHour = START_HOUR_DEFAULT;
	pConfig->minLotSize = 0.0;  /* 0 = use default */
	pConfig->volumeStep = 0.0;  /* 0 = use default */
	*atrRange = 0.01 * 0.55;

	/* Configure based on symbol */
	/* 
	 * SYMBOL-SPECIFIC TRADING LOGIC:
	 * Each symbol has unique characteristics (volatility, liquidity, trading hours, etc.)
	 * that require different MACD parameters, entry thresholds, and filtering logic.
	 * 
	 * Key differences between symbols:
	 * - Level thresholds: Higher for volatile assets (crypto, gold), lower for forex
	 * - MACD periods: Short-term (5/10/5) for most, longer (7/14/7) for crypto after 2021
	 * - Volume filters: Enabled for GBPJPY (high volume sensitivity), disabled for others
	 * - Weekly baseline: Used for commodities (oil, gold) to filter against weekly trend
	 * - Max level checks: Prevent entry when MACD is too extended (overbought/oversold)
	 * - EOD entry: Allow late-day entries for some symbols (crypto, oil)
	 * - Stop loss behavior: Moving back SL for crypto/gold, fixed for oil
	 */
	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		/* Cryptocurrency configuration
		 * Trading Logic:
		 * - High volatility requires dynamic level calculation (0.5% of price)
		 * - MACD parameters: 5/10/5 for backtesting (5 days/week data), 7/14/7 for live (7 days/week)
		 * - No volume control: Crypto markets operate 24/7 with consistent volume
		 * - Slow line filter: Ensures trend is established before entry
		 * - EOD entry enabled: Allows late-day entries when MACD limit is relaxed
		 * - Sunday skip: Crypto markets have lower liquidity on weekends
		 * - Moving stop loss: Protects profits in volatile markets
		 */
		pConfig->level = 0.005 * pParams->bidAsk.ask[0];
		pConfig->maxLevel = 0.05 * pParams->bidAsk.ask[0];
		pConfig->histLevel = 0.01;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = TRUE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isEnableMaxLevelBuy = FALSE;
		pConfig->isEnableMaxLevelSell = FALSE;
		pConfig->useWeeklyBaseLine = FALSE;
		
		/* MACD parameters based on backtest mode and date:
		 * - Backtesting before Jan 18, 2021: Use 5/10/5 (short-term) because historical data only has 5 days/week
		 * - Live trading OR backtesting after Jan 18, 2021: Use 7/14/7 (longer-term) because we have full 7 days/week data
		 */
		BOOL isBacktesting = (BOOL)pParams->settings[IS_BACKTESTING];
		BOOL isAfterCutoffDate = (timeInfo->tm_year > (2021 - 1900) ||
			(timeInfo->tm_year == (2021 - 1900) && timeInfo->tm_mon > 0) ||
			(timeInfo->tm_year == (2021 - 1900) && timeInfo->tm_mon == 0 && timeInfo->tm_mday >= 18));
		
		if (isBacktesting == FALSE || isAfterCutoffDate == TRUE)
		{
			pConfig->fastMAPeriod = 7;
			pConfig->slowMAPeriod = 14;
			pConfig->signalMAPeriod = 7;
		}

		pConfig->stopLossMultiplier = 1.8;
		pConfig->maxRisk = 1.5;
		pConfig->isDailyOnly = FALSE;
		pConfig->shiftPreDayBar = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
		pConfig->stopMovingBackSL = TRUE;
		pConfig->isEnableEntryEOD = TRUE;
		pConfig->isEnableLate = FALSE;
		pConfig->baselinePeriod = MA_BASELINE_PERIOD_50;
		pConfig->range = 5;
		pConfig->isEnableMaxLevelRiskControl = FALSE;
		pConfig->skipSunday = TRUE;
		pConfig->useDynamicLevel = TRUE;
		pConfig->useTimeBasedMACD = TRUE;
		
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
		*dailyBaseLine = iMA(3, B_DAILY_RATES, pConfig->baselinePeriod, 1);
	}
	else if (strstr(pParams->tradeSymbol, "SpotCrudeUSD") != NULL || strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
	{
		/* Crude oil configuration (SpotCrudeUSD and XTIUSD)
		 * Trading Logic:
		 * - Fixed level (0.35): Oil has predictable volatility ranges
		 * - Weekly baseline filter: Only trade when price is above weekly midpoint
		 * - Max level buy check: Prevents entry when MACD is too extended upward
		 * - Daily only: One trade per day to avoid overtrading
		 * - EOD entry: Allows late-day entries for oil markets
		 * - Fixed stop loss: Oil trends can be strong, so SL doesn't move back
		 * - Lot sizes: 0.01 minimum for risk management
		 */
		pConfig->level = 0.35;
		pConfig->maxLevel = 0.01 * pParams->bidAsk.ask[0];
		pConfig->histLevel = 0.01;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isEnableMaxLevelBuy = TRUE;
		pConfig->isEnableMaxLevelSell = FALSE;
		pConfig->isEnableMaxLevel = TRUE;
		pConfig->useWeeklyBaseLine = TRUE;
		pConfig->stopLossMultiplier = 1.8;
		pConfig->maxRisk = 1.5;
		pConfig->isDailyOnly = TRUE;
		pConfig->shiftPreDayBar = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
		pConfig->stopMovingBackSL = FALSE;
		pConfig->isEnableEntryEOD = TRUE;
		pConfig->minLotSize = 0.01;
		pConfig->volumeStep = 0.01;
		pConfig->range = 10;
		
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		/* Gold configuration
		 * Trading Logic:
		 * - Fixed level (2.0): Gold has moderate volatility
		 * - Dynamic max level: Scales with price (higher when price > 1500)
		 * - Slow line filter: Ensures trend is established
		 * - CMF volume gap risk: Uses volume gap for risk adjustment
		 * - Max level checks for both buy/sell: Prevents entry at extremes
		 * - No EOD entry: Gold markets close, so no late-day entries
		 * - Moving stop loss: Protects profits in trending gold markets
		 */
		pConfig->level = 2.0;
		pConfig->maxLevel = max(10.0, ((pParams->bidAsk.ask[0] - 1500.0) / 300.0) + 10.0);
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = TRUE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isEnableMaxLevelBuy = TRUE;
		pConfig->isEnableMaxLevelSell = TRUE;
		pConfig->isCMFVolumeGapRisk = TRUE;
		pConfig->maxRisk = 2.0;
		pConfig->isDailyOnly = FALSE;
		pConfig->shiftPreDayBar = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
		pConfig->stopMovingBackSL = TRUE;
		pConfig->isEnableEntryEOD = FALSE;
		pConfig->isEnableLate = FALSE;
		pConfig->baselinePeriod = MA_BASELINE_PERIOD_50;
		pConfig->range = 10;
		pConfig->startHour = pIndicators->startHour;
		
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
		*dailyBaseLine = iMA(3, B_DAILY_RATES, pConfig->baselinePeriod, 1);
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		/* Silver configuration
		 * Trading Logic:
		 * - Fixed level (0.05): Lower than gold due to lower price
		 * - Dynamic max level: Scales with price relative to $15 baseline
		 * - Next day bar filter: Requires positive MACD on next day bar
		 * - ATR range: 1% of price for volatility filter
		 * - No EOD entry: Silver markets close, so no late-day entries
		 * - Moving stop loss: Protects profits in trending silver markets
		 */
		pConfig->level = 0.05;
		pConfig->maxLevel = max(0.2, (0.4 * (pParams->bidAsk.ask[0] - 15.0) / 5.0) + 0.2);
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = FALSE;
		*atrRange = 0.01; /* 1% */
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isCMFVolumeGapRisk = FALSE;
		pConfig->maxRisk = 1.5;
		pConfig->isDailyOnly = FALSE;
		pConfig->shiftPreDayBar = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
		pConfig->stopMovingBackSL = TRUE;
		pConfig->isEnableEntryEOD = FALSE;
		pConfig->isEnableLate = FALSE;
		pConfig->isEnableNextdayBar = TRUE;
		pConfig->baselinePeriod = MA_BASELINE_PERIOD_50;
		pConfig->range = 10;
		
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
		*dailyBaseLine = iMA(3, B_DAILY_RATES, pConfig->baselinePeriod, 1);
	}
	else if (strstr(pParams->tradeSymbol, "XAUEUR") != NULL)
	{
		/* Gold in EUR configuration */
		pConfig->level = 2.0;
		pConfig->maxLevel = 8.0;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isEnableMaxLevelBuy = TRUE;
		pConfig->isEnableMaxLevelSell = TRUE;
		pConfig->shiftPreDayBar = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		/* GBPJPY configuration - major forex pair with volume controls
		 * Trading Logic:
		 * - Volume control enabled: GBPJPY is sensitive to volume changes
		 * - ATR filter: Uses ATR to filter low volatility periods
		 * - CMF volume filter: Requires positive CMF volume for entry
		 * - Next day bar filter: Requires MACD > 0.2 on next day bar
		 * - Max level buy check: Prevents entry when MACD is too extended
		 * - EOD entry: Allows late-day entries for forex markets
		 * - Higher max risk (2.0): GBPJPY can have strong trends
		 */
		pConfig->level = 0.1;
		pConfig->maxLevel = 0.9;
		pConfig->histLevel = 0.01;
		pConfig->isVolumeControl = TRUE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = TRUE;
		pConfig->isEnableCMFVolume = TRUE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isEnableMaxLevelBuy = TRUE;
		pConfig->isEnableMaxLevelSell = FALSE;
		pConfig->isEnableMaxLevel = FALSE;
		pConfig->maxRisk = 2.0;
		pConfig->isDailyOnly = FALSE;
		pConfig->stopMovingBackSL = TRUE;
		pConfig->isEnableEntryEOD = TRUE;
		pConfig->isEnableLate = FALSE;
		pConfig->nextMACDRange = 0.2;
		pConfig->isEnableNextdayBar = TRUE;
		pConfig->range = 10;
		
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
	}
	else if (strstr(pParams->tradeSymbol, "GBPCHF") != NULL)
	{
		/* GBPCHF configuration - minimal filters */
		pConfig->level = 0.002;
		pConfig->maxLevel = 0.008;
		pConfig->histLevel = 0.01;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = FALSE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
	}
	else if (strstr(pParams->tradeSymbol, "AUDJPY") != NULL)
	{
		/* AUDJPY configuration */
		pConfig->level = 0.1;
		pConfig->maxLevel = 0.5;
		pConfig->histLevel = 0.01;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = TRUE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isDailyOnly = FALSE;
	}
	else if (strstr(pParams->tradeSymbol, "EURJPY") != NULL)
	{
		/* EURJPY configuration */
		pConfig->level = 0.1;
		pConfig->maxLevel = 0.9;
		pConfig->histLevel = 0.01;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = TRUE;
		pConfig->isEnableMaxLevelBuy = TRUE;
		pConfig->isEnableMaxLevelSell = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "CADJPY") != NULL)
	{
		/* CADJPY configuration */
		pConfig->level = 0.1;
		pConfig->maxLevel = 0.9;
		pConfig->histLevel = 0.01;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = TRUE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		/* GBPAUD configuration */
		pConfig->level = 0.001;
		pConfig->maxLevel = 0.008;
		pConfig->histLevel = 0.01;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isEnableMaxLevelBuy = TRUE;
		pConfig->isEnableMaxLevelSell = TRUE;
		pConfig->isAllVolumeRisk = FALSE;
		pConfig->maxRisk = 1.5;
		pConfig->isDailyOnly = FALSE;
		pConfig->stopMovingBackSL = TRUE;
		pConfig->baselinePeriod = MA_BASELINE_PERIOD_50;
		pConfig->range = 10;
		
		*dailyBaseLine = iMA(3, B_DAILY_RATES, pConfig->baselinePeriod, 1);
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		/* GBPUSD configuration
		 * Trading Logic:
		 * - Low level threshold (0.001): GBPUSD has lower volatility than GBPJPY
		 * - Max level checks for both buy/sell: Prevents entry at extremes
		 * - Next day bar filter: Requires positive MACD on next day bar
		 * - No EOD entry: Prevents late-day entries
		 * - Moving stop loss: Protects profits in trending markets
		 */
		pConfig->level = 0.001;
		pConfig->maxLevel = 0.007;
		pConfig->histLevel = 0.01;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isEnableMaxLevelBuy = TRUE;
		pConfig->isEnableMaxLevelSell = TRUE;
		pConfig->maxRisk = 1.5;
		pConfig->isDailyOnly = FALSE;
		pConfig->stopMovingBackSL = TRUE;
		pConfig->isEnableEntryEOD = FALSE;
		pConfig->isEnableNextdayBar = TRUE;
		pConfig->range = 10;
	}
	else if (strstr(pParams->tradeSymbol, "AUDNZD") != NULL)
	{
		/* AUDNZD configuration - uses standard MACD parameters */
		pConfig->level = 0.0025;
		pConfig->maxLevel = 0;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = FALSE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->fastMAPeriod = MACD_FAST_PERIOD_STANDARD;
		pConfig->slowMAPeriod = MACD_SLOW_PERIOD_STANDARD;
		pConfig->signalMAPeriod = MACD_SIGNAL_STANDARD;
	}
	else
	{
		/* Default configuration (EURUSD) */
		pConfig->level = 0;
		pConfig->maxLevel = 0.005;
		pConfig->isVolumeControl = FALSE;
		pConfig->isEnableBeiLi = TRUE;
		pConfig->isEnableSlow = TRUE;
		pConfig->isEnableATR = FALSE;
		pConfig->isEnableCMFVolume = FALSE;
		pConfig->isEnableCMFVolumeGap = FALSE;
		pConfig->isDailyOnly = TRUE;
	}
	
	/* Apply lot size and volume step if specified */
	if (pConfig->minLotSize > 0.0)
		pIndicators->minLotSize = pConfig->minLotSize;
	if (pConfig->volumeStep > 0.0)
		pIndicators->volumeStep = pConfig->volumeStep;
}

/**
 * @brief Executes MACD Daily strategy.
 * 
 * This function implements a comprehensive MACD-based daily trading strategy with
 * extensive symbol-specific configurations and advanced features. It provides
 * sophisticated risk management and filtering capabilities.
 * 
 * Key Features:
 * - Supports BTCUSD, ETHUSD, SpotCrudeUSD, XAGUSD, and all major forex pairs
 * - Advanced stop loss management using hourly close prices
 * - ASI (Accumulation Swing Index) indicator support
 * - Late entry prevention (isEnableLate)
 * - Next day bar filtering (isEnableNextdayBar)
 * - Special stop loss handling modes (isEnableNoStopLoss)
 * - Max level risk control (isEnableMaxLevelRiskControl)
 * - Detailed logging and status messages
 * - Flexible baseline calculations (MA20 or MA50 depending on symbol)
 * - Sunday trading restrictions for crypto symbols
 * - Enhanced volume risk controls
 * 
 * The strategy:
 * 1. Configures MACD parameters and filters based on trade symbol
 * 2. Loads MACD indicators, volume data, and ASI indicators
 * 3. Determines entry signals based on MACD values, trend, and volume
 * 4. Applies extensive filters (BeiLi, max level, volume, weekly baseline, ATR, next day bar)
 * 5. Sets stop loss using hourly close prices for better protection
 * 6. Manages exit signals based on MACD crossover reversal
 * 
 * Entry Conditions (Buy):
 * - fast > 0 (MACD fast line above zero)
 * - preDailyClose > dailyBaseLine (price above moving average baseline)
 * - fast - slow > macdLimit (MACD momentum confirmation)
 * - fast > preFast (increasing momentum)
 * - Various volume, ATR, and baseline filters
 * 
 * Entry Conditions (Sell):
 * - fast < 0 (MACD fast line below zero)
 * - preDailyClose < dailyBaseLine (price below moving average baseline)
 * - slow - fast > macdLimit (MACD momentum confirmation)
 * - fast < preFast (decreasing momentum)
 * - Various volume, ATR, and baseline filters
 * 
 * Stop Loss Calculation:
 * - Buy: max(highHourlyClosePrice, entryPrice) - stopLoss
 * - Sell: min(lowHourlyClosePrice, entryPrice) + stopLoss
 * - Uses highest/lowest hourly close prices since order open for better protection
 * 
 * Supported Symbols:
 * - BTCUSD, ETHUSD: Dynamic level calculation, time-based MACD parameters (5/10/5 or 7/14/7)
 * - SpotCrudeUSD: Commodity-specific configuration
 * - XTIUSD: Oil trading configuration
 * - XAUUSD: Gold with dynamic max level
 * - XAGUSD: Silver with special ATR range
 * - XAUEUR: Gold in EUR
 * - GBPJPY: Forex pair with volume controls and MACD zero exit
 * - GBPCHF, AUDJPY, EURJPY, CADJPY: JPY pairs
 * - GBPAUD, GBPUSD: GBP pairs
 * - AUDNZD: Standard MACD parameters (12/26/9)
 * - Default (EURUSD): Standard configuration
 * 
 * @param pParams Strategy parameters containing rates, settings, and account info
 * @param pIndicators Strategy indicators structure to populate with execution signals
 * @param pBase_Indicators Base indicators structure containing trend and ATR data
 * @return SUCCESS on success
 * 
 * @note This is the primary MACD Daily strategy implementation (AUTOBBS_TREND_MODE=23).
 *       Required for BTCUSD, ETHUSD, SpotCrudeUSD, and XAGUSD trading.
 */
AsirikuyReturnCode workoutExecutionTrend_MACD_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift0Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int   dailyTrend; 
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int orderIndex;
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double volume_ma_5;

	double ma20Daily, preDailyClose;
	/* Historical MACD component buffers for trend analysis */
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;
	double dailyBaseLine;

	double preWeeklyClose;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0, weekly_baseline_short = 0.0;
	int pre3KTrend;

	double stopLoss;
	double asiBull, asiBear;
	int truningPointIndex = -1, minPointIndex = -1;
	double turningPoint, minPoint;
	double atrRange;

	double highHourlyClosePrice;
	double lowHourlyClosePrice;

	/* Symbol-specific configuration */
	MACDSymbolConfig config;
	int startShift = 1;
	double macdLimit = 0.0;
	BOOL isMACDBeili = FALSE;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	/* Determine trend direction from daily chart */
	if (pBase_Indicators->dailyTrend_Phase > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend_Phase < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_MACD_DAILY;
	pIndicators->tpMode = TP_MODE_DAILY_ATR;
	pIndicators->tradeMode = TP_MODE_STANDARD;
	pIndicators->stopMovingBackSL = FALSE;

	ma20Daily = iMA(3, B_DAILY_RATES, MA_BASELINE_PERIOD_20, startShift);
	preDailyClose = iClose(B_DAILY_RATES, startShift);
	dailyBaseLine = ma20Daily;

	/* Initialize symbol-specific configuration */
	initializeSymbolConfig(&config, pParams, pIndicators, pBase_Indicators, &timeInfo1, &dailyBaseLine, &atrRange);
	
	/* Extract configuration values for use in main logic */
	double level = config.level;
	double histLevel = config.histLevel;
	double maxLevel = config.maxLevel;
	double nextMACDRange = config.nextMACDRange;
	int fastMAPeriod = config.fastMAPeriod;
	int slowMAPeriod = config.slowMAPeriod;
	int signalMAPeriod = config.signalMAPeriod;
	double maxRisk = config.maxRisk;
	int range = config.range;
	int shiftPreDayBar = config.shiftPreDayBar;
	int startHour = config.startHour;
	
	/* Calculate stop loss based on multiplier */
	stopLoss = pBase_Indicators->pDailyMaxATR;
	if (config.stopLossMultiplier > 1.0)
		stopLoss = max(stopLoss, pBase_Indicators->dailyATR * config.stopLossMultiplier);
	
	/* Apply configuration flags */
	pIndicators->stopMovingBackSL = config.stopMovingBackSL;

	/* After 23:00, check if need to enter (use current bar, relax MACD limit) */
	if (config.isEnableEntryEOD == TRUE && timeInfo1.tm_hour == END_OF_DAY_HOUR)
	{
		startShift = 0;		
		macdLimit = level / MACD_LIMIT_DIVISOR;

		/* For oil symbols, allow trading on same day at EOD */
		if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL || strstr(pParams->tradeSymbol, "SpotCrudeUSD") != NULL)
		{
			/* Override isDailyOnly for EOD entry */
		}
	}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);

	iASIEasy(B_DAILY_RATES, 0, 10, 5, &asiBull, &asiBear);

	/* If previous day close price not exceed threshold daily ATR, wait for next day */
	if (timeInfo1.tm_hour >= startHour
		&& (config.isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		) /* Start hour check, avoid the first hour */
	{		

		preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
		iTrend3Rules_preDays(pParams, pBase_Indicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
		weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

		pIndicators->cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, startShift);

		pIndicators->CMFVolumeGap = getCMFVolumeGap(B_DAILY_RATES, 1, fastMAPeriod, startShift);

		/* Volume indicator: current volume vs previous volume */
		pIndicators->volume1 = iVolume(B_DAILY_RATES, startShift);
		pIndicators->volume2 = iVolume(B_DAILY_RATES, startShift + 1);
		volume_ma_5 = iMA(4, B_DAILY_RATES, 5, startShift);

		logInfo("System InstanceID = %d, BarTime = %s, cmfVolume =%lf, CMFVolumeGap=%lf, weekly_baseline=%lf, weekly_baseline_short=%lf,volume1=%lf,volume2=%lf,volume_ma_5=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->cmfVolume, pIndicators->CMFVolumeGap, weekly_baseline, weekly_baseline_short, pIndicators->volume1, pIndicators->volume2, volume_ma_5);

		/* Load MACD indicators for current and historical bars */
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 2, &fast3, &slow3, &preHist3);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 3, &fast4, &slow4, &preHist4);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 4, &fast5, &slow5, &preHist5);
				
		pIndicators->fast = fast1;
		pIndicators->slow = slow1;
		pIndicators->preFast = fast2;
		pIndicators->preSlow = slow2;
		

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		pIndicators->stopLoss = stopLoss;

		/* Find the highest/lowest close price after order is opened for stop loss calculation */
		getHighestHourlyClosePrice(pParams, pIndicators, pBase_Indicators, B_HOURLY_RATES, orderIndex, &highHourlyClosePrice, &lowHourlyClosePrice);

		if (config.isEnableMaxLevelRiskControl == TRUE && pIndicators->fast > maxLevel)
			pIndicators->risk = 0.5;

		logInfo("System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow);
		logInfo("System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift + 1), pIndicators->preFast, pIndicators->preSlow);

		logInfo("System InstanceID = %d, BarTime = %s, highHourlyClosePrice =%lf, lowHourlyClosePrice=%lf, stopLoss=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, highHourlyClosePrice, lowHourlyClosePrice, stopLoss);

		if (pIndicators->fast > 0						
			&& preDailyClose > dailyBaseLine
			&& pIndicators->fast - pIndicators->slow > macdLimit
			) /* Buy signal */
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			
			pIndicators->stopLossPrice = max(highHourlyClosePrice,pIndicators->entryPrice) - stopLoss;

			if (config.isEnableNoStopLoss == TRUE && orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{
				/* Not moving stop loss - special handling for adding positions */
				pIndicators->executionTrend = 0;				
				{


					if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice > pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 2 * pIndicators->stopLoss
						)
					{
						pIndicators->executionTrend = 1;
						pIndicators->entryPrice = pParams->bidAsk.ask[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					}
					else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 2 * pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 3 * pIndicators->stopLoss)
					{
						pIndicators->executionTrend = 1;
						pIndicators->entryPrice = pParams->bidAsk.ask[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + pIndicators->stopLoss;
					}
				}
								
			}
			
			logInfo("System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			
			if (
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& pIndicators->fast > pIndicators->preFast		
				)
			{
				
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				logInfo("System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if ( timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{
					pIndicators->entrySignal = 1;

					if ((config.isVolumeControlRisk == TRUE && pIndicators->volume1 > pIndicators->volume2 )
						|| (config.isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap > 0)
						|| (config.isCMFVolumeRisk == TRUE && pIndicators->cmfVolume > 0)
						|| (config.isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap > 0 && pIndicators->cmfVolume >  0 && pIndicators->volume1 > pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;					

					if (config.isEnableSlow == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->slow <= 0)
					{
						sprintf(pIndicators->status, "slow %lf is not greater than level 0.",
							pIndicators->slow);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (pIndicators->entrySignal != 0 &&
						pIndicators->fast <= level)
					{
						sprintf(pIndicators->status, "fast %lf is not greater than level %lf.",
							pIndicators->fast,level);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					/* If price retreats back after 5 days, prevent late entry */
					if (config.isEnableLate == TRUE && preHist1 > histLevel && preHist2 > histLevel && preHist3 > histLevel && preHist4 > histLevel && preHist5 > histLevel
						&& fast1 > level && fast2 > level && fast3 > level && fast4 > level && fast5 > level
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						)
					{
						strcpy(pIndicators->status, "it is late for 5 days\n");

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,pIndicators->status);

						pIndicators->entrySignal = 0;
					}

					if (config.isEnableCMFVolumeGap == TRUE && 
						pIndicators->entrySignal != 0 &&
						pIndicators->CMFVolumeGap <= 0)
					{
						sprintf(pIndicators->status,"CMFVolumeGap %lf is not greater than 0",
							pIndicators->CMFVolumeGap);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (config.isEnableCMFVolume == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->cmfVolume <= 0)
					{
						sprintf(pIndicators->status,"cmfVolume %lf is not greater than 0",
							pIndicators->cmfVolume);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (config.isVolumeControl == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->volume1 <= pIndicators->volume2)
					{
						sprintf(pIndicators->status,"volume1 %lf is not greater than volume2 %lf",
							pIndicators->volume1, pIndicators->volume2);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (config.useWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						preWeeklyClose <= weekly_baseline
						)
					{
						sprintf(pIndicators->status, "preWeeklyClose %lf is not greater than weekly baseline %lf.",
							preWeeklyClose, weekly_baseline);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (config.useWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						weekly_baseline_short <= weekly_baseline && pre3KTrend != UP
						)
					{
						sprintf(pIndicators->status, "Weekly_baseline_short %lf is less than weekly_baseline %lf and pre3KTrend %d is not UP.",
							weekly_baseline_short, weekly_baseline, pre3KTrend);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					
					isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

					if (config.isEnableMaxLevelBuy == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& pIndicators->fast > maxLevel
						&& (config.isEnableMaxLevel == FALSE || minPoint >= level)
						)
					{	

						sprintf(pIndicators->status, "MACD %lf exceeds max level %lf",
							pIndicators->fast, maxLevel);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					
					}

					if (config.isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0 
						&& isMACDBeili == TRUE
						&& (minPoint >= level || truningPointIndex - 1 <= range)
						)
					{
						strcpy(pIndicators->status, "MACD BeiLi");								

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
						
					}

				}

				
			}

			pIndicators->exitSignal = EXIT_SELL;

		}

		if (pIndicators->fast < 0			
			&& preDailyClose < dailyBaseLine
			&& pIndicators->slow - pIndicators->fast > macdLimit
			) /* Sell signal */
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];	
						
			pIndicators->stopLossPrice = min(lowHourlyClosePrice, pIndicators->entryPrice) + stopLoss;

			if (config.isEnableNoStopLoss == TRUE && orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{
				/* Not moving stop loss - special handling for adding positions */
				pIndicators->executionTrend = 0;
				
				{
					if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss
						)
					{
						pIndicators->executionTrend = -1;
						pIndicators->entryPrice = pParams->bidAsk.bid[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					}
					else if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss)
					{
						pIndicators->executionTrend = -1;
						pIndicators->entryPrice = pParams->bidAsk.bid[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - pIndicators->stopLoss;
					}
				}
			}

			logInfo("System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			if (
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& pIndicators->fast < pIndicators->preFast			
				)
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				logWarning("System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{

					pIndicators->entrySignal = -1;

					if ((config.isVolumeControlRisk == TRUE && pIndicators->volume1 > pIndicators->volume2)
						|| (config.isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap < 0)
						|| (config.isCMFVolumeRisk == TRUE && pIndicators->cmfVolume < 0)
						|| (config.isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap < 0 && pIndicators->cmfVolume <  0 && pIndicators->volume1 > pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;

					if (config.isEnableSlow == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->slow >= 0)
					{
						sprintf(pIndicators->status, "slow %lf is not less than level 0.",
							pIndicators->slow);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (pIndicators->entrySignal != 0 &&
						pIndicators->fast >= -1 * level)
					{
						sprintf(pIndicators->status, "fast %lf is not less than level %lf.",
							pIndicators->fast, -1* level);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					/* If price retreats back after 5 days, prevent late entry */
					if (config.isEnableLate == TRUE && preHist1 < (histLevel*-1) && preHist2 < (histLevel*-1) && preHist3 < (histLevel*-1) && preHist4 < (histLevel*-1) && preHist5 < (histLevel*-1)
						&& fast1 < (level*-1) && fast2 < (level*-1) && fast3 < (level*-1) && fast4 < (level*-1) && fast5 < (level*-1)
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						)
					{
						strcpy(pIndicators->status, "it is late for 5 days\n");

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

					if (config.isEnableCMFVolumeGap == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->CMFVolumeGap > 0)
					{
						sprintf(pIndicators->status,"CMFVolumeGap %lf is not less than 0",
							pIndicators->CMFVolumeGap);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (config.isEnableCMFVolume == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->cmfVolume > 0)
					{
						sprintf(pIndicators->status,"cmfVolume %lf is not less than 0",
							pIndicators->cmfVolume);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (config.isVolumeControl == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->volume1 <= pIndicators->volume2)
					{
						sprintf(pIndicators->status,"volume1 %lf is not greater than volume2 %lf",
							pIndicators->volume1, pIndicators->volume2);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (config.useWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						preWeeklyClose >= weekly_baseline
						)
					{
						sprintf(pIndicators->status, "preWeeklyClose %lf is not less than weekly baseline %lf.",
							preWeeklyClose, weekly_baseline);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (config.useWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						weekly_baseline_short >= weekly_baseline && pre3KTrend != DOWN
						)
					{
						sprintf(pIndicators->status, "Weekly_baseline_short %lf is greater than weekly_baseline %lf and pre3KTrend %d is not DOWN",
							weekly_baseline_short, weekly_baseline, pre3KTrend);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

					if (config.isEnableMaxLevelSell == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& pIndicators->fast < (maxLevel*-1)
						&& (config.isEnableMaxLevel == FALSE || minPoint <= -1 * level)
						)
					{

						sprintf(pIndicators->status,"MACD %lf exceeds max level %lf",
							pIndicators->fast,maxLevel*-1);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

					if (config.isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& isMACDBeili == TRUE
						&& (minPoint <= -1 * level || truningPointIndex - 1 <= range)
						)
					{
						strcpy(pIndicators->status, "MACD BeiLi");								

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
						
					}

				}
			}

			pIndicators->exitSignal = EXIT_BUY;

		}

		if (config.isEnableATR == TRUE &&
			pIndicators->entrySignal != 0 &&
			atr5 <= pIndicators->entryPrice * atrRange)
		{
			sprintf(pIndicators->status, "atr5 %lf is not greater than %lf.",
				atr5, pIndicators->entryPrice * atrRange);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		if (
			pIndicators->entrySignal != 0					
			&& config.isEnableNextdayBar == TRUE && isNextdayMACDPostiveBar(startShift) == FALSE
			&& ( nextMACDRange == 0 || fabs(pIndicators->fast) < nextMACDRange)
			)
		{

			sprintf(pIndicators->status,"Nextday MACD Bar %lf is negative value %lf.",
				fabs(pIndicators->fast), nextMACDRange);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
		}

		if (pIndicators->entrySignal > 0  &&
			pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift) > 0.2 * pBase_Indicators->dailyATR)
		{
			sprintf(pIndicators->status, "Open price gap %lf is not less than %lf",
				pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift), 0.2 * pBase_Indicators->dailyATR);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		if (pIndicators->entrySignal < 0 &&
			iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice > 0.2 * pBase_Indicators->dailyATR)
		{
			sprintf(pIndicators->status, "Open price gap %lf is not less than %lf",
				iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice, 0.2 * pBase_Indicators->dailyATR);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		if (pIndicators->entrySignal != 0 && (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL) && DAY_OF_WEEK(currentTime) == SUNDAY)
		{
			sprintf(pIndicators->status, "System InstanceID = %d, BarTime = %s, skip to entry a trade on Sunday.\n",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		/* Exit signal from daily chart: MACD crossover reversal */
		if (pIndicators->fast - pIndicators->slow > macdLimit && pIndicators->preFast <= pIndicators->preSlow)
			pIndicators->exitSignal = EXIT_SELL;

		if (pIndicators->slow - pIndicators->fast > macdLimit && pIndicators->preFast >= pIndicators->preSlow)
			pIndicators->exitSignal = EXIT_BUY;

	}

	return SUCCESS;
}
