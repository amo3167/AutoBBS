#!/usr/bin/env python3
"""
T060: End-to-End Testing for CTester

This script performs comprehensive end-to-end testing of the CTester system,
verifying the complete workflow from configuration to results.

Test Coverage:
1. Configuration loading and validation
2. Library loading and initialization
3. Data preparation and validation
4. Strategy execution (single and portfolio)
5. Results processing and validation
6. Output file generation and verification
"""

import os
import sys
import subprocess
import time
import json
import ctypes
from pathlib import Path
from datetime import datetime

# Add ctester to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

class E2ETestRunner:
    """End-to-end test runner for CTester"""
    
    def __init__(self):
        self.test_results = []
        self.start_time = datetime.now()
        self.ctester_dir = Path(__file__).parent
        self.config_dir = self.ctester_dir / "config" / "diagnostics"
        self.output_dir = self.ctester_dir
        self.library_path = None
        
    def log(self, message, level="INFO"):
        """Log test message"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{timestamp}] [{level}] {message}")
        
    def test_config_loading(self):
        """Test 1: Configuration Loading"""
        self.log("=" * 60)
        self.log("Test 1: Configuration Loading", "TEST")
        self.log("=" * 60)
        
        results = {
            "test": "Configuration Loading",
            "status": "PASS",
            "details": []
        }
        
        try:
            # Check if diagnostic config files exist
            config_files = list(self.config_dir.glob("ast_diagnostic*.config"))
            if not config_files:
                results["status"] = "FAIL"
                results["details"].append("No diagnostic config files found")
                self.log("❌ No diagnostic config files found", "ERROR")
            else:
                results["details"].append(f"Found {len(config_files)} config files")
                self.log(f"✅ Found {len(config_files)} config files")
                
                # Try to parse a config file
                import configparser
                test_config = configparser.ConfigParser()
                test_config.read(config_files[0])
                
                required_sections = ["strategy", "misc", "account"]
                for section in required_sections:
                    if test_config.has_section(section):
                        results["details"].append(f"Section '{section}' found")
                        self.log(f"✅ Config section '{section}' found")
                    else:
                        results["status"] = "FAIL"
                        results["details"].append(f"Section '{section}' missing")
                        self.log(f"❌ Config section '{section}' missing", "ERROR")
                        
        except Exception as e:
            results["status"] = "FAIL"
            results["details"].append(f"Error: {str(e)}")
            self.log(f"❌ Configuration loading failed: {e}", "ERROR")
            
        self.test_results.append(results)
        return results["status"] == "PASS"
    
    def test_library_loading(self):
        """Test 2: Library Loading and Initialization"""
        self.log("=" * 60)
        self.log("Test 2: Library Loading and Initialization", "TEST")
        self.log("=" * 60)
        
        results = {
            "test": "Library Loading",
            "status": "PASS",
            "details": []
        }
        
        try:
            # Set up library paths for macOS
            import os
            import platform
            if platform.system() == "Darwin":
                lib_paths = [
                    "/Users/andym/projects/AutoBBS/bin/gmake/x64/Debug",
                    "/Users/andym/projects/AutoBBS/bin/gmake/x64/Debug/lib",
                    "/Users/andym/projects/AutoBBS/vendor/MiniXML",
                ]
                current_dyld = os.environ.get("DYLD_LIBRARY_PATH", "")
                os.environ["DYLD_LIBRARY_PATH"] = ":".join(lib_paths + ([current_dyld] if current_dyld else []))
                results["details"].append(f"Set DYLD_LIBRARY_PATH: {os.environ['DYLD_LIBRARY_PATH']}")
                self.log(f"✅ Set DYLD_LIBRARY_PATH for library loading")
            
            # Check if library exists
            from include.asirikuy import loadLibrary
            
            system = platform.system()
            if system == "Darwin":
                lib_name = "libCTesterFrameworkAPI.dylib"
            elif system == "Linux":
                lib_name = "libCTesterFrameworkAPI.so"
            else:
                lib_name = "CTesterFrameworkAPI.dll"
                
            # Try to load library
            astdll = loadLibrary(lib_name)
            if astdll:
                results["details"].append(f"Library loaded: {lib_name}")
                self.log(f"✅ Library loaded: {lib_name}")
                self.library_path = lib_name
                
                # Test framework initialization
                init_func = astdll.initCTesterFramework
                init_func.argtypes = [ctypes.c_char_p, ctypes.c_int]
                init_func.restype = ctypes.c_int
                
                log_path = str(self.ctester_dir / "log" / "test_e2e.log").encode('utf-8')
                result = init_func(log_path, 6)
                
                if result == 0:
                    results["details"].append("Framework initialization successful")
                    self.log("✅ Framework initialization successful")
                else:
                    results["status"] = "FAIL"
                    results["details"].append(f"Framework initialization failed: {result}")
                    self.log(f"❌ Framework initialization failed: {result}", "ERROR")
                    
                # Test version function
                version_func = astdll.getCTesterFrameworkVersion
                version_func.argtypes = [ctypes.POINTER(ctypes.c_int), 
                                         ctypes.POINTER(ctypes.c_int),
                                         ctypes.POINTER(ctypes.c_int)]
                version_func.restype = None
                
                major = ctypes.c_int()
                minor = ctypes.c_int()
                bugfix = ctypes.c_int()
                version_func(ctypes.byref(major), ctypes.byref(minor), ctypes.byref(bugfix))
                
                version_str = f"{major.value}.{minor.value}.{bugfix.value}"
                results["details"].append(f"Framework version: {version_str}")
                self.log(f"✅ Framework version: {version_str}")
                
            else:
                results["status"] = "FAIL"
                results["details"].append(f"Failed to load library: {lib_name}")
                self.log(f"❌ Failed to load library: {lib_name}", "ERROR")
                
        except Exception as e:
            results["status"] = "FAIL"
            results["details"].append(f"Error: {str(e)}")
            self.log(f"❌ Library loading failed: {e}", "ERROR")
            import traceback
            traceback.print_exc()
            
        self.test_results.append(results)
        return results["status"] == "PASS"
    
    def test_data_preparation(self):
        """Test 3: Data Preparation"""
        self.log("=" * 60)
        self.log("Test 3: Data Preparation", "TEST")
        self.log("=" * 60)
        
        results = {
            "test": "Data Preparation",
            "status": "PASS",
            "details": []
        }
        
        try:
            # Check if history directory exists
            history_dir = self.ctester_dir / "history"
            if not history_dir.exists():
                results["status"] = "FAIL"
                results["details"].append("History directory not found")
                self.log("❌ History directory not found", "ERROR")
            else:
                results["details"].append("History directory found")
                self.log("✅ History directory found")
                
                # Check for CSV files
                csv_files = list(history_dir.glob("*.csv"))
                if csv_files:
                    results["details"].append(f"Found {len(csv_files)} CSV files")
                    self.log(f"✅ Found {len(csv_files)} CSV files")
                else:
                    results["status"] = "WARN"
                    results["details"].append("No CSV files found (tests may fail)")
                    self.log("⚠️  No CSV files found (tests may fail)", "WARN")
                    
        except Exception as e:
            results["status"] = "FAIL"
            results["details"].append(f"Error: {str(e)}")
            self.log(f"❌ Data preparation test failed: {e}", "ERROR")
            
        self.test_results.append(results)
        return results["status"] in ["PASS", "WARN"]
    
    def test_strategy_execution(self, test_number=1):
        """Test 4: Strategy Execution"""
        self.log("=" * 60)
        self.log(f"Test 4: Strategy Execution (Diagnostic {test_number})", "TEST")
        self.log("=" * 60)
        
        results = {
            "test": f"Strategy Execution (Diagnostic {test_number})",
            "status": "PASS",
            "details": [],
            "execution_time": None
        }
        
        try:
            config_file = self.config_dir / f"ast_diagnostic{test_number}.config"
            if not config_file.exists():
                results["status"] = "SKIP"
                results["details"].append(f"Config file not found: {config_file}")
                self.log(f"⏭️  Config file not found: {config_file}", "SKIP")
                self.test_results.append(results)
                return False
                
            # Run the test
            start_time = time.time()
            output_name = f"e2e_test_{test_number}"
            
            cmd = [
                sys.executable,
                str(self.ctester_dir / "asirikuy_strategy_tester.py"),
                "-ot", output_name,
                "-c", str(config_file)
            ]
            
            self.log(f"Executing: {' '.join(cmd)}")
            result = subprocess.run(
                cmd,
                cwd=str(self.ctester_dir),
                capture_output=True,
                text=True,
                timeout=300  # 5 minute timeout
            )
            
            execution_time = time.time() - start_time
            results["execution_time"] = f"{execution_time:.2f}s"
            
            if result.returncode == 0:
                results["details"].append(f"Test executed successfully in {execution_time:.2f}s")
                self.log(f"✅ Test executed successfully in {execution_time:.2f}s")
                
                # Debug: Check stdout/stderr for file creation info
                if result.stdout:
                    stdout_preview = result.stdout[-500:] if len(result.stdout) > 500 else result.stdout
                    results["details"].append(f"STDOUT: {stdout_preview}")
                    self.log(f"STDOUT: {stdout_preview[:300]}")
                if result.stderr:
                    stderr_preview = result.stderr[-500:] if len(result.stderr) > 500 else result.stderr
                    results["details"].append(f"STDERR: {stderr_preview}")
                    self.log(f"STDERR: {stderr_preview[:300]}")
                
                # Check for output files (in ctester_dir where the script runs)
                output_txt = self.ctester_dir / f"{output_name}.txt"
                output_xml = self.ctester_dir / f"{output_name}.xml"
                
                if output_txt.exists():
                    results["details"].append(f"Output file created: {output_txt.name}")
                    self.log(f"✅ Output file created: {output_txt.name}")
                else:
                    results["status"] = "WARN"
                    results["details"].append(f"Output file not found: {output_txt.name}")
                    self.log(f"⚠️  Output file not found: {output_txt.name}", "WARN")
                    
            else:
                results["status"] = "FAIL"
                results["details"].append(f"Test failed with return code {result.returncode}")
                results["details"].append(f"STDOUT: {result.stdout[-500:]}")
                results["details"].append(f"STDERR: {result.stderr[-500:]}")
                self.log(f"❌ Test failed with return code {result.returncode}", "ERROR")
                if result.stderr:
                    self.log(f"Error output: {result.stderr[-200:]}", "ERROR")
                    
        except subprocess.TimeoutExpired:
            results["status"] = "FAIL"
            results["details"].append("Test timed out after 5 minutes")
            self.log("❌ Test timed out after 5 minutes", "ERROR")
        except Exception as e:
            results["status"] = "FAIL"
            results["details"].append(f"Error: {str(e)}")
            self.log(f"❌ Strategy execution failed: {e}", "ERROR")
            import traceback
            traceback.print_exc()
            
        self.test_results.append(results)
        return results["status"] == "PASS"
    
    def test_output_validation(self, test_number=1):
        """Test 5: Output Validation"""
        self.log("=" * 60)
        self.log(f"Test 5: Output Validation (Test {test_number})", "TEST")
        self.log("=" * 60)
        
        results = {
            "test": f"Output Validation (Test {test_number})",
            "status": "PASS",
            "details": []
        }
        
        try:
            output_name = f"e2e_test_{test_number}"
            # Check in current directory first, then ctester_dir
            output_txt = Path(f"{output_name}.txt")
            if not output_txt.exists():
                output_txt = self.ctester_dir / f"{output_name}.txt"
            output_xml = Path(f"{output_name}.xml")
            if not output_xml.exists():
                output_xml = self.ctester_dir / f"{output_name}.xml"
            
            # Validate text output
            if output_txt.exists():
                file_size = output_txt.stat().st_size
                results["details"].append(f"Text output size: {file_size} bytes")
                self.log(f"✅ Text output file exists ({file_size} bytes)")
                
                # Check if file has content
                if file_size > 0:
                    with open(output_txt, 'r') as f:
                        first_line = f.readline()
                        results["details"].append(f"First line: {first_line[:50]}...")
                else:
                    results["status"] = "WARN"
                    results["details"].append("Text output file is empty")
                    self.log("⚠️  Text output file is empty", "WARN")
            else:
                results["status"] = "FAIL"
                results["details"].append("Text output file not found")
                self.log("❌ Text output file not found", "ERROR")
                
            # Validate XML output (if exists)
            if output_xml.exists():
                file_size = output_xml.stat().st_size
                results["details"].append(f"XML output size: {file_size} bytes")
                self.log(f"✅ XML output file exists ({file_size} bytes)")
                
                # Try to parse XML
                try:
                    import xml.etree.ElementTree as ET
                    tree = ET.parse(output_xml)
                    root = tree.getroot()
                    results["details"].append(f"XML root element: {root.tag}")
                    self.log(f"✅ XML is valid (root: {root.tag})")
                except Exception as e:
                    results["status"] = "WARN"
                    results["details"].append(f"XML parsing error: {str(e)}")
                    self.log(f"⚠️  XML parsing error: {e}", "WARN")
            else:
                results["status"] = "WARN"
                results["details"].append("XML output file not found (may be optional)")
                self.log("⚠️  XML output file not found (may be optional)", "WARN")
                
        except Exception as e:
            results["status"] = "FAIL"
            results["details"].append(f"Error: {str(e)}")
            self.log(f"❌ Output validation failed: {e}", "ERROR")
            
        self.test_results.append(results)
        return results["status"] in ["PASS", "WARN"]
    
    def generate_report(self):
        """Generate E2E test report"""
        self.log("=" * 60)
        self.log("Generating E2E Test Report", "REPORT")
        self.log("=" * 60)
        
        total_tests = len(self.test_results)
        passed = sum(1 for r in self.test_results if r["status"] == "PASS")
        failed = sum(1 for r in self.test_results if r["status"] == "FAIL")
        warnings = sum(1 for r in self.test_results if r["status"] == "WARN")
        skipped = sum(1 for r in self.test_results if r["status"] == "SKIP")
        
        end_time = datetime.now()
        duration = (end_time - self.start_time).total_seconds()
        
        report = {
            "test_suite": "T060: End-to-End Testing for CTester",
            "date": self.start_time.isoformat(),
            "duration_seconds": duration,
            "summary": {
                "total": total_tests,
                "passed": passed,
                "failed": failed,
                "warnings": warnings,
                "skipped": skipped
            },
            "results": self.test_results
        }
        
        # Save JSON report
        report_file = self.ctester_dir / "docs" / "CTESTER_E2E_TEST_REPORT.json"
        report_file.parent.mkdir(parents=True, exist_ok=True)
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
        self.log(f"✅ JSON report saved: {report_file}")
        
        # Print summary
        print("\n" + "=" * 60)
        print("E2E TEST SUMMARY")
        print("=" * 60)
        print(f"Total Tests: {total_tests}")
        print(f"✅ Passed: {passed}")
        print(f"❌ Failed: {failed}")
        print(f"⚠️  Warnings: {warnings}")
        print(f"⏭️  Skipped: {skipped}")
        print(f"Duration: {duration:.2f}s")
        print("=" * 60)
        
        return report
    
    def run_all_tests(self):
        """Run all end-to-end tests"""
        self.log("Starting End-to-End Testing for CTester", "START")
        self.log(f"Test directory: {self.ctester_dir}")
        self.log(f"Config directory: {self.config_dir}")
        
        # Test 1: Configuration Loading
        if not self.test_config_loading():
            self.log("Configuration loading failed, aborting", "ERROR")
            return False
            
        # Test 2: Library Loading
        if not self.test_library_loading():
            self.log("Library loading failed, aborting", "ERROR")
            return False
            
        # Test 3: Data Preparation
        self.test_data_preparation()
        
        # Test 4: Strategy Execution (run diagnostic 1 as E2E test)
        self.test_strategy_execution(test_number=1)
        
        # Test 5: Output Validation
        self.test_output_validation(test_number=1)
        
        # Generate report
        report = self.generate_report()
        
        # Overall status
        if report["summary"]["failed"] == 0:
            self.log("✅ All critical tests passed!", "SUCCESS")
            return True
        else:
            self.log("❌ Some tests failed", "ERROR")
            return False


def main():
    """Main entry point"""
    runner = E2ETestRunner()
    success = runner.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()

