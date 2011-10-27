; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s



; ModuleID = 'mul_str_str.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
bb.nph:
  %0 = tail call i32 @get_global_id(i32 0) nounwind
  %1 = mul nsw i32 %0, 12
  %2 = mul i32 %0, 6
  %3 = add nsw i32 %2, 21
  br label %4

; <label>:4                                       ; preds = %4, %bb.nph
  %5 = phi i32 [ 0, %bb.nph ], [ %8, %4 ]
  %x.11 = phi i32 [ %1, %bb.nph ], [ %.x.1, %4 ]
  %6 = icmp sgt i32 %5, 300
  %7 = zext i1 %6 to i32
  %.x.1 = shl i32 %x.11, %7
  %8 = add nsw i32 %5, 1
  %exitcond = icmp eq i32 %8, 2000
  br i1 %exitcond, label %._crit_edge, label %4

._crit_edge:                                      ; preds = %4
  %9 = mul nsw i32 %.x.1, %3
  %10 = getelementptr inbounds i32 addrspace(1)* %out, i32 %0
  store i32 %9, i32 addrspace(1)* %10, align 4
  %11 = shl i32 %0, 2
  %12 = add nsw i32 %11, 12
  %13 = mul nsw i32 %9, %12
  %14 = add nsw i32 %0, 10
  %15 = getelementptr inbounds i32 addrspace(1)* %out, i32 %14
  store i32 %13, i32 addrspace(1)* %15, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}

;CHECK: WIA 3  %{{[a-z]*}}{{[0-9]*}} = mul
;CHECK: WIA 3  %{{[a-z]*}}{{[0-9]*}} = mul
;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = mul
;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = mul
;CHECK: ret