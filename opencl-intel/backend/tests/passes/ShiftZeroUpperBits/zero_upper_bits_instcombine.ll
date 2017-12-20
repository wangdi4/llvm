; RUN: %oclopt -shift-ignore-upper-bits -instcombine -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define <4 x i32> @sample_test(<4 x i32> %x) {
entry:
    %temp1 = lshr <4 x i32> %x, <i32 63, i32 0, i32 1, i32 2>
    %temp2 = shl <4 x i32> %temp1, <i32 31, i32 32, i32 33, i32 34>
    ret <4 x i32> %temp2
}

; CHECK:        entry:
; CHECK-NOT:    and
; CHECK:        ret <4 x i32> %temp2

