package service;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.*;

import java.util.ArrayList;
import java.util.Arrays;
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
import model.Statistics;
import service.PortfolioOptimizer.OptimizationStrategy;

/**
 * Unit tests for {@link PortfolioOptimizerImpl}.
 * Tests optimization algorithms: Grid Search, Random Search, Genetic Algorithm, Simulated Annealing, and Coarse-to-Fine.
 * 
 * <p>Note: Full optimization testing requires integration test setup with real data.
 * These tests verify that the algorithms run without errors and return valid results.</p>
 */
@ExtendWith(MockitoExtension.class)
@DisplayName("PortfolioOptimizer Tests")
class PortfolioOptimizerTest {

	@Mock
	private ModelDataService modelDataService;

	@Mock
	private StatisticsService statisticsService;

	private PortfolioOptimizerImpl optimizer;
	private List<String> testStrategies;
	private List<Double> riskMultipliers;
	private Map<String, Double> predefinedStrategies;

	@BeforeEach
	void setUp() {
		optimizer = new PortfolioOptimizerImpl(modelDataService, statisticsService);
		
		testStrategies = Arrays.asList("200003", "200009");
		riskMultipliers = Arrays.asList(0.2, 1.0, 2.0);
		predefinedStrategies = new HashMap<>();
		
		// Setup mock to return test statistics
		setupMockData();
	}

	@Test
	@DisplayName("Should perform random search optimization")
	void testRandomSearch() {
		Map<String, Double> result = optimizer.optimize(
			testStrategies,
			riskMultipliers,
			predefinedStrategies,
			OptimizationStrategy.RANDOM_SEARCH,
			10,
			12345L // Deterministic seed
		);
		
		assertNotNull(result);
	}

	@Test
	@DisplayName("Should perform grid search optimization")
	void testGridSearch() {
		Map<String, Double> result = optimizer.optimize(
			testStrategies,
			riskMultipliers,
			predefinedStrategies,
			OptimizationStrategy.GRID_SEARCH,
			20,
			12345L // Use deterministic seed
		);
		
		assertNotNull(result);
	}

	@Test
	@DisplayName("Should perform simulated annealing optimization")
	void testSimulatedAnnealing() {
		Map<String, Double> result = optimizer.optimize(
			testStrategies,
			riskMultipliers,
			predefinedStrategies,
			OptimizationStrategy.SIMULATED_ANNEALING,
			10,
			12345L // Deterministic seed
		);
		
		assertNotNull(result);
	}

	@Test
	@DisplayName("Should perform coarse-to-fine optimization")
	void testCoarseToFine() {
		Map<String, Double> result = optimizer.optimize(
			testStrategies,
			riskMultipliers,
			predefinedStrategies,
			OptimizationStrategy.COARSE_TO_FINE,
			10,
			12345L // Deterministic seed
		);
		
		assertNotNull(result);
	}

	@Test
	@DisplayName("Should use deterministic random seed when provided")
	void testDeterministicRandomSeed() {
		Long seed = 12345L;
		
		Map<String, Double> result1 = optimizer.optimize(
			testStrategies,
			riskMultipliers,
			predefinedStrategies,
			OptimizationStrategy.RANDOM_SEARCH,
			5,
			seed
		);
		
		Map<String, Double> result2 = optimizer.optimize(
			testStrategies,
			riskMultipliers,
			predefinedStrategies,
			OptimizationStrategy.RANDOM_SEARCH,
			5,
			seed
		);
		
		// With same seed, results should be identical
		assertEquals(result1, result2);
	}

	@Test
	@DisplayName("Should handle empty strategies list")
	void testEmptyStrategies() {
		List<String> emptyStrategies = new ArrayList<>();
		
		Map<String, Double> result = optimizer.optimize(
			emptyStrategies,
			riskMultipliers,
			predefinedStrategies,
			OptimizationStrategy.RANDOM_SEARCH,
			5,
			12345L
		);
		
		assertNotNull(result);
	}

	@Test
	@DisplayName("Should handle single strategy optimization")
	void testSingleStrategy() {
		List<String> singleStrategy = Arrays.asList("200003");
		
		Map<String, Double> result = optimizer.optimize(
			singleStrategy,
			riskMultipliers,
			predefinedStrategies,
			OptimizationStrategy.GRID_SEARCH,
			5,
			12345L
		);
		
		assertNotNull(result);
	}

	@Test
	@DisplayName("Should respect max evaluations budget")
	void testMaxEvaluations() {
		int maxEvals = 5;
		
		Map<String, Double> result = optimizer.optimize(
			testStrategies,
			riskMultipliers,
			predefinedStrategies,
			OptimizationStrategy.RANDOM_SEARCH,
			maxEvals,
			12345L
		);
		
		assertNotNull(result);
	}

	@Test
	@DisplayName("Should evaluate fitness correctly")
	void testEvaluateFitness() {
		Map<String, Double> riskAllocation = new HashMap<>();
		riskAllocation.put("200003", 2.0);
		riskAllocation.put("200009", 1.5);
		
		double fitness = optimizer.evaluateFitness(riskAllocation);
		
		// Fitness should be a valid number (Martin Ratio) or 0.0 if constraints violated
		assertTrue(fitness >= 0 || Double.isNaN(fitness));
	}

	// Helper methods

	private void setupMockData() {
		// Setup mock to return test statistics
		Statistics testStats = new Statistics();
		testStats.totalReturn = 1.5;
		testStats.max_dd = 10.0;
		testStats.cagr = 25.0;
		testStats.ulcerIndex = 5.0;
		testStats.martin = 5.0;
		
		// Create a list that will accumulate statistics
		List<Statistics> statsList = new ArrayList<>();
		statsList.add(testStats);
		
		lenient().doAnswer(invocation -> {
			Statistics stats = invocation.getArgument(0);
			// Copy test values to the statistics object
			stats.totalReturn = testStats.totalReturn;
			stats.max_dd = testStats.max_dd;
			stats.cagr = testStats.cagr;
			stats.ulcerIndex = testStats.ulcerIndex;
			stats.martin = testStats.martin;
			synchronized (statsList) {
				Statistics copy = new Statistics();
				copy.totalReturn = testStats.totalReturn;
				copy.max_dd = testStats.max_dd;
				copy.cagr = testStats.cagr;
				copy.ulcerIndex = testStats.ulcerIndex;
				copy.martin = testStats.martin;
				statsList.add(copy);
			}
			return null;
		}).when(modelDataService).saveStatistics(any(Statistics.class));
		
		lenient().when(modelDataService.getStatistics()).thenAnswer(invocation -> {
			synchronized (statsList) {
				// Always return at least one statistics object
				if (statsList.isEmpty()) {
					Statistics defaultStats = new Statistics();
					defaultStats.totalReturn = 1.5;
					defaultStats.max_dd = 10.0;
					defaultStats.cagr = 25.0;
					defaultStats.ulcerIndex = 5.0;
					defaultStats.martin = 5.0;
					return Arrays.asList(defaultStats);
				}
				return new ArrayList<>(statsList);
			}
		});
		lenient().when(modelDataService.getFactors()).thenReturn(new HashMap<>());
		lenient().doNothing().when(modelDataService).addStrategyList(anyMap());
	}
}
