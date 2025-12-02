# -*- coding: utf-8 -*-

# Fallback implementation using standard csv module for Python 3
# The original _fastcsv C extension needs to be rebuilt for Python 3
import csv
import io

class Reader:
    """Fallback Reader using standard csv module"""
    def __init__(self, file_handle):
        self.file_handle = file_handle
        self.reader = csv.reader(file_handle)
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        if hasattr(self.file_handle, 'close'):
            self.file_handle.close()
        return False
    
    def __iter__(self):
        return self.reader

class Writer:
    """Fallback Writer using standard csv module"""
    def __init__(self, file_handle):
        self.file_handle = file_handle
        self.writer = csv.writer(file_handle)
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        if hasattr(self.file_handle, 'close'):
            self.file_handle.close()
        return False
    
    def writerow(self, row):
        self.writer.writerow(row)

