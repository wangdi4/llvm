; RUN: SATest -VAL -config=%s.16i8.cfg | FileCheck %s
; RUN: SATest -VAL -config=%s.8i16.cfg | FileCheck %s
; RUN: SATest -VAL -config=%s.4i32.cfg | FileCheck %s
; RUN: SATest -VAL -config=%s.2i64.cfg | FileCheck %s

;CHECK: Test Passed.
