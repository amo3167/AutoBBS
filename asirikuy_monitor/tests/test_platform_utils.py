"""
Tests for platform detection and utilities
"""

import sys
import os
import unittest
import platform

# Add parent directory to path for imports
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from include.platform_utils import (
    detect_platform, get_platform_name, get_platform_info,
    is_windows, is_macos, is_linux, is_unix,
    get_mt4_process_name, get_platform_error_message,
    get_path_separator, get_line_separator, format_platform_path,
    PlatformType
)

class TestPlatformUtils(unittest.TestCase):
    
    def test_detect_platform(self):
        """Test platform detection"""
        platform_type = detect_platform()
        self.assertIsInstance(platform_type, PlatformType)
        self.assertIn(platform_type, [PlatformType.WINDOWS, PlatformType.MACOS, PlatformType.LINUX, PlatformType.UNKNOWN])
    
    def test_get_platform_name(self):
        """Test getting platform name"""
        name = get_platform_name()
        self.assertIsInstance(name, str)
        self.assertIn(name.lower(), ['windows', 'macos', 'linux', 'unknown'])
    
    def test_platform_checks(self):
        """Test platform check functions"""
        # At least one should be True (unless UNKNOWN)
        platform_type = detect_platform()
        if platform_type != PlatformType.UNKNOWN:
            checks = [is_windows(), is_macos(), is_linux()]
            self.assertTrue(any(checks), "At least one platform check should be True")
    
    def test_is_unix(self):
        """Test Unix detection"""
        unix = is_unix()
        self.assertIsInstance(unix, bool)
        # Unix should be True if macOS or Linux
        if is_macos() or is_linux():
            self.assertTrue(unix)
        elif is_windows():
            self.assertFalse(unix)
    
    def test_get_mt4_process_name(self):
        """Test MT4 process name detection"""
        process_name = get_mt4_process_name()
        self.assertIsInstance(process_name, str)
        
        if is_windows():
            self.assertEqual(process_name, 'terminal.exe')
        else:
            self.assertEqual(process_name, 'terminal')
    
    def test_get_platform_info(self):
        """Test getting platform information"""
        info = get_platform_info()
        
        self.assertIsInstance(info, dict)
        self.assertIn('platform', info)
        self.assertIn('system', info)
        self.assertIn('release', info)
        self.assertIn('version', info)
        self.assertIn('machine', info)
        
        # Platform should match detected platform
        self.assertEqual(info['platform'], detect_platform().value)
        self.assertEqual(info['system'], platform.system())
    
    def test_get_platform_error_message(self):
        """Test platform-specific error messages"""
        test_error = ValueError("Test error message")
        error_msg = get_platform_error_message("test operation", test_error)
        
        self.assertIsInstance(error_msg, str)
        self.assertIn("test operation", error_msg)
        self.assertIn("Test error message", error_msg)
        
        # Should contain platform-specific suggestions
        if is_windows():
            self.assertIn("administrator", error_msg.lower())
        elif is_macos():
            self.assertIn("permissions", error_msg.lower())
        elif is_linux():
            self.assertIn("permissions", error_msg.lower())
    
    def test_get_path_separator(self):
        """Test path separator detection"""
        separator = get_path_separator()
        self.assertIsInstance(separator, str)
        
        if is_windows():
            self.assertEqual(separator, '\\')
        else:
            self.assertEqual(separator, '/')
    
    def test_get_line_separator(self):
        """Test line separator detection"""
        separator = get_line_separator()
        self.assertIsInstance(separator, str)
        self.assertIn(separator, ['\n', '\r\n', '\r'])
    
    def test_format_platform_path(self):
        """Test path formatting"""
        # Test forward slash path
        path1 = format_platform_path("path/to/file")
        if is_windows():
            self.assertEqual(path1, "path\\to\\file")
        else:
            self.assertEqual(path1, "path/to/file")
        
        # Test backslash path
        path2 = format_platform_path("path\\to\\file")
        if is_windows():
            self.assertEqual(path2, "path\\to\\file")
        else:
            self.assertEqual(path2, "path/to/file")
    
    def test_platform_consistency(self):
        """Test that platform detection is consistent"""
        platform_type = detect_platform()
        platform_name = get_platform_name().lower()
        
        # Platform name should match platform type
        if platform_type == PlatformType.WINDOWS:
            self.assertEqual(platform_name, 'windows')
            self.assertTrue(is_windows())
            self.assertFalse(is_macos())
            self.assertFalse(is_linux())
        elif platform_type == PlatformType.MACOS:
            self.assertEqual(platform_name, 'macos')
            self.assertFalse(is_windows())
            self.assertTrue(is_macos())
            self.assertFalse(is_linux())
        elif platform_type == PlatformType.LINUX:
            self.assertEqual(platform_name, 'linux')
            self.assertFalse(is_windows())
            self.assertFalse(is_macos())
            self.assertTrue(is_linux())

if __name__ == '__main__':
    print("=" * 60)
    print("Platform Utils Tests")
    print("=" * 60)
    print()
    
    # Show current platform info
    from include.platform_utils import get_platform_info
    info = get_platform_info()
    print(f"Running on: {info['platform']} ({info['system']} {info['release']})")
    print()
    
    unittest.main(verbosity=2)

