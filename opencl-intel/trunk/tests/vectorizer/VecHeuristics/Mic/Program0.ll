; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -vecHeuristics -print-vec-hue -vec-hue-ismic %t.bc -S -o %t1.ll | FileCheck %s
; CHECK: Should vectorize to 16
; ModuleID = 'ev.ll'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @get_global_id(i32)
define i32 @func2(float* nocapture %A) nounwind {
entry:
  %0 = tail call i32 @get_global_id(i32 0)        ; <i32> [#uses=1]
  %1 = icmp sgt i32 %0, 3                         ; <i1> [#uses=1]
  br i1 %1, label %bb, label %return

bb:                                               ; preds = %entry
  %2 = getelementptr inbounds float* %A, i64 6    ; <float*> [#uses=1]
  store float 8.000000e+00, float* %2, align 4
  ret i32 undef

return:                                           ; preds = %entry
  ret i32 undef
}

