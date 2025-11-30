package service;

import java.util.*;
import java.util.concurrent.ThreadLocalRandom;
import java.util.stream.Collectors;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import model.ModelDataService;
import model.ModelDataServiceImpl;

/**
 * Implementation of smart portfolio optimization strategies.
 * 
 * <p>Provides multiple algorithms suitable for different portfolio sizes:</p>
 * <ul>
 *   <li><strong>Grid Search</strong>: Exhaustive search, best for 1-6 strategies</li>
 *   <li><strong>Random Search</strong>: Fast sampling, good baseline for any size</li>
 *   <li><strong>Genetic Algorithm</strong>: Population-based, efficient for 7-15 strategies</li>
 *   <li><strong>Simulated Annealing</strong>: Gradient-free optimization</li>
 *   <li><strong>Coarse-to-Fine</strong>: Hierarchical search with refinement</li>
 * </ul>
 * 
 * @version 1.0
 * @since 2025
 */
public class PortfolioOptimizerImpl implements PortfolioOptimizer {
	private static final Logger logger = LogManager.getLogger(PortfolioOptimizerImpl.class);
	
	private final ModelDataService model;
	private final StatisticsService statisticsService;
	
	// Genetic algorithm parameters
	private static final int GA_POPULATION_SIZE = 50;
	private static final int GA_GENERATIONS = 100;
	private static final double GA_MUTATION_RATE = 0.15;
	private static final double GA_CROSSOVER_RATE = 0.7;
	private static final int GA_TOURNAMENT_SIZE = 5;
	
	// Simulated annealing parameters
	private static final double SA_INITIAL_TEMP = 100.0;
	private static final double SA_COOLING_RATE = 0.95;
	private static final int SA_ITERATIONS_PER_TEMP = 20;
	
	public PortfolioOptimizerImpl(ModelDataService model, StatisticsService statisticsService) {
		this.model = model;
		this.statisticsService = statisticsService;
	}
	
	@Override
	public Map<String, Double> optimize(
			List<String> strategies,
			List<Double> riskMultipliers,
			Map<String, Double> predefinedStrategies,
			OptimizationStrategy strategy,
			int maxEvaluations) {
		
		logger.info("Starting {} optimization for {} strategies with budget of {} evaluations",
				strategy, strategies.size(), maxEvaluations);
		
		switch (strategy) {
			case GRID_SEARCH:
				return gridSearch(strategies, riskMultipliers, predefinedStrategies, maxEvaluations);
			case RANDOM_SEARCH:
				return randomSearch(strategies, riskMultipliers, predefinedStrategies, maxEvaluations);
			case GENETIC_ALGORITHM:
				return geneticAlgorithm(strategies, riskMultipliers, predefinedStrategies, maxEvaluations);
			case SIMULATED_ANNEALING:
				return simulatedAnnealing(strategies, riskMultipliers, predefinedStrategies, maxEvaluations);
			case COARSE_TO_FINE:
				return coarseToFine(strategies, riskMultipliers, predefinedStrategies, maxEvaluations);
			default:
				logger.warn("Unknown strategy {}, falling back to RANDOM_SEARCH", strategy);
				return randomSearch(strategies, riskMultipliers, predefinedStrategies, maxEvaluations);
		}
	}
	
	/**
	 * Performs N-dimensional grid search with progress logging.
	 * 
	 * <p>This is the legacy brute-force optimizer that tests all combinations
	 * of risk multipliers. It's exposed as a public method for backward compatibility
	 * with existing code that calls it directly.</p>
	 * 
	 * <p><strong>Warning:</strong> Only practical for small portfolios (≤6 strategies).</p>
	 * 
	 * @param strategies List of strategy IDs to optimize
	 * @param riskMultipliers List of risk multipliers to test
	 * @param predefinedStrategies Strategies with fixed risk allocations
	 */
	public void optimizePortfolio(List<String> strategies, List<Double> riskMultipliers,
	                              Map<String,Double> predefinedStrategies) {
		if (strategies.isEmpty()) {
			logger.warn("No strategies to optimize");
			return;
		}
		
		int totalCombinations = (int) Math.pow(riskMultipliers.size(), strategies.size());
		
		// Warn if grid search is impractical
		if (strategies.size() > 7) {
			logger.warn("Grid search with {} strategies requires {} combinations. " +
					"Consider using PortfolioOptimizerImpl with GENETIC_ALGORITHM instead.",
					strategies.size(), totalCombinations);
		}
		
		logger.info("Starting grid search: {} strategies × {} risk levels = {} combinations",
				strategies.size(), riskMultipliers.size(), totalCombinations);
		
		int[] iterationCounter = {0};
		final int LOG_INTERVAL = Math.max(1000, totalCombinations / 10);
		Map<String,Double> workingRisks = new HashMap<>();
		
		// Start recursive grid search
		exploreRiskCombinations(strategies, riskMultipliers, predefinedStrategies,
				workingRisks, 0, iterationCounter, LOG_INTERVAL);
		
		logger.info("Grid search complete. Tested {} portfolio configurations.", iterationCounter[0]);
	}
	
	/**
	 * Recursively explores all risk multiplier combinations.
	 * 
	 * <p>Helper method for optimizePortfolio implementing N-dimensional grid search.</p>
	 */
	private void exploreRiskCombinations(List<String> strategies, List<Double> riskMultipliers,
	                                     Map<String,Double> predefinedStrategies,
	                                     Map<String,Double> workingRisks,
	                                     int depth, int[] iterationCounter, int logInterval) {
		// Base case: all strategies have been assigned a risk multiplier
		if (depth == strategies.size()) {
		// Add predefined strategies with fixed risk allocations
		workingRisks.putAll(predefinedStrategies);
		
		// Calculate statistics for this portfolio configuration
		model.addStrategyList(workingRisks);
		statisticsService.calculate(ModelDataServiceImpl.INITBALANCE, true);			// Track progress
			iterationCounter[0]++;
			if (iterationCounter[0] % logInterval == 0) {
				logger.info("Completed {} iterations...", iterationCounter[0]);
			}
			return;
		}
		
		// Recursive case: try each risk multiplier for the current strategy
		String currentStrategy = strategies.get(depth);
		for (Double multiplier : riskMultipliers) {
			workingRisks.put(currentStrategy, multiplier);
			// Recursively process next strategy
			exploreRiskCombinations(strategies, riskMultipliers, predefinedStrategies,
					workingRisks, depth + 1, iterationCounter, logInterval);
		}
	}
	
	@Override
	public double evaluateFitness(Map<String, Double> riskAllocation) {
		model.addStrategyList(riskAllocation);
		statisticsService.calculate(ModelDataServiceImpl.INITBALANCE, true);
		
		// Use Martin Ratio as primary fitness metric (CAGR / Ulcer Index)
		// This balances return against risk-adjusted drawdown
		return model.getStatistics().get(model.getStatistics().size() - 1).martin;
	}
	
	/**
	 * Grid search: exhaustive evaluation of all combinations.
	 * Complexity: O(R^S) where R = risk levels, S = strategies
	 * Best for: 1-6 strategies (up to ~117k combinations)
	 */
	private Map<String, Double> gridSearch(
			List<String> strategies,
			List<Double> riskMultipliers,
			Map<String, Double> predefinedStrategies,
			int maxEvaluations) {
		
		int totalCombinations = (int) Math.pow(riskMultipliers.size(), strategies.size());
		if (totalCombinations > maxEvaluations) {
			logger.warn("Grid search requires {} evaluations but budget is {}. Consider using a heuristic method.",
					totalCombinations, maxEvaluations);
		}
		
		Map<String, Double> bestAllocation = null;
		double bestFitness = Double.NEGATIVE_INFINITY;
		int[] evaluationCount = {0};
		
		Map<String, Double> currentAllocation = new HashMap<>(predefinedStrategies);
		exploreGrid(strategies, riskMultipliers, predefinedStrategies, currentAllocation,
				0, evaluationCount, maxEvaluations, new double[]{bestFitness}, new Map[]{bestAllocation});
		
		logger.info("Grid search completed {} evaluations. Best fitness: {}", evaluationCount[0], bestFitness);
		return bestAllocation != null ? bestAllocation : new HashMap<>();
	}
	
	private void exploreGrid(
			List<String> strategies, List<Double> riskMultipliers,
			Map<String, Double> predefinedStrategies, Map<String, Double> currentAllocation,
			int depth, int[] evaluationCount, int maxEvaluations,
			double[] bestFitness, Map<String, Double>[] bestAllocation) {
		
		if (evaluationCount[0] >= maxEvaluations) {
			return;
		}
		
		if (depth == strategies.size()) {
			currentAllocation.putAll(predefinedStrategies);
			double fitness = evaluateFitness(currentAllocation);
			evaluationCount[0]++;
			
			if (fitness > bestFitness[0]) {
				bestFitness[0] = fitness;
				bestAllocation[0] = new HashMap<>(currentAllocation);
				logger.debug("New best fitness: {} with allocation: {}", fitness, currentAllocation);
			}
			return;
		}
		
		String strategy = strategies.get(depth);
		for (Double multiplier : riskMultipliers) {
			if (evaluationCount[0] >= maxEvaluations) {
				return;
			}
			currentAllocation.put(strategy, multiplier);
			exploreGrid(strategies, riskMultipliers, predefinedStrategies, currentAllocation,
					depth + 1, evaluationCount, maxEvaluations, bestFitness, bestAllocation);
		}
	}
	
	/**
	 * Random search: samples random combinations.
	 * Complexity: O(maxEvaluations)
	 * Best for: Quick baseline, large portfolios (10+ strategies)
	 */
	private Map<String, Double> randomSearch(
			List<String> strategies,
			List<Double> riskMultipliers,
			Map<String, Double> predefinedStrategies,
			int maxEvaluations) {
		
		ThreadLocalRandom random = ThreadLocalRandom.current();
		Map<String, Double> bestAllocation = null;
		double bestFitness = Double.NEGATIVE_INFINITY;
		
		for (int i = 0; i < maxEvaluations; i++) {
			Map<String, Double> allocation = new HashMap<>(predefinedStrategies);
			
			// Randomly sample risk multiplier for each strategy
			for (String strategy : strategies) {
				int randomIndex = random.nextInt(riskMultipliers.size());
				allocation.put(strategy, riskMultipliers.get(randomIndex));
			}
			
			double fitness = evaluateFitness(allocation);
			
			if (fitness > bestFitness) {
				bestFitness = fitness;
				bestAllocation = new HashMap<>(allocation);
				logger.debug("Random search iteration {}: new best fitness {} ", i, fitness);
			}
			
			if ((i + 1) % 1000 == 0) {
				logger.info("Random search progress: {}/{} evaluations, best fitness: {}",
						i + 1, maxEvaluations, bestFitness);
			}
		}
		
		logger.info("Random search completed. Best fitness: {}", bestFitness);
		return bestAllocation != null ? bestAllocation : new HashMap<>();
	}
	
	/**
	 * Genetic algorithm: evolution-based optimization.
	 * Complexity: O(P * G) where P = population, G = generations
	 * Best for: 7-15 strategies
	 */
	private Map<String, Double> geneticAlgorithm(
			List<String> strategies,
			List<Double> riskMultipliers,
			Map<String, Double> predefinedStrategies,
			int maxEvaluations) {
		
		int generations = Math.min(GA_GENERATIONS, maxEvaluations / GA_POPULATION_SIZE);
		logger.info("Running GA: {} generations, population size {}", generations, GA_POPULATION_SIZE);
		
		// Initialize population randomly
		List<Individual> population = new ArrayList<>();
		for (int i = 0; i < GA_POPULATION_SIZE; i++) {
			population.add(createRandomIndividual(strategies, riskMultipliers, predefinedStrategies));
		}
		
		Individual best = null;
		int evaluations = 0;
		
		for (int gen = 0; gen < generations && evaluations < maxEvaluations; gen++) {
			// Evaluate fitness for all individuals
			for (Individual ind : population) {
				if (evaluations >= maxEvaluations) break;
				if (ind.fitness == null) {
					ind.fitness = evaluateFitness(ind.allocation);
					evaluations++;
				}
			}
			
			// Find best individual
			Individual genBest = population.stream()
					.max(Comparator.comparingDouble(ind -> ind.fitness))
					.orElse(null);
			
			if (best == null || (genBest != null && genBest.fitness > best.fitness)) {
				best = genBest;
				logger.debug("Generation {}: best fitness = {}", gen, best.fitness);
			}
			
			if ((gen + 1) % 10 == 0) {
				logger.info("GA generation {}/{}: best fitness = {}", gen + 1, generations, best.fitness);
			}
			
			// Create next generation
			List<Individual> nextGen = new ArrayList<>();
			
			// Elitism: keep top 10%
			List<Individual> sorted = population.stream()
					.sorted((a, b) -> Double.compare(b.fitness, a.fitness))
					.collect(Collectors.toList());
			int eliteCount = Math.max(1, GA_POPULATION_SIZE / 10);
			nextGen.addAll(sorted.subList(0, Math.min(eliteCount, sorted.size())));
			
			// Generate offspring
			while (nextGen.size() < GA_POPULATION_SIZE) {
				Individual parent1 = tournamentSelect(population);
				Individual parent2 = tournamentSelect(population);
				
				Individual offspring = ThreadLocalRandom.current().nextDouble() < GA_CROSSOVER_RATE
						? crossover(parent1, parent2, strategies)
						: new Individual(new HashMap<>(parent1.allocation));
				
				if (ThreadLocalRandom.current().nextDouble() < GA_MUTATION_RATE) {
					mutate(offspring, strategies, riskMultipliers);
				}
				
				nextGen.add(offspring);
			}
			
			population = nextGen;
		}
		
		logger.info("GA completed {} evaluations. Best fitness: {}", evaluations, best.fitness);
		return best != null ? best.allocation : new HashMap<>();
	}
	
	/**
	 * Simulated annealing: probabilistic hill climbing.
	 * Complexity: O(maxEvaluations)
	 * Best for: Continuous optimization feel, 5-12 strategies
	 */
	private Map<String, Double> simulatedAnnealing(
			List<String> strategies,
			List<Double> riskMultipliers,
			Map<String, Double> predefinedStrategies,
			int maxEvaluations) {
		
		ThreadLocalRandom random = ThreadLocalRandom.current();
		
		// Start with random solution
		Map<String, Double> current = createRandomAllocation(strategies, riskMultipliers, predefinedStrategies);
		double currentFitness = evaluateFitness(current);
		
		Map<String, Double> best = new HashMap<>(current);
		double bestFitness = currentFitness;
		
		double temperature = SA_INITIAL_TEMP;
		int evaluations = 1;
		
		while (evaluations < maxEvaluations && temperature > 0.01) {
			for (int i = 0; i < SA_ITERATIONS_PER_TEMP && evaluations < maxEvaluations; i++) {
				// Generate neighbor by tweaking one strategy
				Map<String, Double> neighbor = new HashMap<>(current);
				String strategy = strategies.get(random.nextInt(strategies.size()));
				neighbor.put(strategy, riskMultipliers.get(random.nextInt(riskMultipliers.size())));
				
				double neighborFitness = evaluateFitness(neighbor);
				evaluations++;
				
				// Accept if better, or probabilistically if worse
				double delta = neighborFitness - currentFitness;
				if (delta > 0 || random.nextDouble() < Math.exp(delta / temperature)) {
					current = neighbor;
					currentFitness = neighborFitness;
					
					if (currentFitness > bestFitness) {
						best = new HashMap<>(current);
						bestFitness = currentFitness;
						logger.debug("SA found new best: fitness = {} at temp = {}", bestFitness, temperature);
					}
				}
			}
			
			temperature *= SA_COOLING_RATE;
			
			if (evaluations % 1000 == 0) {
				logger.info("SA progress: {} evaluations, temp = {:.2f}, best fitness = {}",
						evaluations, temperature, bestFitness);
			}
		}
		
		logger.info("SA completed {} evaluations. Best fitness: {}", evaluations, bestFitness);
		return best;
	}
	
	/**
	 * Coarse-to-fine: hierarchical search with progressive refinement.
	 * Complexity: O(stages * evaluations_per_stage)
	 * Best for: Large portfolios with gradual refinement
	 */
	private Map<String, Double> coarseToFine(
			List<String> strategies,
			List<Double> riskMultipliers,
			Map<String, Double> predefinedStrategies,
			int maxEvaluations) {
		
		// Stage 1: Coarse search with fewer risk levels (use 3 levels)
		List<Double> coarseMultipliers = Arrays.asList(
				riskMultipliers.get(0),
				riskMultipliers.get(riskMultipliers.size() / 2),
				riskMultipliers.get(riskMultipliers.size() - 1)
		);
		
		int stage1Budget = maxEvaluations / 3;
		logger.info("Coarse-to-fine stage 1: using {} risk levels, budget {}", 
				coarseMultipliers.size(), stage1Budget);
		
		Map<String, Double> coarseBest = randomSearch(strategies, coarseMultipliers, 
				predefinedStrategies, stage1Budget);
		
		// Stage 2: Refine around best solution
		int stage2Budget = maxEvaluations / 3;
		logger.info("Coarse-to-fine stage 2: refining around best solution, budget {}", stage2Budget);
		
		Map<String, Double> refined = localSearch(strategies, riskMultipliers, coarseBest,
				predefinedStrategies, stage2Budget);
		
		// Stage 3: Final polish with full resolution
		int stage3Budget = maxEvaluations - stage1Budget - stage2Budget;
		logger.info("Coarse-to-fine stage 3: final refinement, budget {}", stage3Budget);
		
		Map<String, Double> finalBest = localSearch(strategies, riskMultipliers, refined,
				predefinedStrategies, stage3Budget);
		
		return finalBest;
	}
	
	// Helper methods for genetic algorithm
	
	private static class Individual {
		Map<String, Double> allocation;
		Double fitness;
		
		Individual(Map<String, Double> allocation) {
			this.allocation = allocation;
		}
	}
	
	private Individual createRandomIndividual(List<String> strategies, List<Double> riskMultipliers,
											   Map<String, Double> predefinedStrategies) {
		Map<String, Double> allocation = createRandomAllocation(strategies, riskMultipliers, predefinedStrategies);
		return new Individual(allocation);
	}
	
	private Map<String, Double> createRandomAllocation(List<String> strategies, List<Double> riskMultipliers,
														Map<String, Double> predefinedStrategies) {
		ThreadLocalRandom random = ThreadLocalRandom.current();
		Map<String, Double> allocation = new HashMap<>(predefinedStrategies);
		
		for (String strategy : strategies) {
			allocation.put(strategy, riskMultipliers.get(random.nextInt(riskMultipliers.size())));
		}
		
		return allocation;
	}
	
	private Individual tournamentSelect(List<Individual> population) {
		ThreadLocalRandom random = ThreadLocalRandom.current();
		Individual best = null;
		
		for (int i = 0; i < GA_TOURNAMENT_SIZE; i++) {
			Individual candidate = population.get(random.nextInt(population.size()));
			if (best == null || candidate.fitness > best.fitness) {
				best = candidate;
			}
		}
		
		return best;
	}
	
	private Individual crossover(Individual parent1, Individual parent2, List<String> strategies) {
		ThreadLocalRandom random = ThreadLocalRandom.current();
		Map<String, Double> childAllocation = new HashMap<>();
		
		for (String strategy : strategies) {
			// Randomly inherit from either parent
			childAllocation.put(strategy,
					random.nextBoolean() ? parent1.allocation.get(strategy) : parent2.allocation.get(strategy));
		}
		
		return new Individual(childAllocation);
	}
	
	private void mutate(Individual individual, List<String> strategies, List<Double> riskMultipliers) {
		ThreadLocalRandom random = ThreadLocalRandom.current();
		String strategy = strategies.get(random.nextInt(strategies.size()));
		individual.allocation.put(strategy, riskMultipliers.get(random.nextInt(riskMultipliers.size())));
		individual.fitness = null; // Invalidate fitness cache
	}
	
	private Map<String, Double> localSearch(List<String> strategies, List<Double> riskMultipliers,
											 Map<String, Double> startPoint, Map<String, Double> predefinedStrategies,
											 int maxEvaluations) {
		Map<String, Double> best = new HashMap<>(startPoint);
		best.putAll(predefinedStrategies);
		double bestFitness = evaluateFitness(best);
		int evaluations = 1;
		
		ThreadLocalRandom random = ThreadLocalRandom.current();
		
		while (evaluations < maxEvaluations) {
			// Try modifying one strategy at a time
			String strategy = strategies.get(random.nextInt(strategies.size()));
			Double originalValue = best.get(strategy);
			
			for (Double multiplier : riskMultipliers) {
				if (evaluations >= maxEvaluations) break;
				
				Map<String, Double> neighbor = new HashMap<>(best);
				neighbor.put(strategy, multiplier);
				
				double fitness = evaluateFitness(neighbor);
				evaluations++;
				
				if (fitness > bestFitness) {
					best = neighbor;
					bestFitness = fitness;
					logger.debug("Local search improvement: fitness = {}", bestFitness);
				}
			}
		}
		
		return best;
	}
}
