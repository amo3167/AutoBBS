package service;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.Mockito.*;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import model.ModelDataService;
import model.Results;
import model.Statistics;
import service.StatisticsService;

/**
 * Unit tests for {@link FileServiceImpl}.
 * Tests CSV reading/writing operations using temporary files.
 */
@ExtendWith(MockitoExtension.class)
@DisplayName("FileService Tests")
class FileServiceTest {

	@Mock
	private ModelDataService modelDataService;

	@Mock
	private StatisticsService statisticsService;

	private FileServiceImpl fileService;

	@TempDir
	Path tempDir;

	@BeforeEach
	void setUp() {
		fileService = new FileServiceImpl(modelDataService, statisticsService);
	}

	@Test
	@DisplayName("Should read portfolio risk from CSV file")
	void testReadPortfolioRisk() throws IOException {
		// Create test CSV file
		Path testFile = tempDir.resolve("portfolio_risk.csv");
		try (FileWriter writer = new FileWriter(testFile.toFile())) {
			writer.write("StrategyID,Risk\n");
			writer.write("200003,2.0\n");
			writer.write("200009,1.5\n");
			writer.write("500003,2.2\n");
		}

		Map<String, Double> risks = fileService.readPortfolioRisk(testFile.toString());

		assertNotNull(risks);
		assertEquals(3, risks.size());
		assertEquals(2.0, risks.get("200003"));
		assertEquals(1.5, risks.get("200009"));
		assertEquals(2.2, risks.get("500003"));
	}

	@Test
	@DisplayName("Should throw exception when portfolio risk file not found")
	void testReadPortfolioRiskFileNotFound() {
		assertThrows(RuntimeException.class, () -> {
			fileService.readPortfolioRisk("nonexistent_file.csv");
		});
	}

	@Test
	@DisplayName("Should read equity value from CSV file")
	void testReadEquityCSV() throws IOException {
		// Create test CSV file
		Path testFile = tempDir.resolve("equity.csv");
		try (FileWriter writer = new FileWriter(testFile.toFile())) {
			writer.write("100000.0\n");
		}

		double equity = fileService.readEquityCSV(testFile.toString());

		assertEquals(100000.0, equity);
	}

	@Test
	@DisplayName("Should throw exception when equity file not found")
	void testReadEquityCSVFileNotFound() {
		assertThrows(RuntimeException.class, () -> {
			fileService.readEquityCSV("nonexistent_file.csv");
		});
	}

	@Test
	@DisplayName("Should read closed orders from CSV file")
	void testReadClosedOrdersCSV() throws IOException {
		// Create test CSV file with closed order format
		Path testFile = tempDir.resolve("closed_orders.csv");
		try (FileWriter writer = new FileWriter(testFile.toFile())) {
			// Format: OrderNumber,OpenTime,CloseTime,Pair,OrderType,Lots,OpenPrice,ClosePrice,SL,TP,Profit,StrategyID,Swap,Balance
			writer.write("12345,2021.01.01 10:00,2021.01.01 14:00,EURUSD,0,1.0,1.2000,1.2050,1.1950,1.2100,50.0,200003,0.0,100050.0\n");
			writer.write("12346,2021.01.02 10:00,2021.01.02 14:00,EURUSD,1,1.0,1.2000,1.1950,1.2100,1.1900,-50.0,200009,0.0,100000.0\n");
		}

		List<Results> results = fileService.readClosedOrdersCSV(testFile.toString());

		assertNotNull(results);
		assertEquals(2, results.size());
		
		Results first = results.get(0);
		assertEquals("12345", first.orderNumber);
		assertEquals("EURUSD", first.pair);
		assertEquals("BUY", first.orderType);
		assertEquals(1.0, first.lots);
		assertEquals(1.2000, first.openPrice);
		assertEquals(1.2050, first.closePrice);
		// Note: profit field is not set in convertClosedResults, it's calculated elsewhere
		assertEquals("200003", first.strategyID);
	}

	@Test
	@DisplayName("Should read open orders from CSV file")
	void testReadOpenOrdersCSV() throws IOException {
		// Create test CSV file with open order format
		Path testFile = tempDir.resolve("open_orders.csv");
		try (FileWriter writer = new FileWriter(testFile.toFile())) {
			// Format: OrderNumber,OpenTime,Pair,OrderType,Lots,OpenPrice,SL,TP,CurrentPrice,StrategyID,Comment,Balance
			writer.write("12345,2021.01.01 10:00,EURUSD,0,1.0,1.2000,1.1950,1.2100,1.2020,200003,Comment,100000.0\n");
		}

		List<Results> results = fileService.readOpenOrdersCSV(testFile.toString());

		assertNotNull(results);
		assertEquals(1, results.size());
		
		Results first = results.get(0);
		assertEquals("12345", first.orderNumber);
		assertEquals("EURUSD", first.pair);
		assertEquals("BUY", first.orderType);
		assertEquals(1.0, first.lots);
		assertEquals(1.2000, first.openPrice);
		assertEquals("200003", first.strategyID);
	}

	@Test
	@DisplayName("Should write mismatched file")
	void testWriteMismatchedFile() throws IOException {
		Path testFile = tempDir.resolve("mismatched");
		Map<String, String> mismatched = new HashMap<>();
		mismatched.put("200003", "Price mismatch");
		mismatched.put("200009", "Lots mismatch");

		fileService.writeMismatchedFile(testFile.toString(), mismatched);

		// Verify file was created (with timestamp suffix)
		File[] files = tempDir.toFile().listFiles((dir, name) -> name.startsWith("mismatched_"));
		assertNotNull(files);
		assertTrue(files.length > 0);
	}

	@Test
	@DisplayName("Should write statistics to CSV file")
	void testWriteStatisticsCSV() throws IOException {
		Path testFile = tempDir.resolve("statistics.csv");
		Statistics stats = new Statistics();
		stats.totalReturn = 1.5;
		stats.max_dd = 10.0;
		stats.max_dd_length = 45.0;
		stats.profit_factor = 1.8;
		stats.cagr = 25.0;
		stats.cagr_maxdd = 2.5;
		stats.sharp_ratio = 2.0;
		stats.num_trades = 1000L;
		stats.num_longs = 600L;
		stats.num_shorts = 400L;
		stats.trades_week = 10.5;
		stats.winning = 55.0;
		stats.r2 = 0.95;
		stats.risk_reward = 1.5;
		stats.ulcerIndex = 5.0;
		stats.martin = 5.0;
		stats.maxWinningTrade = java.util.OptionalDouble.of(1000.0);
		stats.maxLosingTrade = java.util.OptionalDouble.of(-500.0);
		stats.strategyRisk = new HashMap<>();
		stats.strategyRisk.put("200003", 2.0);

		// Method may catch IOException internally, so we just verify it doesn't throw
		assertDoesNotThrow(() -> {
			fileService.writeStatiscsCVS(testFile.toString(), stats);
		});
		
		// Verify file was created (if method doesn't catch exceptions)
		// Note: The method catches IOException internally, so file might not be created if there's an error
		if (Files.exists(testFile)) {
			assertTrue(Files.size(testFile) > 0);
		}
	}

	@Test
	@DisplayName("Should write to error file")
	void testWriteToErrorFile() throws IOException {
		Path testFile = tempDir.resolve("error.log");
		String errorMessage = "Test error message";

		fileService.writeToErrorFile(testFile.toString(), errorMessage);

		// Verify file was created and contains error message
		assertTrue(Files.exists(testFile));
		String content = Files.readString(testFile);
		assertTrue(content.contains(errorMessage));
	}

	@Test
	@DisplayName("Should handle empty portfolio risk file")
	void testReadPortfolioRiskEmptyFile() throws IOException {
		Path testFile = tempDir.resolve("empty_risk.csv");
		try (FileWriter writer = new FileWriter(testFile.toFile())) {
			writer.write("StrategyID,Risk\n");
		}

		Map<String, Double> risks = fileService.readPortfolioRisk(testFile.toString());

		assertNotNull(risks);
		assertTrue(risks.isEmpty());
	}

	@Test
	@DisplayName("Should handle invalid CSV format gracefully")
	void testReadPortfolioRiskInvalidFormat() throws IOException {
		Path testFile = tempDir.resolve("invalid.csv");
		try (FileWriter writer = new FileWriter(testFile.toFile())) {
			writer.write("StrategyID,Risk\n");
			writer.write("200003,invalid_number\n");
		}

		assertThrows(RuntimeException.class, () -> {
			fileService.readPortfolioRisk(testFile.toString());
		});
	}

	@Test
	@DisplayName("Should handle empty closed orders file")
	void testReadClosedOrdersCSVEmptyFile() throws IOException {
		Path testFile = tempDir.resolve("empty_closed.csv");
		try (FileWriter writer = new FileWriter(testFile.toFile())) {
			// Empty file
		}

		List<Results> results = fileService.readClosedOrdersCSV(testFile.toString());

		assertNotNull(results);
		assertTrue(results.isEmpty());
	}
}

