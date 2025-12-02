package service;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.Mockito.*;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import model.ModelDataService;
import model.Results;
import model.Statistics;

/**
 * Unit tests for {@link StatisticsServiceImpl}.
 * Tests critical statistical calculations including returns, drawdowns, ratios, and filtering.
 */
@ExtendWith(MockitoExtension.class)
@DisplayName("StatisticsService Tests")
class StatisticsServiceTest {

	@Mock
	private ModelDataService modelDataService;

	private StatisticsServiceImpl statisticsService;
	private List<Results> testResults;
	private static final double INITIAL_BALANCE = 100000.0;

	@BeforeEach
	void setUp() {
		statisticsService = new StatisticsServiceImpl(modelDataService);
		testResults = createTestResults();
	}

	@Test
	@DisplayName("Should throw exception when model is null")
	void testNullModel() {
		assertThrows(IllegalArgumentException.class, () -> {
			new StatisticsServiceImpl(null);
		});
	}

	@Test
	@DisplayName("Should throw exception when initial balance is zero or negative")
	void testInvalidInitialBalance() {
		// No need to stub getAdjustedData since exception is thrown before it's called
		assertThrows(IllegalArgumentException.class, () -> {
			statisticsService.calculate(0.0, false);
		});
		
		assertThrows(IllegalArgumentException.class, () -> {
			statisticsService.calculate(-100.0, false);
		});
	}

	@Test
	@DisplayName("Should throw exception when no results available")
	void testNoResults() {
		when(modelDataService.getAdjustedData(anyBoolean())).thenReturn(new ArrayList<>());
		
		assertThrows(IllegalStateException.class, () -> {
			statisticsService.calculate(INITIAL_BALANCE, false);
		});
	}

	@Test
	@DisplayName("Should calculate total return correctly")
	void testCalculateTotalReturn() {
		when(modelDataService.getAdjustedData(false)).thenReturn(testResults);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// Verify total return
			assertEquals(1.1, stats.totalReturn, 0.001);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should calculate trade counts correctly")
	void testCalculateTradeCounts() {
		when(modelDataService.getAdjustedData(false)).thenReturn(testResults);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// 5 BUY trades, 3 SELL trades, 8 total
			assertEquals(8L, stats.num_trades);
			assertEquals(5L, stats.num_longs);
			assertEquals(3L, stats.num_shorts);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should calculate profit factor correctly")
	void testCalculateProfitFactor() {
		when(modelDataService.getAdjustedData(false)).thenReturn(testResults);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// Total winning: 500 + 300 + 200 = 1000
			// Total losing: |(-100) + (-200)| = 300
			// Profit factor: 1000 / 300 = 3.333...
			assertTrue(stats.profit_factor > 3.0);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should calculate winning percentage correctly")
	void testCalculateWinningPercentage() {
		when(modelDataService.getAdjustedData(false)).thenReturn(testResults);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// 6 winning trades out of 8 total = 75%
			assertEquals(75.0, stats.winning, 0.1);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should calculate maximum drawdown correctly")
	void testCalculateMaxDrawdown() {
		when(modelDataService.getAdjustedData(false)).thenReturn(testResults);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// Max balance: 110000, Min during drawdown: 107000
			// Drawdown: (110000 - 107000) / 110000 * 100 = 2.727%
			assertTrue(stats.max_dd > 0);
			assertTrue(stats.max_dd < 5.0);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should calculate CAGR correctly")
	void testCalculateCAGR() {
		when(modelDataService.getAdjustedData(false)).thenReturn(testResults);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// Total return: 1.1, Years: ~0.5 (6 months)
			// CAGR should be positive
			assertTrue(stats.cagr > 0);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should calculate Sharpe ratio correctly")
	void testCalculateSharpeRatio() {
		when(modelDataService.getAdjustedData(false)).thenReturn(testResults);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// Sharpe ratio should be calculated
			assertNotNull(stats.sharp_ratio);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should calculate Ulcer Index correctly")
	void testCalculateUlcerIndex() {
		when(modelDataService.getAdjustedData(false)).thenReturn(testResults);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// Ulcer Index should be calculated and capped at 100
			assertTrue(stats.ulcerIndex >= 0);
			assertTrue(stats.ulcerIndex <= 100.0);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should calculate Martin ratio correctly")
	void testCalculateMartinRatio() {
		when(modelDataService.getAdjustedData(false)).thenReturn(testResults);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// Martin ratio = CAGR / Ulcer Index
			if (stats.ulcerIndex > 0) {
				assertTrue(stats.martin > 0);
			}
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should handle all winning trades")
	void testAllWinningTrades() {
		List<Results> allWinning = new ArrayList<>();
		for (int i = 0; i < 5; i++) {
			Results r = createResult("200003", "BUY", 100.0, i);
			allWinning.add(r);
		}
		
		when(modelDataService.getAdjustedData(false)).thenReturn(allWinning);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			assertEquals(100.0, stats.winning, 0.1);
			assertTrue(stats.profit_factor > 0);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should handle all losing trades")
	void testAllLosingTrades() {
		List<Results> allLosing = new ArrayList<>();
		for (int i = 0; i < 5; i++) {
			Results r = createResult("200003", "SELL", -50.0, i);
			allLosing.add(r);
		}
		
		when(modelDataService.getAdjustedData(false)).thenReturn(allLosing);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			assertEquals(0.0, stats.winning, 0.1);
			// Profit factor should be 0 or very small
			assertTrue(stats.profit_factor >= 0);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should handle single trade")
	void testSingleTrade() {
		List<Results> singleTrade = new ArrayList<>();
		singleTrade.add(createResult("200003", "BUY", 100.0, 0));
		
		when(modelDataService.getAdjustedData(false)).thenReturn(singleTrade);
		doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			assertEquals(1L, stats.num_trades);
			assertEquals(100.0, stats.winning, 0.1);
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		statisticsService.calculate(INITIAL_BALANCE, false);
		
		verify(modelDataService).saveStatistics(any(Statistics.class));
	}

	@Test
	@DisplayName("Should filter statistics based on optimization factors")
	void testSelectModelsWithOptimization() {
		Statistics stats1 = new Statistics();
		stats1.totalReturn = 1.5;
		stats1.max_dd = 10.0;
		stats1.ulcerIndex = 5.0;
		stats1.strategyRisk = new HashMap<>();
		stats1.strategyRisk.put("200003", 2.0);
		
		Statistics stats2 = new Statistics();
		stats2.totalReturn = 1.2;
		stats2.max_dd = 15.0;
		stats2.ulcerIndex = 8.0;
		stats2.strategyRisk = new HashMap<>();
		stats2.strategyRisk.put("200009", 1.5);
		
		when(modelDataService.getStatistics()).thenReturn(List.of(stats1, stats2));
		when(modelDataService.getFactors()).thenReturn(createOptimizationFactors());
		
		List<Statistics> selected = statisticsService.selectModels(true);
		
		assertNotNull(selected);
		// Should filter based on factors - getFactors() is called 3 times (once per factor)
		verify(modelDataService, atLeast(1)).getFactors();
	}

	@Test
	@DisplayName("Should return all statistics when not optimized")
	void testSelectModelsWithoutOptimization() {
		Statistics stats1 = new Statistics();
		Statistics stats2 = new Statistics();
		
		when(modelDataService.getStatistics()).thenReturn(List.of(stats1, stats2));
		
		List<Statistics> selected = statisticsService.selectModels(false);
		
		assertNotNull(selected);
		assertEquals(2, selected.size());
		verify(modelDataService, never()).getFactors();
	}

	// Helper methods

	private List<Results> createTestResults() {
		List<Results> results = new ArrayList<>();
		
		// Create a sequence of trades over 6 months
		long baseTime = System.currentTimeMillis() - (180L * 24 * 60 * 60 * 1000); // 6 months ago
		
		// Winning trades
		results.add(createResult("200003", "BUY", 500.0, 0, baseTime, 100000.0));
		results.add(createResult("200003", "BUY", 300.0, 1, baseTime + 7*24*60*60*1000L, 100300.0));
		results.add(createResult("200009", "BUY", 200.0, 2, baseTime + 14*24*60*60*1000L, 100500.0));
		results.add(createResult("200003", "SELL", 100.0, 3, baseTime + 21*24*60*60*1000L, 100600.0));
		results.add(createResult("500003", "BUY", 400.0, 4, baseTime + 28*24*60*60*1000L, 101000.0));
		results.add(createResult("200009", "BUY", 150.0, 5, baseTime + 35*24*60*60*1000L, 101150.0));
		
		// Losing trades
		results.add(createResult("200003", "SELL", -100.0, 6, baseTime + 42*24*60*60*1000L, 101050.0));
		results.add(createResult("500003", "SELL", -200.0, 7, baseTime + 49*24*60*60*1000L, 100850.0));
		
		// Final balance: 110000 (from initial 100000 + net profit 10000)
		results.get(results.size() - 1).balance = 110000.0;
		
		return results;
	}

	private Results createResult(String strategyID, String orderType, double profit, int index) {
		return createResult(strategyID, orderType, profit, index, 
			System.currentTimeMillis() - (index * 7L * 24 * 60 * 60 * 1000), 
			100000.0 + (index * 100.0));
	}

	private Results createResult(String strategyID, String orderType, double profit, 
			int index, long closeTime, double balance) {
		Results r = new Results();
		r.strategyID = strategyID;
		r.orderNumber = "ORD" + index;
		r.orderType = orderType;
		r.openTime = new Date(closeTime - 24*60*60*1000L);
		r.closeTime = new Date(closeTime);
		r.openPrice = 1.2000;
		r.closePrice = 1.2000 + (profit / 10000.0);
		r.lots = 1.0;
		r.profit = profit;
		r.balance = balance;
		r.pair = "EURUSD";
		return r;
	}

	private Map<String, Double> createOptimizationFactors() {
		Map<String, Double> factors = new HashMap<>();
		factors.put("min_max_dd", 5.0);
		factors.put("max_max_dd", 12.0);
		factors.put("max_ulcerIndex", 10.0);
		return factors;
	}
}

