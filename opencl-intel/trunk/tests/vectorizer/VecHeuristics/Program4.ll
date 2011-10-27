; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/runtime.bc -vecHeuristics -print-vec-hue %t.bc -S -o %t1.ll | grep 4
; RUN: opt -runtimelib %p/../Full/runtime.bc -vecHeuristics -vec-hue-hasavx -print-vec-hue %t.bc -S -o %t1.ll | grep 4



define i17 @func(i17 %T) nounwind {
BB0:
  %A = add i17 1, %T 
  ret i17 %A
}

