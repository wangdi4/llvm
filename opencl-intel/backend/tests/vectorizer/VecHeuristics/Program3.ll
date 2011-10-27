; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/runtime.bc -vecHeuristics -vec-hue-hasavx -print-vec-hue %t.bc -S -o %t1.ll | grep 4

declare void @func2(i32) nounwind 

define void @recalculateEigenIntervals() nounwind {
BB0:
   call void @func2(i32 0) nounwind
   ret void
}
