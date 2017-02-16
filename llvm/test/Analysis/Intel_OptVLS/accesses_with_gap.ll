; Tests if following three accesses with gaps are grouped together.
; Vector Length:16 Group#1(1, 2, 3)
; REQUIRES: asserts
; RUN: intelovls-test < %s -debug 2>&1 | FileCheck %s
;
; CHECK: Group#
; CHECK-NOT: Group#
# 16
1 A 0 i32 4 SLoad C 40
2 A 12 i32 4 SLoad C 40
3 A 4 i32 4 SLoad C 40