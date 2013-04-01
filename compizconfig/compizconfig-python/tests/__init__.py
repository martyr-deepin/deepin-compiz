import unittest
import compizconfig
import os

def load_tests (loader, tests, pattern):
    tests =  loader.discover (os.getcwd (), pattern='test_*.py')
    return tests

if __name__ == "__main__":
    loader = unittest.defaultTestLoader
    unittest.main (testLoader=loader)
