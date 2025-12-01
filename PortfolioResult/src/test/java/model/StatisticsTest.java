package model;

import static org.junit.jupiter.api.Assertions.*;

import java.util.HashMap;
import java.util.Map;
import java.util.OptionalDouble;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

/**
 * Unit tests for {@link Statistics} model class.
 * Tests statistical metrics data structure and initialization.
 */
@DisplayName("Statistics Model Tests")
class StatisticsTest {

	private Statistics statistics;

	@BeforeEach
	void setUp() {
		statistics = new Statistics();
	}

	@Test
	@DisplayName("Should create Statistics with no-args constructor")
	void testNoArgsConstructor() {
		assertNotNull(statistics);
		assertEquals(0.0, statistics.totalReturn);
		assertEquals(0.0, statistics.max_dd);
		assertEquals(0.0, statistics.profit_factor);
		assertNotNull(statistics.strategyRisk);
	}

	@Test
	@DisplayName("Should set and get total return")
	void testTotalReturn() {
		statistics.totalReturn = 1.5;
		assertEquals(1.5, statistics.totalReturn);
	}

	@Test
	@DisplayName("Should set and get maximum drawdown")
	void testMaxDrawdown() {
		statistics.max_dd = 15.5;
		assertEquals(15.5, statistics.max_dd);
	}

	@Test
	@DisplayName("Should set and get profit factor")
	void testProfitFactor() {
		statistics.profit_factor = 1.8;
		assertEquals(1.8, statistics.profit_factor);
	}

	@Test
	@DisplayName("Should set and get CAGR")
	void testCAGR() {
		statistics.cagr = 25.5;
		assertEquals(25.5, statistics.cagr);
	}

	@Test
	@DisplayName("Should set and get Sharpe ratio")
	void testSharpeRatio() {
		statistics.sharp_ratio = 2.1;
		assertEquals(2.1, statistics.sharp_ratio);
	}

	@Test
	@DisplayName("Should set and get trade counts")
	void testTradeCounts() {
		statistics.num_trades = 1000L;
		statistics.num_longs = 600L;
		statistics.num_shorts = 400L;
		
		assertEquals(1000L, statistics.num_trades);
		assertEquals(600L, statistics.num_longs);
		assertEquals(400L, statistics.num_shorts);
	}

	@Test
	@DisplayName("Should set and get winning percentage")
	void testWinningPercentage() {
		statistics.winning = 55.5;
		assertEquals(55.5, statistics.winning);
	}

	@Test
	@DisplayName("Should set and get R-squared")
	void testRSquared() {
		statistics.r2 = 0.95;
		assertEquals(0.95, statistics.r2);
	}

	@Test
	@DisplayName("Should set and get Ulcer Index")
	void testUlcerIndex() {
		statistics.ulcerIndex = 5.2;
		assertEquals(5.2, statistics.ulcerIndex);
	}

	@Test
	@DisplayName("Should set and get Martin ratio")
	void testMartinRatio() {
		statistics.martin = 4.5;
		assertEquals(4.5, statistics.martin);
	}

	@Test
	@DisplayName("Should set and get max winning trade")
	void testMaxWinningTrade() {
		statistics.maxWinningTrade = OptionalDouble.of(1000.0);
		assertTrue(statistics.maxWinningTrade.isPresent());
		assertEquals(1000.0, statistics.maxWinningTrade.getAsDouble());
	}

	@Test
	@DisplayName("Should set and get max losing trade")
	void testMaxLosingTrade() {
		statistics.maxLosingTrade = OptionalDouble.of(-500.0);
		assertTrue(statistics.maxLosingTrade.isPresent());
		assertEquals(-500.0, statistics.maxLosingTrade.getAsDouble());
	}

	@Test
	@DisplayName("Should set and get strategy risk allocations")
	void testStrategyRisk() {
		Map<String, Double> riskMap = new HashMap<>();
		riskMap.put("200003", 2.0);
		riskMap.put("200009", 1.5);
		riskMap.put("500003", 2.2);
		
		statistics.strategyRisk = riskMap;
		
		assertEquals(3, statistics.strategyRisk.size());
		assertEquals(2.0, statistics.strategyRisk.get("200003"));
		assertEquals(1.5, statistics.strategyRisk.get("200009"));
		assertEquals(2.2, statistics.strategyRisk.get("500003"));
	}

	@Test
	@DisplayName("Should handle empty strategy risk map")
	void testEmptyStrategyRisk() {
		assertNotNull(statistics.strategyRisk);
		assertTrue(statistics.strategyRisk.isEmpty());
	}

	@Test
	@DisplayName("Should set and get CAGR/MaxDD ratio")
	void testCAGRMaxDD() {
		statistics.cagr = 20.0;
		statistics.max_dd = 10.0;
		statistics.cagr_maxdd = statistics.cagr / statistics.max_dd;
		
		assertEquals(2.0, statistics.cagr_maxdd);
	}

	@Test
	@DisplayName("Should set and get trades per week")
	void testTradesPerWeek() {
		statistics.trades_week = 10.5;
		assertEquals(10.5, statistics.trades_week);
	}

	@Test
	@DisplayName("Should set and get risk-reward ratio")
	void testRiskReward() {
		statistics.risk_reward = 1.5;
		assertEquals(1.5, statistics.risk_reward);
	}

	@Test
	@DisplayName("Should set and get years traded")
	void testYearsTraded() {
		statistics.yearsTraded = 3.5;
		assertEquals(3.5, statistics.yearsTraded);
	}

	@Test
	@DisplayName("Should set and get max drawdown length")
	void testMaxDrawdownLength() {
		statistics.max_dd_length = 45.5;
		assertEquals(45.5, statistics.max_dd_length);
	}

	@Test
	@DisplayName("Should handle complete statistics object")
	void testCompleteStatistics() {
		statistics.totalReturn = 1.5;
		statistics.max_dd = 12.5;
		statistics.profit_factor = 1.8;
		statistics.cagr = 25.0;
		statistics.sharp_ratio = 2.0;
		statistics.num_trades = 1000L;
		statistics.winning = 55.0;
		statistics.r2 = 0.95;
		statistics.ulcerIndex = 5.0;
		statistics.martin = 5.0;
		statistics.maxWinningTrade = OptionalDouble.of(1000.0);
		statistics.maxLosingTrade = OptionalDouble.of(-500.0);
		
		Map<String, Double> riskMap = new HashMap<>();
		riskMap.put("200003", 2.0);
		statistics.strategyRisk = riskMap;
		
		assertEquals(1.5, statistics.totalReturn);
		assertEquals(12.5, statistics.max_dd);
		assertEquals(1.8, statistics.profit_factor);
		assertEquals(25.0, statistics.cagr);
		assertEquals(2.0, statistics.sharp_ratio);
		assertEquals(1000L, statistics.num_trades);
		assertEquals(55.0, statistics.winning);
		assertEquals(0.95, statistics.r2);
		assertEquals(5.0, statistics.ulcerIndex);
		assertEquals(5.0, statistics.martin);
		assertTrue(statistics.maxWinningTrade.isPresent());
		assertTrue(statistics.maxLosingTrade.isPresent());
		assertEquals(1, statistics.strategyRisk.size());
	}
}

