#!/usr/bin/python

# This is a stub for manual valgrind test runs.
import sys
import os

srcDir = "@CMAKE_CURRENT_SOURCE_DIR@"

sys.path.insert(0, srcDir)
import testAndDiff
testAndDiff.srcDir = srcDir
testAndDiff.pwd = os.getcwd()
testAndDiff.batchMode = False

testAndDiff.main()
