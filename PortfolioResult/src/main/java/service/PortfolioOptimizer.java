package service;

import java.util.List;
import java.util.Map;

/**
 * Interface for portfolio optimization strategies.
 * 
 * <p>Provides different algorithms to find optimal risk allocations for trading strategies,
 * from exhaustive grid search to heuristic approaches suitable for large portfolios.</p>
 * 
 * @version 1.0
 * @since 2025
 */
public interface PortfolioOptimizer {
	
	/**
	 * Optimization strategy types.
	 */
	enum OptimizationStrategy {
		/** Exhaustive grid search - best for 1-6 strategies */
		GRID_SEARCH,
		
		/** Random sampling - fast approximation */
		RANDOM_SEARCH,
		
		/** Genetic algorithm - good for 7-15 strategies */
		GENETIC_ALGORITHM,
		
		/** Simulated annealing - alternative heuristic */
		SIMULATED_ANNEALING,
		
		/** Coarse-to-fine - hierarchical approach */
		COARSE_TO_FINE
	}
	
	/**
	 * Optimizes portfolio risk allocations using the specified strategy.
	 * 
	 * @param strategies List of strategy IDs to optimize
	 * @param riskMultipliers Available risk multipliers to test
	 * @param predefinedStrategies Strategies with fixed allocations
	 * @param strategy Optimization algorithm to use
	 * @param maxEvaluations Maximum portfolio configurations to evaluate (budget limit)
	 * @param randomSeed Optional random seed for deterministic results (null for non-deterministic)
	 * @return Best risk allocation found (strategy ID -> risk multiplier)
	 */
	Map<String, Double> optimize(
		List<String> strategies,
		List<Double> riskMultipliers,
		Map<String, Double> predefinedStrategies,
		OptimizationStrategy strategy,
		int maxEvaluations,
		Long randomSeed
	);
	
	/**
	 * Evaluates the fitness (quality) of a risk allocation.
	 * Returns the Martin Ratio (CAGR / Ulcer Index) as the fitness metric.
	 * 
	 * @param riskAllocation Strategy risk allocations to evaluate
	 * @return Fitness score (higher is better)
	 */
	double evaluateFitness(Map<String, Double> riskAllocation);
	
	/**
	 * Legacy grid search method for backward compatibility.
	 * Performs exhaustive N-dimensional grid search.
	 * 
	 * <p><strong>Warning:</strong> Only practical for small portfolios (≤6 strategies).</p>
	 * 
	 * @param strategies List of strategy IDs to optimize
	 * @param riskMultipliers List of risk multipliers to test
	 * @param predefinedStrategies Strategies with fixed risk allocations
	 */
	void optimizePortfolio(List<String> strategies, List<Double> riskMultipliers,
	                       Map<String,Double> predefinedStrategies);
}
