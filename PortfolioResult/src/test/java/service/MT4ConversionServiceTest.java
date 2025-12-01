package service;

import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.*;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import model.Rates;

/**
 * Unit tests for {@link MT4ConversionServiceImpl}.
 * Tests MT4 to NTS conversion and rate merging operations.
 */
@ExtendWith(MockitoExtension.class)
@DisplayName("MT4ConversionService Tests")
class MT4ConversionServiceTest {

	@Mock
	private FileService fileService;

	private MT4ConversionServiceImpl mt4ConversionService;

	@BeforeEach
	void setUp() {
		mt4ConversionService = new MT4ConversionServiceImpl(fileService);
	}

	@Test
	@DisplayName("Should convert MT4 to NTS format with new rates")
	void testConvertMT4ToNTS_WithNewRates() throws IOException {
		List<Rates> mt4Rates = createTestRates(5);
		List<Rates> existingNtsRates = createTestRates(3);
		
		// Set dates so MT4 rates are newer
		for (int i = 0; i < mt4Rates.size(); i++) {
			mt4Rates.get(i).datetime = new Date(System.currentTimeMillis() + (i * 1000));
		}
		for (int i = 0; i < existingNtsRates.size(); i++) {
			existingNtsRates.get(i).datetime = new Date(System.currentTimeMillis() - (10000 + i * 1000));
		}
		
		when(fileService.readMt4RateCSV("mt4.csv")).thenReturn(mt4Rates);
		when(fileService.readNtsRateCSV("nts.csv")).thenReturn(existingNtsRates);
		
		mt4ConversionService.convertMT4ToNTS("mt4.csv", "nts.csv");
		
		verify(fileService).readMt4RateCSV("mt4.csv");
		verify(fileService).readNtsRateCSV("nts.csv");
		verify(fileService).writeNtsRateCSV(eq("nts.csv"), anyList());
	}

	@Test
	@DisplayName("Should convert MT4 to NTS format when NTS file is empty")
	void testConvertMT4ToNTS_EmptyNTSFile() throws IOException {
		List<Rates> mt4Rates = createTestRates(5);
		List<Rates> emptyNtsRates = new ArrayList<>();
		
		when(fileService.readMt4RateCSV("mt4.csv")).thenReturn(mt4Rates);
		when(fileService.readNtsRateCSV("nts.csv")).thenReturn(emptyNtsRates);
		
		mt4ConversionService.convertMT4ToNTS("mt4.csv", "nts.csv");
		
		verify(fileService).writeNtsRateCSV(eq("nts.csv"), anyList());
	}

	@Test
	@DisplayName("Should not add duplicate rates when converting")
	void testConvertMT4ToNTS_NoNewRates() throws IOException {
		Date baseDate = new Date(System.currentTimeMillis() - 10000);
		List<Rates> mt4Rates = createTestRates(3);
		List<Rates> ntsRates = createTestRates(5);
		
		// Set all dates to be older than NTS rates
		for (Rates rate : mt4Rates) {
			rate.datetime = new Date(baseDate.getTime() - 10000);
		}
		for (int i = 0; i < ntsRates.size(); i++) {
			ntsRates.get(i).datetime = new Date(baseDate.getTime() + (i * 1000));
		}
		
		when(fileService.readMt4RateCSV("mt4.csv")).thenReturn(mt4Rates);
		when(fileService.readNtsRateCSV("nts.csv")).thenReturn(ntsRates);
		
		mt4ConversionService.convertMT4ToNTS("mt4.csv", "nts.csv");
		
		verify(fileService).writeNtsRateCSV(eq("nts.csv"), eq(ntsRates));
	}

	@Test
	@DisplayName("Should merge MT4 rates successfully when validation passes")
	void testMergeMT4Rates_Success() throws IOException {
		List<Rates> currentRates = createTestRates(100);
		List<Rates> newRates = createTestRates(200); // Enough rates to pass validation
		
		// Set dates so new rates are after current rates
		// Last current rate: 1 day ago
		// First new rate: now
		// This gives us 1 day = 1440 minutes / 60 = 24 bars expected
		// With 1% discount, we need at least 0.24 bars, so 200 bars will pass
		Date lastCurrentDate = new Date(System.currentTimeMillis() - 86400000L); // 1 day ago
		Date firstNewDate = new Date(System.currentTimeMillis()); // Now
		
		for (int i = 0; i < currentRates.size(); i++) {
			currentRates.get(i).datetime = new Date(lastCurrentDate.getTime() - (currentRates.size() - i) * 60000);
		}
		for (int i = 0; i < newRates.size(); i++) {
			newRates.get(i).datetime = new Date(firstNewDate.getTime() + i * 60000);
		}
		
		when(fileService.readMt4RateCSV("current.csv")).thenReturn(currentRates);
		when(fileService.readMt4RateCSV("new.csv")).thenReturn(newRates);
		
		mt4ConversionService.mergeMT4Rates("current.csv", "new.csv", "error.csv", 60);
		
		verify(fileService).readMt4RateCSV("current.csv");
		verify(fileService).readMt4RateCSV("new.csv");
		verify(fileService).writeMT4RateCSV(eq("current.csv"), anyList());
		verify(fileService, never()).writeToErrorFile(anyString(), anyString());
	}

	@Test
	@DisplayName("Should write error file when validation fails")
	void testMergeMT4Rates_ValidationFails() throws IOException {
		List<Rates> currentRates = createTestRates(100);
		List<Rates> newRates = createTestRates(1); // Too few rates
		
		// 7 days = 7 * 1440 = 10080 minutes
		// Expected bars = 10080 / 60 = 168 bars
		// Min bars = 168 * 0.01 = 1.68 bars
		// We have only 1 bar, so validation should fail
		Date lastCurrentDate = new Date(System.currentTimeMillis() - 7 * 86400000L); // 7 days ago
		Date firstNewDate = new Date(System.currentTimeMillis()); // Today
		
		for (int i = 0; i < currentRates.size(); i++) {
			currentRates.get(i).datetime = new Date(lastCurrentDate.getTime() - (currentRates.size() - i) * 60000);
		}
		newRates.get(0).datetime = firstNewDate;
		
		when(fileService.readMt4RateCSV("current.csv")).thenReturn(currentRates);
		when(fileService.readMt4RateCSV("new.csv")).thenReturn(newRates);
		
		mt4ConversionService.mergeMT4Rates("current.csv", "new.csv", "error.csv", 60);
		
		verify(fileService).writeToErrorFile(eq("error.csv"), anyString());
		verify(fileService, never()).writeMT4RateCSV(anyString(), anyList());
	}


	@Test
	@DisplayName("Should handle empty current rates during merge")
	void testMergeMT4Rates_EmptyCurrentRates() throws IOException {
		List<Rates> emptyRates = new ArrayList<>();
		List<Rates> newRates = createTestRates(10);
		
		when(fileService.readMt4RateCSV("current.csv")).thenReturn(emptyRates);
		when(fileService.readMt4RateCSV("new.csv")).thenReturn(newRates);
		
		mt4ConversionService.mergeMT4Rates("current.csv", "new.csv", "error.csv", 60);
		
		// When current rates are empty, the method returns early without writing
		// This is expected behavior - verify it doesn't crash
		verify(fileService).readMt4RateCSV("current.csv");
		verify(fileService).readMt4RateCSV("new.csv");
		verify(fileService, never()).writeMT4RateCSV(anyString(), anyList());
	}

	// Helper methods

	private List<Rates> createTestRates(int count) {
		List<Rates> rates = new ArrayList<>();
		for (int i = 0; i < count; i++) {
			// Using @AllArgsConstructor: (datetime, open, close, high, low, volume)
			// Parameter order matches field declaration order in Rates class
			Rates rate = new Rates(
				new Date(System.currentTimeMillis() + (i * 60000)),  // datetime
				1.2000 + (i * 0.0001),  // open
				1.2005 + (i * 0.0001),  // close
				1.2010 + (i * 0.0001),  // high
				1.1990 + (i * 0.0001),  // low
				1000 + i                 // volume
			);
			rates.add(rate);
		}
		return rates;
	}
}

