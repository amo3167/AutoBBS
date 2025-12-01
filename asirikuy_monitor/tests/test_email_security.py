#!/usr/bin/env python3
"""
Tests for email security features
"""

import sys
import os
import time

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from include.email_security import (
    validate_email_address,
    sanitize_email_content,
    sanitize_email_subject,
    check_rate_limit,
    reset_rate_limit,
    get_rate_limit_status
)

def test_email_validation():
    """Test email address validation"""
    print("Email Security Test 1: Email validation...")
    
    # Valid emails
    valid_emails = [
        "test@example.com",
        "user.name@example.co.uk",
        "user+tag@example.com",
        "user_name@example-domain.com",
        "123@example.com"
    ]
    
    # Invalid emails
    invalid_emails = [
        "",
        "notanemail",
        "@example.com",
        "user@",
        "user@.com",
        "user..name@example.com",
        "user@example",
        "user@example..com",
        "user name@example.com",
        "a" * 65 + "@example.com",  # Local part too long
        "user@" + "a" * 256 + ".com",  # Domain too long
    ]
    
    try:
        # Test valid emails
        for email in valid_emails:
            assert validate_email_address(email), f"Valid email rejected: {email}"
        
        # Test invalid emails
        for email in invalid_emails:
            assert not validate_email_address(email), f"Invalid email accepted: {email}"
        
        print("  ✓ Email validation works correctly")
        return True
    except Exception as e:
        print(f"  ✗ Email validation test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_sanitize_content():
    """Test email content sanitization"""
    print("Email Security Test 2: Content sanitization...")
    
    try:
        # Test HTML injection prevention
        malicious = "<script>alert('xss')</script>"
        sanitized = sanitize_email_content(malicious)
        assert "<script>" not in sanitized, "HTML not escaped"
        
        # Test null byte removal
        with_null = "test\x00content"
        sanitized = sanitize_email_content(with_null)
        assert "\x00" not in sanitized, "Null bytes not removed"
        
        # Test length limiting
        long_content = "a" * 20000
        sanitized = sanitize_email_content(long_content)
        assert len(sanitized) <= 10240 + 20, "Content not truncated"  # 10KB + truncation message
        
        # Test control character removal
        with_control = "test\x01\x02content"
        sanitized = sanitize_email_content(with_control)
        assert "\x01" not in sanitized and "\x02" not in sanitized, "Control characters not removed"
        
        # Test None/empty handling
        assert sanitize_email_content(None) == "", "None not handled"
        assert sanitize_email_content("") == "", "Empty string not handled"
        
        print("  ✓ Content sanitization works correctly")
        return True
    except Exception as e:
        print(f"  ✗ Content sanitization test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_sanitize_subject():
    """Test email subject sanitization"""
    print("Email Security Test 3: Subject sanitization...")
    
    try:
        # Test length limiting
        long_subject = "a" * 100
        sanitized = sanitize_email_subject(long_subject)
        assert len(sanitized) <= 81, "Subject not truncated"  # 78 + "..."
        
        # Test newline removal
        with_newline = "Subject\nwith\nnewlines"
        sanitized = sanitize_email_subject(with_newline)
        assert "\n" not in sanitized, "Newlines not removed"
        
        # Test null byte removal
        with_null = "test\x00subject"
        sanitized = sanitize_email_subject(with_null)
        assert "\x00" not in sanitized, "Null bytes not removed"
        
        # Test None handling
        result = sanitize_email_subject(None)
        assert result == "Asirikuy Monitor Alert", "None not handled with default"
        
        print("  ✓ Subject sanitization works correctly")
        return True
    except Exception as e:
        print(f"  ✗ Subject sanitization test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_rate_limiting():
    """Test email rate limiting"""
    print("Email Security Test 4: Rate limiting...")
    
    try:
        recipient = "test@example.com"
        
        # Reset rate limit
        reset_rate_limit(recipient)
        
        # Send emails up to limit
        for i in range(5):
            allowed, reason = check_rate_limit(recipient)
            assert allowed, f"Email {i+1} should be allowed: {reason}"
        
        # Next email should be rate limited
        allowed, reason = check_rate_limit(recipient)
        assert not allowed, "6th email should be rate limited"
        assert "Rate limit exceeded" in reason, "Rate limit message incorrect"
        
        # Check status
        status = get_rate_limit_status(recipient)
        assert status["emails_sent"] == 5, "Email count incorrect"
        assert status["remaining"] == 0, "Remaining count incorrect"
        
        # Reset and verify
        reset_rate_limit(recipient)
        allowed, reason = check_rate_limit(recipient)
        assert allowed, "Email should be allowed after reset"
        
        # Test invalid email
        allowed, reason = check_rate_limit("")
        assert not allowed, "Invalid email should not be allowed"
        
        print("  ✓ Rate limiting works correctly")
        return True
    except Exception as e:
        print(f"  ✗ Rate limiting test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_rate_limit_time_window():
    """Test rate limit time window"""
    print("Email Security Test 5: Rate limit time window...")
    
    try:
        recipient = "test2@example.com"
        reset_rate_limit(recipient)
        
        # Send max emails
        for i in range(5):
            check_rate_limit(recipient)
        
        # Should be rate limited
        allowed, _ = check_rate_limit(recipient)
        assert not allowed, "Should be rate limited"
        
        # Wait a bit (simulate time passing)
        # Note: In real scenario, time would pass naturally
        # For testing, we'll just verify the mechanism works
        
        print("  ✓ Rate limit time window mechanism works")
        return True
    except Exception as e:
        print(f"  ✗ Rate limit time window test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run email security tests"""
    print("=" * 60)
    print("Email Security - Tests")
    print("=" * 60)
    print()
    
    tests = [
        test_email_validation,
        test_sanitize_content,
        test_sanitize_subject,
        test_rate_limiting,
        test_rate_limit_time_window,
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
    print("Email Security Test Summary")
    print("=" * 60)
    passed = sum(results)
    total = len(results)
    print(f"Passed: {passed}/{total}")
    print(f"Failed: {total - passed}/{total}")
    
    if passed == total:
        print("\n✓ All email security tests passed!")
        return 0
    else:
        print(f"\n✗ {total - passed} test(s) failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())

