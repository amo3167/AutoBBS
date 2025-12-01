#!/usr/bin/env python3
"""
Integration tests for checker.py main script
Tests the integration of all modules working together
"""

import sys
import os
import subprocess
import time
import tempfile
import unittest
from unittest.mock import patch, MagicMock
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


class TestCheckerIntegration(unittest.TestCase):
    """Integration tests for checker.py"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.test_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        self.test_config = os.path.join(self.test_dir, 'tests', 'fixtures', 'test_config.config')
    
    def test_import_checker(self):
        """Test that checker.py can be imported and has required functions"""
        try:
            import checker
            # Test that main function exists
            self.assertTrue(hasattr(checker, 'main'), "main function not found")
            self.assertTrue(hasattr(checker, 'version'), "version function not found")
            self.assertTrue(callable(checker.main), "main is not callable")
            self.assertTrue(callable(checker.version), "version is not callable")
        except Exception as e:
            self.fail(f"Import failed: {e}")
    
    def test_version_command(self):
        """Test version command works via subprocess"""
        result = subprocess.run(
            [sys.executable, 'checker.py', '-v'],
            capture_output=True,
            text=True,
            timeout=5,
            cwd=self.test_dir
        )
        self.assertEqual(result.returncode, 0, f"Version command failed with exit code {result.returncode}")
        self.assertIn("Asirikuy Monitor", result.stdout, "Version output not found")
        self.assertIn("v0.05", result.stdout, "Version number not found")
    
    def test_version_command_import(self):
        """Test version function directly"""
        import checker
        from io import StringIO
        import sys
        
        # Capture stdout
        old_stdout = sys.stdout
        sys.stdout = StringIO()
        
        try:
            checker.version()
            output = sys.stdout.getvalue()
            self.assertIn("Asirikuy Monitor", output)
            self.assertIn("v0.05", output)
        finally:
            sys.stdout = old_stdout
    
    def test_config_file_loading(self):
        """Test that config file can be loaded"""
        import checker
        from include.config_manager import load_config_with_defaults
        
        # Test loading config file
        config = load_config_with_defaults(self.test_config)
        self.assertIsNotNone(config)
        self.assertTrue(config.has_section('general'))
        self.assertTrue(config.has_section('test_account'))
    
    def test_config_file_validation(self):
        """Test config file validation"""
        from include.config_manager import load_config_with_defaults, validate_config
        
        config = load_config_with_defaults(self.test_config)
        errors = validate_config(config)
        self.assertEqual(len(errors), 0, f"Config validation errors: {errors}")
    
    def test_module_imports(self):
        """Test that all required modules can be imported"""
        modules = [
            'include.asirikuy',
            'include.misc',
            'include.process_manager',
            'include.retry_mechanism',
            'include.config_manager',
            'include.logging_config',
            'include.platform_utils',
            'include.email_security',
            'include.version'
        ]
        
        for module_name in modules:
            try:
                __import__(module_name)
            except ImportError as e:
                self.fail(f"Failed to import {module_name}: {e}")
    
    def test_config_manager_integration(self):
        """Test config manager integration with checker"""
        from include.config_manager import load_config_with_defaults, get_config_value
        
        config = load_config_with_defaults(self.test_config)
        
        # Test getting values
        interval = get_config_value(config, 'general', 'monitoringInterval', '60')
        self.assertEqual(interval, '5')
        
        use_email = get_config_value(config, 'general', 'useEmail', '0')
        self.assertEqual(use_email, '0')
    
    def test_logging_integration(self):
        """Test logging configuration integration"""
        from include.config_manager import load_config_with_defaults
        from include.logging_config import configure_logging_from_config
        
        config = load_config_with_defaults(self.test_config)
        
        # Configure logging from config
        logger = configure_logging_from_config(config)
        self.assertIsNotNone(logger)
        self.assertEqual(logger.name, 'Monitor')
    
    def test_platform_utils_integration(self):
        """Test platform utilities integration"""
        from include.platform_utils import (
            detect_platform, get_platform_name, get_platform_info,
            is_windows, is_macos, is_linux
        )
        
        # Test platform detection
        platform_type = detect_platform()
        self.assertIsNotNone(platform_type)
        
        platform_name = get_platform_name()
        self.assertIsInstance(platform_name, str)
        self.assertIn(platform_name.lower(), ['windows', 'macos', 'linux', 'unknown'])
        
        # Test platform info
        info = get_platform_info()
        self.assertIsInstance(info, dict)
        self.assertIn('platform', info)
        self.assertIn('system', info)
    
    def test_email_security_integration(self):
        """Test email security module integration"""
        from include.email_security import (
            validate_email_address,
            sanitize_email_content,
            sanitize_email_subject,
            check_rate_limit
        )
        
        # Test email validation
        self.assertTrue(validate_email_address("test@example.com"))
        self.assertFalse(validate_email_address("invalid-email"))
        
        # Test content sanitization
        content = "<script>alert('xss')</script>"
        sanitized = sanitize_email_content(content)
        self.assertNotIn("<script>", sanitized)
        
        # Test subject sanitization
        subject = "Test\nSubject"
        sanitized_subject = sanitize_email_subject(subject)
        self.assertNotIn("\n", sanitized_subject)
        
        # Test rate limiting
        allowed, reason = check_rate_limit("test@example.com")
        self.assertIsInstance(allowed, bool)
        self.assertIsInstance(reason, str)
    
    @patch('smtplib.SMTP')
    def test_email_sending_integration(self, mock_smtp_class):
        """Test email sending integration with mocked SMTP"""
        from include.asirikuy import sendemail
        
        mock_server = MagicMock()
        mock_server.sendmail.return_value = {}
        mock_smtp_class.return_value = mock_server
        
        # Send test email
        sendemail(
            from_addr="test@example.com",
            to_addr_list="recipient@example.com",
            cc_addr_list=[],
            subject="Test Subject",
            message="Test Message",
            login="test@example.com",
            password="password",
            smtpserver="smtp.example.com:587"
        )
        
        # Verify SMTP was called
        mock_smtp_class.assert_called_once()
        mock_server.starttls.assert_called_once()
        mock_server.login.assert_called_once()
        mock_server.sendmail.assert_called_once()
        mock_server.quit.assert_called_once()
    
    def test_process_manager_integration(self):
        """Test process manager integration"""
        from include.process_manager import kill_process_by_name, kill_mt4_processes
        from include.platform_utils import get_mt4_process_name
        
        # Test getting MT4 process name
        process_name = get_mt4_process_name()
        self.assertIsInstance(process_name, str)
        self.assertIn(process_name, ['terminal.exe', 'terminal'])
        
        # Test process killing (won't actually kill anything in test)
        # Just verify function exists and can be called
        try:
            result = kill_process_by_name("nonexistent_process_12345")
            self.assertIsInstance(result, int)
            self.assertEqual(result, 0)  # No processes found
        except Exception as e:
            self.fail(f"Process manager failed: {e}")
    
    def test_retry_mechanism_integration(self):
        """Test retry mechanism integration"""
        from include.retry_mechanism import (
            retry_file_operation,
            retry_email_send,
            RetryConfig,
            graceful_degradation
        )
        
        # Test retry config
        config = RetryConfig(max_attempts=3, initial_delay=1.0)
        self.assertEqual(config.max_attempts, 3)
        self.assertEqual(config.initial_delay, 1.0)
        
        # Test graceful degradation
        def primary():
            return "primary"
        
        def fallback():
            return "fallback"
        
        result = graceful_degradation(primary, fallback)
        self.assertEqual(result, "primary")
        
        # Test with failing primary
        def failing_primary():
            raise Exception("Failed")
        
        result = graceful_degradation(failing_primary, fallback)
        self.assertEqual(result, "fallback")
    
    def test_pathlib_integration(self):
        """Test pathlib integration in checker"""
        from pathlib import Path
        from include.config_manager import load_config_with_defaults
        
        # Test that config manager uses Path
        config_path = Path(self.test_config)
        self.assertTrue(config_path.exists())
        
        config = load_config_with_defaults(str(config_path))
        self.assertIsNotNone(config)
    
    def test_environment_variables_integration(self):
        """Test environment variable integration"""
        import os
        from include.config_manager import load_config_with_defaults
        
        # Test that environment variables are checked
        # (This is tested in test_security.py, but verify integration)
        config = load_config_with_defaults(self.test_config)
        
        # Config should load even without env vars (falls back to config file)
        self.assertIsNotNone(config)
    
    def test_error_handling_integration(self):
        """Test error handling across modules"""
        from include.config_manager import load_config_with_defaults
        from include.platform_utils import get_platform_error_message
        
        # Test error message generation
        try:
            raise ValueError("Test error")
        except Exception as e:
            error_msg = get_platform_error_message("test operation", e)
            self.assertIn("test operation", error_msg)
            self.assertIn("Test error", error_msg)
            self.assertIn("suggestions", error_msg.lower())
    
    def test_version_module_integration(self):
        """Test version module integration"""
        from include.version import (
            get_version,
            get_version_info,
            get_version_string,
            get_full_version_info
        )
        
        # Test version functions
        version = get_version()
        self.assertEqual(version, "0.05")
        
        version_info = get_version_info()
        self.assertEqual(version_info, (0, 5, 0))
        
        version_string = get_version_string()
        self.assertIn("Asirikuy Monitor", version_string)
        self.assertIn("0.05", version_string)
        
        full_info = get_full_version_info()
        self.assertIsInstance(full_info, dict)
        self.assertEqual(full_info['version'], "0.05")
        self.assertEqual(full_info['version_info'], (0, 5, 0))


def run_integration_tests():
    """Run all integration tests"""
    print("=" * 60)
    print("Asirikuy Monitor - Integration Tests")
    print("=" * 60)
    print()
    
    # Run unittest tests
    loader = unittest.TestLoader()
    suite = loader.loadTestsFromTestCase(TestCheckerIntegration)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    print()
    print("=" * 60)
    print("Integration Test Summary")
    print("=" * 60)
    print(f"Tests run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    
    if result.wasSuccessful():
        print("\n✓ All integration tests passed!")
        return 0
    else:
        print(f"\n✗ {len(result.failures) + len(result.errors)} test(s) failed")
        return 1


if __name__ == "__main__":
    sys.exit(run_integration_tests())
