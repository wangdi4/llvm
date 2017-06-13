; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -O3 -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -gather-scatter -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; No Mask because the index is 64bit
; ModuleID = '/tmp/webcompile/_2701_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: kernel
;CHECK-NOT: @gather
;CHECK-NOT: @scatter
;CHECK: ret void

define void @kernel(i32* nocapture %A, i64 %k) nounwind {
  %1 = tail call i32 (...) @_Z13get_global_idj(i32 0) nounwind
  %2 = icmp sgt i32 %1, 70
  br i1 %2, label %3, label %9

; <label>:3                                       ; preds = %0
  %4 = sext i32 %1 to i64
  %5 = mul nsw i64 %4, %k
  %6 = getelementptr inbounds i32, i32* %A, i64 %5
  %7 = load i32, i32* %6, align 4, !tbaa !0
  %8 = add nsw i32 %7, 3
  store i32 %8, i32* %6, align 4, !tbaa !0
  br label %9

; <label>:9                                       ; preds = %3, %0
  ret void
}

declare i32 @_Z13get_global_idj(...)

!0 = !{!"int", !1}
!1 = !{!"omnipotent char", !2}
!2 = !{!"Simple C/C++ TBAA"}
