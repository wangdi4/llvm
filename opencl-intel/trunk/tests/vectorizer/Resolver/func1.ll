; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main(i1 %pred) {
; CHECK-NOT: @maskedf
  %f = call float @maskedf_001_printf(i1 %pred)
  ret void
; CHECK: ret
}

declare float @maskedf_001_printf(i1 %pred)


