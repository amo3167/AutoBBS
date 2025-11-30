#!/usr/bin/env python3
"""
BTCUSD Trade Analysis Script
Analyzes results.txt to identify patterns and optimization opportunities
"""

import csv
from datetime import datetime
from collections import defaultdict

def parse_datetime(date_str):
    """Parse datetime from DD/MM/YYYY HH:MM format"""
    try:
        return datetime.strptime(date_str, "%d/%m/%Y %H:%M")
    except:
        try:
            return datetime.strptime(date_str, "%Y.%m.%d %H:%M")
        except:
            return None

def analyze_trades(filename):
    """Analyze BTCUSD trading results"""
    
    # Data structures
    trades = []
    weekday_stats = defaultdict(lambda: {'profit': 0, 'count': 0, 'wins': 0})
    hour_stats = defaultdict(lambda: {'profit': 0, 'count': 0, 'wins': 0})
    month_stats = defaultdict(lambda: {'profit': 0, 'count': 0, 'wins': 0})
    year_stats = defaultdict(lambda: {'profit': 0, 'count': 0, 'wins': 0})
    
    total_profit = 0
    total_trades = 0
    winning_trades = 0
    
    # Read CSV
    with open(filename, 'r') as f:
        reader = csv.DictReader(f, skipinitialspace=True)
        for row in reader:
            open_time = parse_datetime(row['Open Time'].strip())
            close_time = parse_datetime(row['Close Time'].strip())
            
            if not open_time or not close_time:
                continue
                
            profit = float(row['Profit'])
            
            trades.append({
                'open_time': open_time,
                'close_time': close_time,
                'profit': profit,
                'order_type': row['Order Type'],
                'duration_hours': (close_time - open_time).total_seconds() / 3600
            })
            
            # Aggregate stats
            total_profit += profit
            total_trades += 1
            if profit > 0:
                winning_trades += 1
            
            # Weekday (0=Monday, 6=Sunday)
            weekday = open_time.weekday()
            weekday_stats[weekday]['profit'] += profit
            weekday_stats[weekday]['count'] += 1
            if profit > 0:
                weekday_stats[weekday]['wins'] += 1
            
            # Hour
            hour = open_time.hour
            hour_stats[hour]['profit'] += profit
            hour_stats[hour]['count'] += 1
            if profit > 0:
                hour_stats[hour]['wins'] += 1
            
            # Month
            month = open_time.month
            month_stats[month]['profit'] += profit
            month_stats[month]['count'] += 1
            if profit > 0:
                month_stats[month]['wins'] += 1
                
            # Year
            year = open_time.year
            year_stats[year]['profit'] += profit
            year_stats[year]['count'] += 1
            if profit > 0:
                year_stats[year]['wins'] += 1
    
    # Print results
    print(f"\n{'='*80}")
    print(f"BTCUSD TRADE ANALYSIS")
    print(f"{'='*80}")
    print(f"\nOVERALL STATISTICS:")
    print(f"  Total Trades: {total_trades}")
    print(f"  Total Profit: ${total_profit:,.2f}")
    print(f"  Win Rate: {100*winning_trades/total_trades:.1f}%")
    print(f"  Avg Profit/Trade: ${total_profit/total_trades:.2f}")
    
    # Weekday analysis
    print(f"\n{'='*80}")
    print(f"DAY OF WEEK ANALYSIS:")
    print(f"{'='*80}")
    weekday_names = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday']
    for day in range(7):
        if day in weekday_stats:
            stats = weekday_stats[day]
            win_rate = 100 * stats['wins'] / stats['count'] if stats['count'] > 0 else 0
            avg_profit = stats['profit'] / stats['count'] if stats['count'] > 0 else 0
            print(f"  {weekday_names[day]:9s}: ${stats['profit']:8,.2f} profit | "
                  f"{stats['count']:3d} trades | {win_rate:5.1f}% wins | ${avg_profit:6.2f} avg")
    
    # Hour analysis
    print(f"\n{'='*80}")
    print(f"HOUR OF DAY ANALYSIS:")
    print(f"{'='*80}")
    for hour in range(24):
        if hour in hour_stats:
            stats = hour_stats[hour]
            win_rate = 100 * stats['wins'] / stats['count'] if stats['count'] > 0 else 0
            avg_profit = stats['profit'] / stats['count'] if stats['count'] > 0 else 0
            print(f"  Hour {hour:02d}: ${stats['profit']:8,.2f} profit | "
                  f"{stats['count']:3d} trades | {win_rate:5.1f}% wins | ${avg_profit:6.2f} avg")
    
    # Session analysis
    print(f"\n{'='*80}")
    print(f"SESSION ANALYSIS:")
    print(f"{'='*80}")
    sessions = {
        'Asian (00-08)': range(0, 8),
        'London (08-16)': range(8, 16),
        'NY (16-24)': range(16, 24)
    }
    for session_name, hours in sessions.items():
        session_profit = sum(hour_stats[h]['profit'] for h in hours if h in hour_stats)
        session_trades = sum(hour_stats[h]['count'] for h in hours if h in hour_stats)
        session_wins = sum(hour_stats[h]['wins'] for h in hours if h in hour_stats)
        if session_trades > 0:
            win_rate = 100 * session_wins / session_trades
            avg_profit = session_profit / session_trades
            print(f"  {session_name:15s}: ${session_profit:8,.2f} profit | "
                  f"{session_trades:3d} trades | {win_rate:5.1f}% wins | ${avg_profit:6.2f} avg")
    
    # Month analysis
    print(f"\n{'='*80}")
    print(f"MONTH ANALYSIS:")
    print(f"{'='*80}")
    month_names = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
    for month in range(1, 13):
        if month in month_stats:
            stats = month_stats[month]
            win_rate = 100 * stats['wins'] / stats['count'] if stats['count'] > 0 else 0
            avg_profit = stats['profit'] / stats['count'] if stats['count'] > 0 else 0
            print(f"  {month_names[month-1]:3s}: ${stats['profit']:8,.2f} profit | "
                  f"{stats['count']:3d} trades | {win_rate:5.1f}% wins | ${avg_profit:6.2f} avg")
    
    # Duration analysis
    print(f"\n{'='*80}")
    print(f"TRADE DURATION ANALYSIS:")
    print(f"{'='*80}")
    duration_buckets = {
        '< 1 hour': [],
        '1-4 hours': [],
        '4-8 hours': [],
        '8-24 hours': [],
        '> 24 hours': []
    }
    for trade in trades:
        duration = trade['duration_hours']
        if duration < 1:
            duration_buckets['< 1 hour'].append(trade['profit'])
        elif duration < 4:
            duration_buckets['1-4 hours'].append(trade['profit'])
        elif duration < 8:
            duration_buckets['4-8 hours'].append(trade['profit'])
        elif duration < 24:
            duration_buckets['8-24 hours'].append(trade['profit'])
        else:
            duration_buckets['> 24 hours'].append(trade['profit'])
    
    for bucket_name, profits in duration_buckets.items():
        if profits:
            total = sum(profits)
            count = len(profits)
            wins = sum(1 for p in profits if p > 0)
            win_rate = 100 * wins / count
            avg = total / count
            print(f"  {bucket_name:12s}: ${total:8,.2f} profit | "
                  f"{count:3d} trades | {win_rate:5.1f}% wins | ${avg:6.2f} avg")
    
    # Profit distribution
    print(f"\n{'='*80}")
    print(f"PROFIT DISTRIBUTION:")
    print(f"{'='*80}")
    profit_buckets = {
        'Large Loss (< -$500)': 0,
        'Medium Loss (-$500 to -$100)': 0,
        'Small Loss (-$100 to $0)': 0,
        'Small Win ($0 to $100)': 0,
        'Medium Win ($100 to $300)': 0,
        'Large Win (> $300)': 0
    }
    for trade in trades:
        p = trade['profit']
        if p < -500:
            profit_buckets['Large Loss (< -$500)'] += 1
        elif p < -100:
            profit_buckets['Medium Loss (-$500 to -$100)'] += 1
        elif p < 0:
            profit_buckets['Small Loss (-$100 to $0)'] += 1
        elif p < 100:
            profit_buckets['Small Win ($0 to $100)'] += 1
        elif p < 300:
            profit_buckets['Medium Win ($100 to $300)'] += 1
        else:
            profit_buckets['Large Win (> $300)'] += 1
    
    for bucket_name, count in profit_buckets.items():
        pct = 100 * count / total_trades if total_trades > 0 else 0
        print(f"  {bucket_name:30s}: {count:3d} trades ({pct:5.1f}%)")
    
    # Year analysis
    print(f"\n{'='*80}")
    print(f"YEAR ANALYSIS:")
    print(f"{'='*80}")
    for year in sorted(year_stats.keys()):
        stats = year_stats[year]
        win_rate = 100 * stats['wins'] / stats['count'] if stats['count'] > 0 else 0
        avg_profit = stats['profit'] / stats['count'] if stats['count'] > 0 else 0
        print(f"  {year}: ${stats['profit']:8,.2f} profit | "
              f"{stats['count']:3d} trades | {win_rate:5.1f}% wins | ${avg_profit:6.2f} avg")
    
    print(f"\n{'='*80}\n")

if __name__ == '__main__':
    analyze_trades('/Users/andym/projects/AutoBBS/ctester/tmp/BTCUSD_200002/results.txt')
