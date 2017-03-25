; Tests whether following two heterogeneous typed accesses get grouped together.
; REQUIRES: asserts
; RUN: intelovls-test < %s -debug 2>&1 | FileCheck %s
;
; CHECK: Group#
; CHECK-NOT: Group#

# 16
1 A 0 i64 4 SLoad C 16
2 A 8 i32 4 SLoad C 16