; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/runtime.bc -vecHeuristics -print-vec-hue %t.bc -S -o %t1.ll | not grep 4
; RUN: opt -runtimelib %p/../Full/runtime.bc -vecHeuristics -vec-hue-hasavx -print-vec-hue %t.bc -S -o %t1.ll | not grep 8


declare i32 @get_global_id(i32)
  
define i32 @func2(float* nocapture %A, i32 %Z) nounwind {
entry:
  %0 = tail call i32 @get_global_id(i32 %Z)        ; <i32> [#uses=1]
  %1 = icmp sgt i32 %0, 3                         ; <i1> [#uses=1]
  br i1 %1, label %bb, label %return

bb:                                               ; preds = %entry
  %2 = getelementptr inbounds float* %A, i64 6    ; <float*> [#uses=1]
  store float 8.000000e+00, float* %2, align 4
  ret i32 undef

return:                                           ; preds = %entry
  ret i32 undef
}

