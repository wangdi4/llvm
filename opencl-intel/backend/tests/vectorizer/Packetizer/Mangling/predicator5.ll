; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -O3 -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = '/tmp/webcompile/_27953_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: multi
;CHECK: masked_store
;CHECK: ret void
; ModuleID = '/tmp/webcompile/_6340_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

define void @multi(i32* nocapture %A, i32 %f) nounwind {
  %1 = tail call i32 (...) @_Z13get_global_idj(i32 0) nounwind
  %2 = icmp sgt i32 %1, 3
  br i1 %2, label %3, label %13

; <label>:3                                       ; preds = %0
  %4 = load i32, i32* %A, align 4, !tbaa !0
  %5 = add nsw i32 %4, 3
  store i32 %5, i32* %A, align 4, !tbaa !0
  %6 = sext i32 %1 to i64
  %7 = getelementptr inbounds i32, i32* %A, i64 %6
  %8 = load i32, i32* %7, align 4, !tbaa !0
  %9 = add nsw i32 %8, %1
  store i32 %9, i32* %7, align 4, !tbaa !0
  %10 = getelementptr inbounds i32, i32* %A, i64 3
  %11 = load i32, i32* %10, align 4, !tbaa !0
  %12 = add nsw i32 %11, %1
  store i32 %12, i32* %10, align 4, !tbaa !0
  br label %13

; <label>:13                                      ; preds = %3, %0
  %14 = icmp eq i32 %f, 0
  br i1 %14, label %25, label %15

; <label>:15                                      ; preds = %13
  %16 = getelementptr inbounds i32, i32* %A, i64 3
  %17 = load i32, i32* %16, align 4, !tbaa !0
  %18 = add nsw i32 %17, 9
  store i32 %18, i32* %16, align 4, !tbaa !0
  %19 = sext i32 %1 to i64
  %20 = getelementptr inbounds i32, i32* %A, i64 %19
  %21 = load i32, i32* %20, align 4, !tbaa !0
  %22 = add nsw i32 %21, %1
  store i32 %22, i32* %20, align 4, !tbaa !0
  %23 = load i32, i32* %16, align 4, !tbaa !0
  %24 = add nsw i32 %23, %1
  store i32 %24, i32* %16, align 4, !tbaa !0
  br label %25

; <label>:25                                      ; preds = %15, %13
  ret void
}

declare i32 @_Z13get_global_idj(...)

!0 = !{!"int", !1}
!1 = !{!"omnipotent char", !2}
!2 = !{!"Simple C/C++ TBAA"}
