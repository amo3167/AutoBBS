#!/usr/bin/env python3
"""
Tests for cross-platform process management
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def test_process_manager_import():
    """Test that process_manager module can be imported"""
    print("Process Manager Test 1: Module import...")
    try:
        from include.process_manager import kill_process_by_name, kill_mt4_processes
        assert callable(kill_process_by_name)
        assert callable(kill_mt4_processes)
        print("  ✓ Process manager module imports successfully")
        return True
    except Exception as e:
        print(f"  ✗ Import failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_process_manager_functions():
    """Test that process manager functions exist and are callable"""
    print("Process Manager Test 2: Function availability...")
    try:
        from include.process_manager import kill_process_by_name, kill_mt4_processes
        
        # Test that functions can be called (even if no processes match)
        # This is safe - it will just return 0 if no processes found
        result1 = kill_process_by_name("nonexistent_process_xyz123")
        assert isinstance(result1, int)
        assert result1 == 0  # Should return 0 if no processes found
        
        result2 = kill_mt4_processes()
        assert isinstance(result2, int)
        assert result2 >= 0  # Should return 0 or more
        
        print("  ✓ Process manager functions work correctly")
        return True
    except Exception as e:
        print(f"  ✗ Function test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_psutil_available():
    """Test that psutil is available"""
    print("Process Manager Test 3: psutil availability...")
    try:
        import psutil
        # Test basic psutil functionality
        processes = list(psutil.process_iter(['pid', 'name']))
        assert len(processes) > 0  # Should have at least some processes
        print("  ✓ psutil is available and working")
        return True
    except Exception as e:
        print(f"  ✗ psutil test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run process manager tests"""
    print("=" * 60)
    print("Process Manager - Cross-Platform Tests")
    print("=" * 60)
    print()
    
    tests = [
        test_psutil_available,
        test_process_manager_import,
        test_process_manager_functions,
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
    print("Process Manager Test Summary")
    print("=" * 60)
    passed = sum(results)
    total = len(results)
    print(f"Passed: {passed}/{total}")
    print(f"Failed: {total - passed}/{total}")
    
    if passed == total:
        print("\n✓ All process manager tests passed!")
        return 0
    else:
        print(f"\n✗ {total - passed} test(s) failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())

