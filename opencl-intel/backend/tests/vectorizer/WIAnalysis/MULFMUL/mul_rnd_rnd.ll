; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s



; ModuleID = 'mul_rnd_rnd.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
bb.nph:
  %0 = tail call i32 @get_global_id(i32 0) nounwind
  %1 = getelementptr inbounds i32 addrspace(1)* %in, i32 %0
  %2 = load i32 addrspace(1)* %1, align 4
  %3 = add nsw i32 %0, 7
  %4 = getelementptr inbounds i32 addrspace(1)* %in, i32 %3
  %5 = load i32 addrspace(1)* %4, align 4
  br label %6

; <label>:6                                       ; preds = %6, %bb.nph
  %7 = phi i32 [ 0, %bb.nph ], [ %10, %6 ]
  %x.11 = phi i32 [ %2, %bb.nph ], [ %.x.1, %6 ]
  %8 = icmp sgt i32 %7, 300
  %9 = zext i1 %8 to i32
  %.x.1 = shl i32 %x.11, %9
  %10 = add nsw i32 %7, 1
  %exitcond = icmp eq i32 %10, 2000
  br i1 %exitcond, label %._crit_edge, label %6

._crit_edge:                                      ; preds = %6
  %11 = mul nsw i32 %.x.1, %5
  %12 = getelementptr inbounds i32 addrspace(1)* %out, i32 %0
  store i32 %11, i32 addrspace(1)* %12, align 4
  %13 = add nsw i32 %0, 12
  %14 = mul nsw i32 %11, %13
  %15 = add nsw i32 %0, 10
  %16 = getelementptr inbounds i32 addrspace(1)* %out, i32 %15
  store i32 %14, i32 addrspace(1)* %16, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}


;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = mul
;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = mul
;CHECK: ret