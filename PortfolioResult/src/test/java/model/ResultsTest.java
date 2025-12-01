package model;

import static org.junit.jupiter.api.Assertions.*;

import java.util.Date;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

/**
 * Unit tests for {@link Results} model class.
 * Tests trade result data structure, comparison, and equality.
 */
@DisplayName("Results Model Tests")
class ResultsTest {

	private Results result1;
	private Results result2;
	private Date testDate1;
	private Date testDate2;

	@BeforeEach
	void setUp() {
		testDate1 = new Date(1609459200000L); // 2021-01-01 00:00:00
		testDate2 = new Date(1609545600000L); // 2021-01-02 00:00:00
		
		result1 = new Results();
		result1.strategyID = "200003";
		result1.orderNumber = "12345";
		result1.orderType = "BUY";
		result1.openTime = testDate1;
		result1.closeTime = testDate2;
		result1.openPrice = 1.2000;
		result1.closePrice = 1.2050;
		result1.lots = 1.0;
		result1.profit = 50.0;
		result1.pair = "EURUSD";
		result1.balance = 100050.0;
		
		result2 = new Results();
		result2.strategyID = "200009";
		result2.orderNumber = "12346";
		result2.orderType = "SELL";
		result2.openTime = testDate1;
		result2.closeTime = testDate2;
		result2.openPrice = 1.2000;
		result2.closePrice = 1.1950;
		result2.lots = 1.0;
		result2.profit = -50.0;
		result2.pair = "EURUSD";
		result2.balance = 99950.0;
	}

	@Test
	@DisplayName("Should create Results with no-args constructor")
	void testNoArgsConstructor() {
		Results result = new Results();
		assertNotNull(result);
		assertNull(result.strategyID);
		assertNull(result.orderNumber);
		assertNull(result.orderType);
	}

	@Test
	@DisplayName("Should copy Results using copy constructor")
	void testCopyConstructor() {
		Results copy = new Results(result1);
		
		assertEquals(result1.strategyID, copy.strategyID);
		assertEquals(result1.orderNumber, copy.orderNumber);
		assertEquals(result1.orderType, copy.orderType);
		assertEquals(result1.openTime, copy.openTime);
		assertEquals(result1.closeTime, copy.closeTime);
		assertEquals(result1.openPrice, copy.openPrice);
		assertEquals(result1.closePrice, copy.closePrice);
		assertEquals(result1.lots, copy.lots);
		assertEquals(result1.profit, copy.profit);
		assertEquals(result1.pair, copy.pair);
		assertEquals(result1.balance, copy.balance);
	}

	@Test
	@DisplayName("Should compare Results by close time")
	void testCompareToByCloseTime() {
		Results earlier = new Results();
		earlier.closeTime = testDate1;
		earlier.strategyID = "200003";
		earlier.orderNumber = "1";
		earlier.pair = "EURUSD";
		
		Results later = new Results();
		later.closeTime = testDate2;
		later.strategyID = "200003";
		later.orderNumber = "2";
		later.pair = "EURUSD";
		
		assertTrue(earlier.compareTo(later) < 0);
		assertTrue(later.compareTo(earlier) > 0);
		assertEquals(0, earlier.compareTo(earlier));
	}

	@Test
	@DisplayName("Should compare Results by strategy ID when close time is equal")
	void testCompareToByStrategyID() {
		Results resultA = new Results();
		resultA.closeTime = testDate1;
		resultA.strategyID = "200003";
		resultA.orderNumber = "1";
		resultA.pair = "EURUSD";
		
		Results resultB = new Results();
		resultB.closeTime = testDate1;
		resultB.strategyID = "200009";
		resultB.orderNumber = "2";
		resultB.pair = "EURUSD";
		
		assertTrue(resultA.compareTo(resultB) < 0);
		assertTrue(resultB.compareTo(resultA) > 0);
	}

	@Test
	@DisplayName("Should compare Results by order number when close time and strategy ID are equal")
	void testCompareToByOrderNumber() {
		Results resultA = new Results();
		resultA.closeTime = testDate1;
		resultA.strategyID = "200003";
		resultA.orderNumber = "12345";
		resultA.pair = "EURUSD";
		
		Results resultB = new Results();
		resultB.closeTime = testDate1;
		resultB.strategyID = "200003";
		resultB.orderNumber = "12346";
		resultB.pair = "EURUSD";
		
		assertTrue(resultA.compareTo(resultB) < 0);
		assertTrue(resultB.compareTo(resultA) > 0);
	}

	@Test
	@DisplayName("Should test equality based on strategyID, orderNumber, orderType, closeTime, and pair")
	void testEquals() {
		Results resultA = new Results();
		resultA.strategyID = "200003";
		resultA.orderNumber = "12345";
		resultA.orderType = "BUY";
		resultA.closeTime = testDate1;
		resultA.pair = "EURUSD";
		
		Results resultB = new Results();
		resultB.strategyID = "200003";
		resultB.orderNumber = "12345";
		resultB.orderType = "BUY";
		resultB.closeTime = testDate1;
		resultB.pair = "EURUSD";
		
		Results resultC = new Results();
		resultC.strategyID = "200009";
		resultC.orderNumber = "12345";
		resultC.orderType = "BUY";
		resultC.closeTime = testDate1;
		resultC.pair = "EURUSD";
		
		assertEquals(resultA, resultB);
		assertNotEquals(resultA, resultC);
	}

	@Test
	@DisplayName("Should generate consistent hashCode for equal objects")
	void testHashCode() {
		Results resultA = new Results();
		resultA.strategyID = "200003";
		resultA.orderNumber = "12345";
		resultA.orderType = "BUY";
		resultA.closeTime = testDate1;
		resultA.pair = "EURUSD";
		
		Results resultB = new Results();
		resultB.strategyID = "200003";
		resultB.orderNumber = "12345";
		resultB.orderType = "BUY";
		resultB.closeTime = testDate1;
		resultB.pair = "EURUSD";
		
		assertEquals(resultA.hashCode(), resultB.hashCode());
	}

	@Test
	@DisplayName("Should generate toString with key trade details")
	void testToString() {
		String toString = result1.toString();
		assertNotNull(toString);
		assertTrue(toString.contains("StrategyID=" + result1.strategyID));
		assertTrue(toString.contains("Symbol=" + result1.pair));
		assertTrue(toString.contains("OrderType=" + result1.orderType));
		assertTrue(toString.contains("OpenPrice="));
		assertTrue(toString.contains("ClosePrice="));
		assertTrue(toString.contains("Lots="));
	}

	@Test
	@DisplayName("Should handle null values in comparison")
	void testCompareToWithNullValues() {
		Results resultA = new Results();
		resultA.closeTime = testDate1;
		resultA.strategyID = "200003";
		resultA.orderNumber = "12345";
		resultA.pair = "EURUSD";
		
		Results resultB = new Results();
		resultB.closeTime = testDate1;
		resultB.strategyID = "200003";
		resultB.orderNumber = "12345";
		resultB.pair = "EURUSD";
		
		// Both have same values, should be equal
		assertEquals(0, resultA.compareTo(resultB));
	}
}

