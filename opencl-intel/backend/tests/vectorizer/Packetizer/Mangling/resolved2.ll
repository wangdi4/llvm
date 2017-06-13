; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -O3 -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = '/tmp/webcompile/_27953_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: mask_ptr_conseq
;CHECK: load i32, i32*
;CHECK: load i32, i32*
;CHECK: load i32, i32*
;CHECK: load i32, i32*

;CHECK: store i32
;CHECK: store i32
;CHECK: store i32
;CHECK: store i32
;CHECK: ret void

; ModuleID = '/tmp/webcompile/_29773_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

define void @mask_ptr_conseq(i32* nocapture %A) nounwind {
  %1 = tail call i32 (...) @_Z13get_global_idj(i32 0) nounwind
  %2 = icmp ugt i32 %1, 10
  br i1 %2, label %3, label %8

; <label>:3                                       ; preds = %0
  %4 = zext i32 %1 to i64
  %5 = getelementptr inbounds i32, i32* %A, i64 %4
  %6 = load i32, i32* %5, align 4, !tbaa !0
  %7 = add nsw i32 %6, 3
  store i32 %7, i32* %5, align 4, !tbaa !0
  br label %8

; <label>:8                                       ; preds = %3, %0
  ret void
}

declare i32 @_Z13get_global_idj(...)

!0 = !{!"int", !1}
!1 = !{!"omnipotent char", !2}
!2 = !{!"Simple C/C++ TBAA"}
