; RUN: SATest -VAL -force_ref --config=%s.cfg 2>&1 | FileCheck %s

; CHECK: Test Passed.
