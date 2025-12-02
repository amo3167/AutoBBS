#!/usr/bin/env python3
"""
Tests for configuration management
"""

import sys
import os
import tempfile

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from include.config_manager import (
    validate_config,
    load_config_with_defaults,
    get_config_value,
    DEFAULT_CONFIG
)

def test_config_validation():
    """Test config validation"""
    print("Config Test 1: Validation...")
    try:
        from include.asirikuy import readConfigFile
        
        # Test valid config
        with tempfile.NamedTemporaryFile(mode='w', suffix='.config', delete=False) as f:
            f.write("""[general]
monitoringInterval = 60
useEmail = 0
weekOpenDay = 0
weekOpenHour = 6
weekCloseDay = 5
weekCloseHour = 7

[accounts]
accounts = test

[test]
accountNumber = 123
path = /test/path
frontend = MT4
""")
            config_file = f.name
        
        config = readConfigFile(config_file)
        errors = validate_config(config)
        assert len(errors) == 0, f"Valid config should have no errors: {errors}"
        
        # Test invalid config (missing section)
        with tempfile.NamedTemporaryFile(mode='w', suffix='.config', delete=False) as f:
            f.write("""[general]
monitoringInterval = 60
""")
            invalid_config_file = f.name
        
        invalid_config = readConfigFile(invalid_config_file)
        errors = validate_config(invalid_config)
        assert len(errors) > 0, "Invalid config should have errors"
        assert any("accounts" in e.lower() for e in errors), "Should detect missing accounts section"
        
        os.unlink(config_file)
        os.unlink(invalid_config_file)
        
        print("  ✓ Config validation works")
        return True
    except Exception as e:
        print(f"  ✗ Config validation test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_load_with_defaults():
    """Test loading config with defaults"""
    print("Config Test 2: Load with defaults...")
    try:
        # Create minimal config
        with tempfile.NamedTemporaryFile(mode='w', suffix='.config', delete=False) as f:
            f.write("""[general]
monitoringInterval = 30
weekOpenDay = 0
weekOpenHour = 6
weekCloseDay = 5
weekCloseHour = 7

[accounts]
accounts = test

[test]
accountNumber = 123
path = /test/path
frontend = MT4
""")
            config_file = f.name
        
        config = load_config_with_defaults(config_file)
        
        # Check that defaults were applied
        assert config.get('general', 'useEmail') == '0', "Default useEmail should be applied"
        assert config.get('general', 'smtpServer') == 'smtp.gmail.com:587', "Default smtpServer should be applied"
        
        # Check that provided values are used
        assert config.get('general', 'monitoringInterval') == '30', "Provided value should be used"
        
        os.unlink(config_file)
        
        print("  ✓ Load with defaults works")
        return True
    except Exception as e:
        print(f"  ✗ Load with defaults test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_get_config_value():
    """Test get_config_value helper"""
    print("Config Test 3: Get config value...")
    try:
        from include.asirikuy import readConfigFile
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.config', delete=False) as f:
            f.write("""[general]
monitoringInterval = 60

[accounts]
accounts = test
""")
            config_file = f.name
        
        config = readConfigFile(config_file)
        
        # Test existing value
        value = get_config_value(config, 'general', 'monitoringInterval')
        assert value == '60', "Should return existing value"
        
        # Test with default
        value = get_config_value(config, 'general', 'useEmail', default='0')
        assert value == '0', "Should return default for missing value"
        
        # Test missing value without default
        try:
            get_config_value(config, 'general', 'missingKey')
            assert False, "Should raise ValueError for missing key without default"
        except ValueError:
            pass  # Expected
        
        os.unlink(config_file)
        
        print("  ✓ Get config value works")
        return True
    except Exception as e:
        print(f"  ✗ Get config value test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run config manager tests"""
    print("=" * 60)
    print("Configuration Manager - Tests")
    print("=" * 60)
    print()
    
    tests = [
        test_config_validation,
        test_load_with_defaults,
        test_get_config_value,
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
    print("Config Manager Test Summary")
    print("=" * 60)
    passed = sum(results)
    total = len(results)
    print(f"Passed: {passed}/{total}")
    print(f"Failed: {total - passed}/{total}")
    
    if passed == total:
        print("\n✓ All config manager tests passed!")
        return 0
    else:
        print(f"\n✗ {total - passed} test(s) failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())

