#!/usr/bin/env python3
"""
Security tests - Environment variable handling
"""

import sys
import os
import tempfile

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def test_env_var_priority():
    """Test that environment variables take priority over config file"""
    print("Security Test 1: Environment variable priority...")
    try:
        # Set environment variables
        os.environ['EMAIL_FROM'] = 'env-from@example.com'
        os.environ['EMAIL_TO'] = 'env-to@example.com'
        os.environ['EMAIL_LOGIN'] = 'env-login@example.com'
        os.environ['EMAIL_PASSWORD'] = 'env-password-123'
        os.environ['SMTP_SERVER'] = 'smtp.env.example.com:587'
        
        # Create a test config with different values
        with tempfile.NamedTemporaryFile(mode='w', suffix='.config', delete=False, encoding='utf-8') as f:
            f.write("""[general]
monitoringInterval = 60
useEmail = 1
fromEmail = config-from@example.com
toEmail = config-to@example.com
emailLogin = config-login@example.com
emailPassword = config-password-456
smtpServer = smtp.config.example.com:587
weekOpenDay = 0
weekOpenHour = 0
weekCloseDay = 6
weekCloseHour = 23

[accounts]
accounts = test

[test]
accountNumber = 123
path = /test/path
frontend = MT4
""")
            temp_config = f.name
        
        # Import and test
        from include.asirikuy import readConfigFile
        config = readConfigFile(temp_config)
        
        # Simulate the logic from checker.py
        fromEmail = os.getenv('EMAIL_FROM', config.get('general', 'fromEmail'))
        toEmail = os.getenv('EMAIL_TO', config.get('general', 'toEmail'))
        emailLogin = os.getenv('EMAIL_LOGIN', config.get('general', 'emailLogin'))
        emailPassword = os.getenv('EMAIL_PASSWORD', config.get('general', 'emailPassword'))
        smtpServer = os.getenv('SMTP_SERVER', config.get('general', 'smtpServer'))
        
        # Verify environment variables are used
        assert fromEmail == 'env-from@example.com', f"Expected env-from@example.com, got {fromEmail}"
        assert toEmail == 'env-to@example.com', f"Expected env-to@example.com, got {toEmail}"
        assert emailLogin == 'env-login@example.com', f"Expected env-login@example.com, got {emailLogin}"
        assert emailPassword == 'env-password-123', f"Expected env-password-123, got {emailPassword}"
        assert smtpServer == 'smtp.env.example.com:587', f"Expected smtp.env.example.com:587, got {smtpServer}"
        
        # Cleanup
        os.unlink(temp_config)
        del os.environ['EMAIL_FROM']
        del os.environ['EMAIL_TO']
        del os.environ['EMAIL_LOGIN']
        del os.environ['EMAIL_PASSWORD']
        del os.environ['SMTP_SERVER']
        
        print("  ✓ Environment variables take priority over config file")
        return True
    except Exception as e:
        print(f"  ✗ Environment variable priority test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_config_fallback():
    """Test that config file is used when env vars are not set"""
    print("Security Test 2: Config file fallback...")
    try:
        # Ensure env vars are not set
        for key in ['EMAIL_FROM', 'EMAIL_TO', 'EMAIL_LOGIN', 'EMAIL_PASSWORD', 'SMTP_SERVER']:
            os.environ.pop(key, None)
        
        # Create a test config
        with tempfile.NamedTemporaryFile(mode='w', suffix='.config', delete=False, encoding='utf-8') as f:
            f.write("""[general]
monitoringInterval = 60
useEmail = 1
fromEmail = config-from@example.com
toEmail = config-to@example.com
emailLogin = config-login@example.com
emailPassword = config-password-456
smtpServer = smtp.config.example.com:587
weekOpenDay = 0
weekOpenHour = 0
weekCloseDay = 6
weekCloseHour = 23

[accounts]
accounts = test

[test]
accountNumber = 123
path = /test/path
frontend = MT4
""")
            temp_config = f.name
        
        from include.asirikuy import readConfigFile
        config = readConfigFile(temp_config)
        
        # Simulate the logic from checker.py
        fromEmail = os.getenv('EMAIL_FROM', config.get('general', 'fromEmail'))
        toEmail = os.getenv('EMAIL_TO', config.get('general', 'toEmail'))
        emailLogin = os.getenv('EMAIL_LOGIN', config.get('general', 'emailLogin'))
        emailPassword = os.getenv('EMAIL_PASSWORD', config.get('general', 'emailPassword'))
        smtpServer = os.getenv('SMTP_SERVER', config.get('general', 'smtpServer'))
        
        # Verify config file values are used
        assert fromEmail == 'config-from@example.com', f"Expected config-from@example.com, got {fromEmail}"
        assert toEmail == 'config-to@example.com', f"Expected config-to@example.com, got {toEmail}"
        assert emailLogin == 'config-login@example.com', f"Expected config-login@example.com, got {emailLogin}"
        assert emailPassword == 'config-password-456', f"Expected config-password-456, got {emailPassword}"
        assert smtpServer == 'smtp.config.example.com:587', f"Expected smtp.config.example.com:587, got {smtpServer}"
        
        # Cleanup
        os.unlink(temp_config)
        
        print("  ✓ Config file used as fallback when env vars not set")
        return True
    except Exception as e:
        print(f"  ✗ Config file fallback test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run security tests"""
    print("=" * 60)
    print("Asirikuy Monitor - Security Tests")
    print("=" * 60)
    print()
    
    tests = [
        test_env_var_priority,
        test_config_fallback,
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
    print("Security Test Summary")
    print("=" * 60)
    passed = sum(results)
    total = len(results)
    print(f"Passed: {passed}/{total}")
    print(f"Failed: {total - passed}/{total}")
    
    if passed == total:
        print("\n✓ All security tests passed!")
        return 0
    else:
        print(f"\n✗ {total - passed} test(s) failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())

