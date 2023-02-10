; RUN: SATest -VAL --tsize=16 --config=%s.cfg -llvm-option=-vplan-enable-peeling | FileCheck %s

; CHECK: Test Passed.
