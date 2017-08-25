; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -O3 -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -gather-scatter -gather-scatter-v4i8 -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; Check masked scatter/gather of chars

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: kernel
;CHECK: @"internal.gather.v4i8[i32].m4"(<4 x i1> [[NAME1:%[0-9]*]], i8* [[NAME2:%stripAS[0-9]*]], <4 x i32> [[NAME4:%[0-9]*]], i32 32, i1 true)
;CHECK: @"internal.scatter.v4i8[i32].m4"(<4 x i1> [[NAME1]], i8* [[NAME3:%stripAS[0-9]*]], <4 x i32> [[NAME4]], <4 x i8> [[NAME5:%[0-9]*]], i32 32, i1 true)
;CHECK: ret void

define void @kernel(i8* nocapture %A, i64 %k) nounwind {
  %1 = tail call i32 (...) @_Z13get_global_idj(i32 0) nounwind
  %2 = icmp sgt i32 %1, 70
  br i1 %2, label %3, label %9

; <label>:3                                       ; preds = %0
  %4 = shl i32 %1, 1
  %5 = sext i32 %4 to i64
  %6 = getelementptr inbounds i8, i8* %A, i64 %5
  %7 = load i8, i8* %6, align 1, !tbaa !0
  %8 = add i8 %7, 3
  store i8 %8, i8* %6, align 1, !tbaa !0
  br label %9

; <label>:9                                       ; preds = %3, %0
  ret void
}

declare i32 @_Z13get_global_idj(...)

!0 = !{!"omnipotent char", !1}
!1 = !{!"Simple C/C++ TBAA"}
