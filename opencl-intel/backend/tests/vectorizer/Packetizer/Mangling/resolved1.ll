; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = '/tmp/webcompile/_27953_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: no_mask_ptr_conseq
;CHECK: load <4 x i32>
;CHECK: store <4 x i32>
;CHECK: ret void

define void @no_mask_ptr_conseq(i32* nocapture %A) nounwind {
  %1 = tail call i32 (...)* @get_global_id(i32 0) nounwind
  %2 = sext i32 %1 to i64
  %3 = getelementptr inbounds i32* %A, i64 %2
  %4 = load i32* %3, align 4, !tbaa !0
  %5 = add nsw i32 %4, 3
  store i32 %5, i32* %3, align 4, !tbaa !0
  ret void
}

declare i32 @get_global_id(...)

!0 = metadata !{metadata !"int", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}