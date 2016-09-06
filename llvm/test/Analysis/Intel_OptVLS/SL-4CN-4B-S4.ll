; Test whether getSequence() generates four contiguous loads for the four
; gathers in this test, which seems to be the optimized load sequence.
; This test needs to be adjusted if better load sequence is found.
;
; REQUIRES: asserts
; RUN: intelovls-test < %s 2>&1 | FileCheck %s
;
; CHECK: mask.load.32.4
; CHECK: mask.load.32.4
; CHECK: mask.load.32.4
; CHECK: mask.load.32.4
; CHECK-NOT: mask.load.32.4
# 16
1 A 0 i32 4 SLoad C 16
2 A 4 i32 4 SLoad C 16
3 A 8 i32 4 SLoad C 16
4 A 12 i32 4 SLoad C 16