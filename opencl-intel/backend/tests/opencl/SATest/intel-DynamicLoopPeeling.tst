; RUN: SATest -VAL --tsize=16 --config=%s.cfg -llvm-option=-vplan-enable-peeling 2>&1 | FileCheck %s

; CHECK: Test Passed.
