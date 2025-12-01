package service;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.*;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import com.opencsv.exceptions.CsvValidationException;

import model.ModelDataService;
import model.Rates;

/**
 * Unit tests for {@link RateAdjustmentServiceImpl}.
 * Tests timezone adjustment operations for various currency pairs and commodities.
 */
@ExtendWith(MockitoExtension.class)
@DisplayName("RateAdjustmentService Tests")
class RateAdjustmentServiceTest {

	@Mock
	private ModelDataService modelDataService;

	@Mock
	private FileService fileService;

	private RateAdjustmentServiceImpl rateAdjustmentService;

	@BeforeEach
	void setUp() {
		rateAdjustmentService = new RateAdjustmentServiceImpl(modelDataService, fileService);
	}

	@Test
	@DisplayName("Should adjust EUR/USD 5M rates successfully")
	void testAdjustEURUSD_5M() throws IOException, CsvValidationException {
		List<Rates> adjustedRates = new ArrayList<>();
		when(modelDataService.adjustTimeZone()).thenReturn(adjustedRates);
		
		rateAdjustmentService.adjustEURUSD_5M("/test/dir/");
		
		verify(fileService, times(4)).readHistoricalRateCSV(anyString());
		verify(modelDataService).adjustTimeZone();
		verify(fileService).writeNtsRateCSV(endsWith("EURUSD_5.csv"), eq(adjustedRates));
	}

	@Test
	@DisplayName("Should adjust EUR/GBP 5M rates successfully")
	void testAdjustEURGBP_5M() throws IOException, CsvValidationException {
		List<Rates> adjustedRates = new ArrayList<>();
		when(modelDataService.adjustTimeZone()).thenReturn(adjustedRates);
		
		rateAdjustmentService.adjustEURGBP_5M("/test/dir/");
		
		verify(fileService, times(5)).readHistoricalRateCSV(anyString());
		verify(modelDataService).adjustTimeZone();
		verify(fileService).writeNtsRateCSV(endsWith("EURGBP_5.csv"), eq(adjustedRates));
	}

	@Test
	@DisplayName("Should adjust XAG/USD 60M rates successfully")
	void testAdjustXAGUSD_60M() throws IOException, CsvValidationException {
		List<Rates> adjustedRates = new ArrayList<>();
		when(modelDataService.adjustTimeZone_Commodity()).thenReturn(adjustedRates);
		
		rateAdjustmentService.adjustXAGUSD_60M("/test/dir/");
		
		verify(fileService, atLeastOnce()).readHistoricalRateCSV(anyString());
		verify(modelDataService).adjustTimeZone_Commodity();
		verify(fileService).writeNtsRateCSV(endsWith("XAGUSD_60.csv"), eq(adjustedRates));
	}

	@Test
	@DisplayName("Should adjust USTEC/USD 60M rates successfully")
	void testAdjustUSTECUSD_60M() throws IOException, CsvValidationException {
		List<Rates> adjustedRates = new ArrayList<>();
		when(modelDataService.adjustTimeZone_Commodity()).thenReturn(adjustedRates);
		
		rateAdjustmentService.adjustUSTECUSD_60M("/test/dir/");
		
		verify(fileService, atLeastOnce()).readHistoricalRateCSV(anyString());
		verify(modelDataService).adjustTimeZone_Commodity();
		verify(fileService).writeNtsRateCSV(endsWith("USTECUSD_60.csv"), eq(adjustedRates));
	}

	@Test
	@DisplayName("Should throw RuntimeException when file reading fails")
	void testAdjustEURUSD_5M_FileReadError() throws IOException, CsvValidationException {
		doThrow(new IOException("File not found")).when(fileService)
			.readHistoricalRateCSV(anyString());
		
		assertThrows(RuntimeException.class, () -> {
			rateAdjustmentService.adjustEURUSD_5M("/test/dir/");
		});
	}

	@Test
	@DisplayName("Should throw RuntimeException when CSV validation fails")
	void testAdjustEURUSD_5M_CSVValidationError() throws IOException, CsvValidationException {
		doThrow(new CsvValidationException("Invalid CSV")).when(fileService)
			.readHistoricalRateCSV(anyString());
		
		assertThrows(RuntimeException.class, () -> {
			rateAdjustmentService.adjustEURUSD_5M("/test/dir/");
		});
	}

	@Test
	@DisplayName("Should throw RuntimeException when file writing fails")
	void testAdjustEURUSD_5M_FileWriteError() throws IOException, CsvValidationException {
		List<Rates> adjustedRates = new ArrayList<>();
		when(modelDataService.adjustTimeZone()).thenReturn(adjustedRates);
		doThrow(new IOException("Write failed")).when(fileService)
			.writeNtsRateCSV(anyString(), anyList());
		
		assertThrows(RuntimeException.class, () -> {
			rateAdjustmentService.adjustEURUSD_5M("/test/dir/");
		});
	}

	@Test
	@DisplayName("Should handle empty adjusted rates list")
	void testAdjustEURUSD_5M_EmptyRates() throws IOException, CsvValidationException {
		List<Rates> emptyRates = new ArrayList<>();
		when(modelDataService.adjustTimeZone()).thenReturn(emptyRates);
		
		rateAdjustmentService.adjustEURUSD_5M("/test/dir/");
		
		verify(fileService).writeNtsRateCSV(anyString(), eq(emptyRates));
	}
}

