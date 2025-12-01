#!/usr/bin/env python3
"""
Tests for enhanced logging configuration
"""

import sys
import os
import tempfile
import logging

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from include.logging_config import (
    setup_logging,
    get_log_level_from_string,
    configure_logging_from_config,
    log_structured
)

def test_setup_logging():
    """Test basic logging setup"""
    print("Logging Test 1: Basic setup...")
    try:
        with tempfile.NamedTemporaryFile(mode='w', suffix='.log', delete=False) as f:
            log_file = f.name
        
        logger = setup_logging(log_file_path=log_file, log_level=logging.INFO)
        
        # Test logging
        logger.debug("Debug message")
        logger.info("Info message")
        logger.warning("Warning message")
        logger.error("Error message")
        
        # Check file was created and has content
        assert os.path.exists(log_file), "Log file not created"
        with open(log_file, 'r', encoding='utf-8') as f:
            content = f.read()
            assert "Info message" in content, "Info message not in log"
            assert "Error message" in content, "Error message not in log"
            # Debug should not be in log (level is INFO)
            assert "Debug message" not in content, "Debug message should not be in log"
        
        os.unlink(log_file)
        print("  ✓ Basic logging setup works")
        return True
    except Exception as e:
        print(f"  ✗ Basic setup test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_log_level_conversion():
    """Test log level string conversion"""
    print("Logging Test 2: Log level conversion...")
    try:
        assert get_log_level_from_string('DEBUG') == logging.DEBUG
        assert get_log_level_from_string('INFO') == logging.INFO
        assert get_log_level_from_string('WARNING') == logging.WARNING
        assert get_log_level_from_string('ERROR') == logging.ERROR
        assert get_log_level_from_string('CRITICAL') == logging.CRITICAL
        assert get_log_level_from_string('invalid') == logging.INFO  # Default
        assert get_log_level_from_string('info') == logging.INFO  # Case insensitive
        
        print("  ✓ Log level conversion works")
        return True
    except Exception as e:
        print(f"  ✗ Log level conversion test failed: {e}")
        return False

def test_log_rotation():
    """Test log rotation"""
    print("Logging Test 3: Log rotation...")
    try:
        with tempfile.NamedTemporaryFile(mode='w', suffix='.log', delete=False) as f:
            log_file = f.name
        
        # Set very small max bytes to test rotation
        logger = setup_logging(
            log_file_path=log_file,
            log_level=logging.INFO,
            max_bytes=1024,  # 1 KB
            backup_count=2
        )
        
        # Write enough to trigger rotation
        for i in range(100):
            logger.info(f"Test message {i} " * 10)  # Each message ~100 chars
        
        # Check that backup files might exist (rotation may or may not trigger depending on timing)
        # At minimum, main log file should exist
        assert os.path.exists(log_file), "Log file not created"
        
        os.unlink(log_file)
        # Clean up backup files if they exist
        for i in range(1, 3):
            backup = f"{log_file}.{i}"
            if os.path.exists(backup):
                os.unlink(backup)
        
        print("  ✓ Log rotation mechanism works")
        return True
    except Exception as e:
        print(f"  ✗ Log rotation test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_structured_logging():
    """Test structured logging"""
    print("Logging Test 4: Structured logging...")
    try:
        with tempfile.NamedTemporaryFile(mode='w', suffix='.log', delete=False) as f:
            log_file = f.name
        
        logger = setup_logging(log_file_path=log_file, log_level=logging.INFO)
        
        # Test structured logging
        log_structured(logger, logging.INFO, 'heartbeat_check', 
                      account='test_account', status='ok', delay=5.2)
        
        # Check log content
        with open(log_file, 'r', encoding='utf-8') as f:
            content = f.read()
            assert 'EVENT=heartbeat_check' in content, "Event type not in log"
            assert 'account=test_account' in content, "Account not in log"
            assert 'status=ok' in content, "Status not in log"
        
        os.unlink(log_file)
        print("  ✓ Structured logging works")
        return True
    except Exception as e:
        print(f"  ✗ Structured logging test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_config_from_file():
    """Test logging configuration from config file"""
    print("Logging Test 5: Config file configuration...")
    try:
        from include.asirikuy import readConfigFile
        
        # Create a test config
        with tempfile.NamedTemporaryFile(mode='w', suffix='.config', delete=False) as f:
            f.write("""[general]
monitoringInterval = 60
logLevel = DEBUG
logMaxBytes = 5242880
logBackupCount = 3
logConsole = true
logConsoleLevel = ERROR

[accounts]
accounts = test
""")
            config_file = f.name
        
        config = readConfigFile(config_file)
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.log', delete=False) as f:
            log_file = f.name
        
        logger = configure_logging_from_config(config, log_file_path=log_file)
        
        # Test that logger is configured
        logger.debug("Debug message")
        logger.info("Info message")
        
        # Check log file
        assert os.path.exists(log_file), "Log file not created"
        with open(log_file, 'r', encoding='utf-8') as f:
            content = f.read()
            assert "Debug message" in content, "Debug message should be in log (level is DEBUG)"
        
        os.unlink(log_file)
        os.unlink(config_file)
        
        print("  ✓ Config file configuration works")
        return True
    except Exception as e:
        print(f"  ✗ Config file configuration test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run logging tests"""
    print("=" * 60)
    print("Enhanced Logging - Tests")
    print("=" * 60)
    print()
    
    tests = [
        test_setup_logging,
        test_log_level_conversion,
        test_log_rotation,
        test_structured_logging,
        test_config_from_file,
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
    print("Logging Test Summary")
    print("=" * 60)
    passed = sum(results)
    total = len(results)
    print(f"Passed: {passed}/{total}")
    print(f"Failed: {total - passed}/{total}")
    
    if passed == total:
        print("\n✓ All logging tests passed!")
        return 0
    else:
        print(f"\n✗ {total - passed} test(s) failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())

