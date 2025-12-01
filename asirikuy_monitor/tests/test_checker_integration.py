#!/usr/bin/env python3
"""
Integration test for checker.py main script
Tests the main monitoring functionality
"""

import sys
import os
import subprocess
import time

def test_version_command():
    """Test version command works"""
    print("Integration Test 1: Version command...")
    try:
        result = subprocess.run(
            [sys.executable, 'checker.py', '-v'],
            capture_output=True,
            text=True,
            timeout=5,
            cwd=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        )
        assert result.returncode == 0, f"Version command failed with exit code {result.returncode}"
        assert "Asirikuy Monitor" in result.stdout, "Version output not found"
        print("  ✓ Version command works")
        return True
    except subprocess.TimeoutExpired:
        print("  ✗ Version command timed out")
        return False
    except Exception as e:
        print(f"  ✗ Version command failed: {e}")
        return False

def test_config_file_loading():
    """Test that config file can be loaded"""
    print("Integration Test 2: Config file loading...")
    try:
        # Test with test config
        result = subprocess.run(
            [sys.executable, 'checker.py', '-c', 'tests/fixtures/test_config.config'],
            capture_output=True,
            text=True,
            timeout=2,
            cwd=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        )
        # The script will start monitoring loop, so we expect it to run
        # We'll just check it doesn't crash immediately
        # If it returns quickly with error, that's a problem
        # If it runs (even briefly), that's success
        if result.returncode != 0 and "Error" in result.stderr.lower():
            print(f"  ✗ Config loading failed: {result.stderr}")
            return False
        print("  ✓ Config file loads without immediate errors")
        return True
    except subprocess.TimeoutExpired:
        # Timeout is expected - script runs in a loop
        print("  ✓ Config file loads and script starts (timeout expected)")
        return True
    except Exception as e:
        print(f"  ✗ Config loading test failed: {e}")
        return False

def test_import_checker():
    """Test that checker.py can be imported"""
    print("Integration Test 3: Import checker module...")
    try:
        sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
        import checker
        # Test that main function exists
        assert hasattr(checker, 'main'), "main function not found"
        assert hasattr(checker, 'version'), "version function not found"
        print("  ✓ Checker module imports successfully")
        return True
    except Exception as e:
        print(f"  ✗ Import failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run integration tests"""
    print("=" * 60)
    print("Asirikuy Monitor - Integration Tests")
    print("=" * 60)
    print()
    
    tests = [
        test_import_checker,
        test_version_command,
        test_config_file_loading,
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
    print("Integration Test Summary")
    print("=" * 60)
    passed = sum(results)
    total = len(results)
    print(f"Passed: {passed}/{total}")
    print(f"Failed: {total - passed}/{total}")
    
    if passed == total:
        print("\n✓ All integration tests passed!")
        return 0
    else:
        print(f"\n✗ {total - passed} test(s) failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())

