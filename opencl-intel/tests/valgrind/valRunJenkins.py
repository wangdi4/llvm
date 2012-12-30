#!/usr/bin/python

# This is an empty stub for batch (Jenkins) valgrind test runs, where the sources directory may be unreachable.
# Using this wrapper you cannot updated, nor create new tests.
import sys
import os

srcDir = "@VALTEST_JENKINS_DIR@"

sys.path.insert(0, srcDir)
import testAndDiff
testAndDiff.srcDir = srcDir
testAndDiff.pwd = os.getcwd()
testAndDiff.batchMode = True

testAndDiff.main()

