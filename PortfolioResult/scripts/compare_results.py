#!/usr/bin/env python3
"""
Regression Test Results Comparison Script

Compares current test results against baseline (golden) dataset.
Validates that all metrics match within specified tolerance.

Usage:
    python3 compare_results.py <baseline_portfolio> <current_portfolio> <baseline_optimization> <current_optimization> [tolerance]

Arguments:
    baseline_portfolio: Path to baseline portfolio statistics CSV
    current_portfolio: Path to current portfolio statistics CSV
    baseline_optimization: Path to baseline optimization results CSV
    current_optimization: Path to current optimization results CSV
    tolerance: Maximum allowed difference percentage (default: 0.01)
"""

import sys
import csv
import os
from pathlib import Path

def load_csv(filepath):
    """Load CSV file and return as list of dictionaries"""
    if not os.path.exists(filepath):
        print(f"ERROR: File not found: {filepath}")
        sys.exit(1)
    
    with open(filepath, 'r') as f:
        reader = csv.DictReader(f)
        return list(reader)

def compare_values(baseline_val, current_val, field_name, tolerance=0.01):
    """
    Compare two numeric values with tolerance.
    Returns (match, difference_percent, message)
    """
    try:
        baseline = float(baseline_val)
        current = float(current_val)
    except (ValueError, TypeError):
        return (False, None, f"Non-numeric value: baseline={baseline_val}, current={current_val}")
    
    if baseline == 0:
        # Handle zero baseline case
        if abs(current) < tolerance:
            return (True, 0.0, "Both zero or near-zero")
        else:
            diff_pct = abs(current) * 100
            return (False, diff_pct, f"Baseline is zero but current is {current}")
    
    diff = abs(current - baseline)
    diff_pct = (diff / abs(baseline)) * 100
    
    if diff_pct <= tolerance:
        return (True, diff_pct, f"Match (diff: {diff_pct:.6f}%)")
    else:
        return (False, diff_pct, f"Mismatch (diff: {diff_pct:.6f}% > {tolerance}%)")

def compare_portfolio_stats(baseline_file, current_file, tolerance):
    """Compare portfolio statistics"""
    print("=" * 80)
    print("Portfolio Statistics Comparison")
    print("=" * 80)
    print()
    
    baseline = load_csv(baseline_file)
    current = load_csv(current_file)
    
    if len(baseline) == 0 or len(current) == 0:
        print("ERROR: Empty CSV files")
        return False
    
    # Get first row (should be only one row for portfolio stats)
    baseline_row = baseline[0]
    current_row = current[0]
    
    # Get all numeric fields
    numeric_fields = [k for k in baseline_row.keys() 
                     if k not in ['StrategyID', 'Strategy'] and baseline_row[k]]
    
    print(f"Comparing {len(numeric_fields)} metrics...")
    print()
    
    all_match = True
    mismatches = []
    
    for field in numeric_fields:
        if field not in current_row:
            print(f"  ⚠️  {field}: Missing in current results")
            all_match = False
            continue
        
        baseline_val = baseline_row[field]
        current_val = current_row[field]
        
        match, diff_pct, message = compare_values(baseline_val, current_val, field, tolerance)
        
        if match:
            print(f"  ✓ {field:30s} {message}")
        else:
            print(f"  ✗ {field:30s} {message}")
            all_match = False
            mismatches.append({
                'field': field,
                'baseline': baseline_val,
                'current': current_val,
                'diff_pct': diff_pct
            })
    
    print()
    if all_match:
        print("✓ Portfolio Statistics: ALL MATCH")
    else:
        print("✗ Portfolio Statistics: MISMATCHES FOUND")
        print()
        print("Mismatches:")
        for m in mismatches:
            print(f"  {m['field']}: baseline={m['baseline']}, current={m['current']}, diff={m['diff_pct']:.6f}%")
    
    return all_match

def compare_optimization_results(baseline_file, current_file, tolerance):
    """Compare optimization results"""
    print("=" * 80)
    print("Optimization Results Comparison")
    print("=" * 80)
    print()
    
    baseline = load_csv(baseline_file)
    current = load_csv(current_file)
    
    if len(baseline) == 0:
        print("ERROR: Empty baseline file")
        return False
    
    if len(current) == 0:
        print("ERROR: Empty current file")
        return False
    
    # Compare best result (first row, sorted by Martin Ratio)
    baseline_best = baseline[0]
    current_best = current[0]
    
    print("Comparing best optimization result...")
    print()
    
    # Key metrics to compare
    key_metrics = ['TotalReturn', 'Cagr', 'Max_DD', 'Martin', 'Sharp_ratio', 
                   'Profit_factor', 'UlcerIndex', 'Winning']
    
    all_match = True
    mismatches = []
    
    for field in key_metrics:
        if field not in baseline_best or field not in current_best:
            continue
        
        baseline_val = baseline_best[field]
        current_val = current_best[field]
        
        match, diff_pct, message = compare_values(baseline_val, current_val, field, tolerance)
        
        if match:
            print(f"  ✓ {field:30s} {message}")
        else:
            print(f"  ✗ {field:30s} {message}")
            all_match = False
            mismatches.append({
                'field': field,
                'baseline': baseline_val,
                'current': current_val,
                'diff_pct': diff_pct
            })
    
    # Compare result counts
    baseline_count = len(baseline)
    current_count = len(current)
    
    print()
    print(f"  Baseline results: {baseline_count}")
    print(f"  Current results:  {current_count}")
    
    if baseline_count != current_count:
        count_diff = abs(current_count - baseline_count)
        count_diff_pct = (count_diff / baseline_count) * 100 if baseline_count > 0 else 0
        if count_diff_pct > tolerance:
            print(f"  ✗ Result count mismatch: {count_diff} difference ({count_diff_pct:.2f}%)")
            all_match = False
        else:
            print(f"  ⚠️  Result count difference: {count_diff} ({count_diff_pct:.2f}%) - within tolerance")
    else:
        print(f"  ✓ Result counts match")
    
    print()
    if all_match:
        print("✓ Optimization Results: ALL MATCH")
    else:
        print("✗ Optimization Results: MISMATCHES FOUND")
        if mismatches:
            print()
            print("Mismatches:")
            for m in mismatches:
                print(f"  {m['field']}: baseline={m['baseline']}, current={m['current']}, diff={m['diff_pct']:.6f}%")
    
    return all_match

def main():
    if len(sys.argv) < 5:
        print(__doc__)
        sys.exit(1)
    
    baseline_portfolio = sys.argv[1]
    current_portfolio = sys.argv[2]
    baseline_optimization = sys.argv[3]
    current_optimization = sys.argv[4]
    tolerance = float(sys.argv[5]) if len(sys.argv) > 5 else 0.01
    
    print()
    print("=" * 80)
    print("Regression Test Results Comparison")
    print("=" * 80)
    print()
    print(f"Tolerance: {tolerance}%")
    print()
    
    portfolio_match = compare_portfolio_stats(baseline_portfolio, current_portfolio, tolerance)
    print()
    optimization_match = compare_optimization_results(baseline_optimization, current_optimization, tolerance)
    
    print()
    print("=" * 80)
    if portfolio_match and optimization_match:
        print("✓ ALL REGRESSION TESTS PASSED")
        print("=" * 80)
        sys.exit(0)
    else:
        print("✗ REGRESSION TESTS FAILED")
        print("=" * 80)
        sys.exit(1)

if __name__ == '__main__':
    main()

