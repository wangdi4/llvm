; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

define <16 x i32> @B(<16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; CHECK: @B
; CHECK-NOT: vpermd
; CHECK: ret
  %m = shufflevector <16 x i32> %a, <16 x i32> %b, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 15, i32 14, i32 13, i32 12, i32 17, i32 undef, i32 1, i32 7, i32 0, i32 9, i32 undef, i32 undef>
  ret <16 x i32> %m
}
