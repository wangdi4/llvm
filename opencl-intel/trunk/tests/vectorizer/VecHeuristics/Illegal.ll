; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/runtime.bc -vecHeuristics -print-vec-hue %t.bc -S -o %t1.ll | not grep 4


define i150 @func(i150 %T) nounwind {
BB0:
  %A = add i150 1, %T 
  ret i150 %A
}

