; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -subscript  -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; Check masked scatter/gather of chars

;CHECK: kernel
;CHECK: @masked_gather_I8
;CHECK: @masked_scatter_I8
;CHECK: ret void

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

define void @kernel(i8* nocapture %A, i64 %k) nounwind {
  %1 = tail call i32 (...)* @get_global_id(i32 0) nounwind
  %2 = icmp sgt i32 %1, 70
  br i1 %2, label %3, label %9

; <label>:3                                       ; preds = %0
  %4 = shl i32 %1, 1
  %5 = sext i32 %4 to i64
  %6 = getelementptr inbounds i8* %A, i64 %5
  %7 = load i8* %6, align 1, !tbaa !0
  %8 = add i8 %7, 3
  store i8 %8, i8* %6, align 1, !tbaa !0
  br label %9

; <label>:9                                       ; preds = %3, %0
  ret void
}

declare i32 @get_global_id(...)

!0 = metadata !{metadata !"omnipotent char", metadata !1}
!1 = metadata !{metadata !"Simple C/C++ TBAA", null}
