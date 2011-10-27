; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s


; ModuleID = 'mul_uni_str.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
bb.nph:
  %0 = tail call i32 @get_global_id(i32 0) nounwind
  %1 = mul nsw i32 %0, 10
  %2 = add nsw i32 %1, 2
  br label %3

; <label>:3                                       ; preds = %3, %bb.nph
  %4 = phi i32 [ 0, %bb.nph ], [ %7, %3 ]
  %x.11 = phi i32 [ %2, %bb.nph ], [ %.x.1, %3 ]
  %5 = icmp sgt i32 %4, 300
  %6 = zext i1 %5 to i32
  %.x.1 = shl i32 %x.11, %6
  %7 = add nsw i32 %4, 1
  %exitcond = icmp eq i32 %7, 2000
  br i1 %exitcond, label %._crit_edge, label %3

._crit_edge:                                      ; preds = %3
  %8 = mul nsw i32 %.x.1, 12
  %9 = getelementptr inbounds i32 addrspace(1)* %out, i32 %0
  store i32 %8, i32 addrspace(1)* %9, align 4
  %10 = mul nsw i32 %.x.1, 1200
  %11 = add nsw i32 %0, 10
  %12 = getelementptr inbounds i32 addrspace(1)* %out, i32 %11
  store i32 %10, i32 addrspace(1)* %12, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}


;CHECK: WIA 3  %{{[a-z]*}}{{[0-9]*}} = mul
;CHECK: WIA 3  %{{[a-z]*}}{{[0-9]*}} = mul
;CHECK: ret