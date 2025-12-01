#!/usr/bin/env python3
"""
Tests for retry mechanism and error recovery
"""

import sys
import os
import time
import tempfile

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from include.retry_mechanism import (
    RetryConfig,
    retry_with_backoff,
    retry_file_operation,
    retry_email_send,
    graceful_degradation,
    HealthCheck
)

def test_retry_config():
    """Test RetryConfig"""
    print("Retry Test 1: RetryConfig...")
    try:
        config = RetryConfig(max_attempts=3, initial_delay=1.0)
        assert config.max_attempts == 3
        assert config.initial_delay == 1.0
        assert config.max_delay == 60.0
        assert config.exponential_base == 2.0
        
        print("  ✓ RetryConfig works correctly")
        return True
    except Exception as e:
        print(f"  ✗ RetryConfig test failed: {e}")
        return False

def test_retry_decorator():
    """Test retry decorator"""
    print("Retry Test 2: Retry decorator...")
    try:
        attempt_count = [0]
        
        @retry_with_backoff(RetryConfig(max_attempts=3, initial_delay=0.1))
        def failing_function():
            attempt_count[0] += 1
            if attempt_count[0] < 3:
                raise IOError("Simulated failure")
            return "success"
        
        result = failing_function()
        assert result == "success"
        assert attempt_count[0] == 3  # Should have tried 3 times
        
        print("  ✓ Retry decorator works correctly")
        return True
    except Exception as e:
        print(f"  ✗ Retry decorator test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_retry_file_operation():
    """Test file operation retry"""
    print("Retry Test 3: File operation retry...")
    try:
        # Test successful operation
        with tempfile.NamedTemporaryFile(mode='w', delete=False) as f:
            f.write("test content")
            temp_file = f.name
        
        def read_file():
            with open(temp_file, 'r') as f:
                return f.read()
        
        result = retry_file_operation(read_file, max_attempts=3)
        assert result == "test content"
        
        os.unlink(temp_file)
        
        # Test failing operation (file doesn't exist)
        def read_missing_file():
            with open("nonexistent_file_xyz123.txt", 'r') as f:
                return f.read()
        
        try:
            retry_file_operation(read_missing_file, max_attempts=2)
            assert False, "Should have raised exception"
        except (IOError, OSError, FileNotFoundError):
            pass  # Expected
        
        print("  ✓ File operation retry works correctly")
        return True
    except Exception as e:
        print(f"  ✗ File operation retry test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_graceful_degradation():
    """Test graceful degradation"""
    print("Retry Test 4: Graceful degradation...")
    try:
        # Test with fallback value
        def failing_func():
            raise ValueError("Error")
        
        result = graceful_degradation(failing_func, fallback_value="fallback")
        assert result == "fallback"
        
        # Test with fallback function
        def fallback_func():
            return "fallback_result"
        
        result = graceful_degradation(failing_func, fallback=fallback_func)
        assert result == "fallback_result"
        
        # Test successful operation
        def success_func():
            return "success"
        
        result = graceful_degradation(success_func, fallback_value="fallback")
        assert result == "success"
        
        print("  ✓ Graceful degradation works correctly")
        return True
    except Exception as e:
        print(f"  ✗ Graceful degradation test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_health_check():
    """Test health check"""
    print("Retry Test 5: Health check...")
    try:
        health = HealthCheck()
        
        # Register a healthy check
        health.register_check('test_healthy', lambda: True, timeout=1.0)
        
        # Register an unhealthy check
        health.register_check('test_unhealthy', lambda: False, timeout=1.0)
        
        # Register a failing check
        health.register_check('test_failing', lambda: (_ for _ in ()).throw(ValueError("Error")), timeout=1.0)
        
        # Run individual checks
        is_healthy, error = health.run_check('test_healthy')
        assert is_healthy is True
        assert error is None
        
        is_healthy, error = health.run_check('test_unhealthy')
        assert is_healthy is False
        
        is_healthy, error = health.run_check('test_failing')
        assert is_healthy is False
        assert error is not None
        
        # Run all checks
        results = health.run_all_checks()
        assert 'test_healthy' in results
        assert 'test_unhealthy' in results
        assert 'test_failing' in results
        
        # Get status
        status = health.get_status()
        assert 'overall' in status
        assert 'checks' in status
        assert status['overall'] == 'unhealthy'  # At least one unhealthy
        
        print("  ✓ Health check works correctly")
        return True
    except Exception as e:
        print(f"  ✗ Health check test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run retry mechanism tests"""
    print("=" * 60)
    print("Retry Mechanism - Tests")
    print("=" * 60)
    print()
    
    tests = [
        test_retry_config,
        test_retry_decorator,
        test_retry_file_operation,
        test_graceful_degradation,
        test_health_check,
    ]
    
    results = []
    for test in tests:
        try:
            result = test()
            results.append(result)
        except Exception as e:
            print(f"  ✗ Test crashed: {e}")
            results.append(False)
        print()
    
    # Summary
    print("=" * 60)
    print("Retry Mechanism Test Summary")
    print("=" * 60)
    passed = sum(results)
    total = len(results)
    print(f"Passed: {passed}/{total}")
    print(f"Failed: {total - passed}/{total}")
    
    if passed == total:
        print("\n✓ All retry mechanism tests passed!")
        return 0
    else:
        print(f"\n✗ {total - passed} test(s) failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())

