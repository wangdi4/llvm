; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s



; ModuleID = 'mul_un_un.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
bb.nph:
  %0 = tail call i32 @get_global_id(i32 0) nounwind
  br label %1

; <label>:1                                       ; preds = %1, %bb.nph
  %2 = phi i32 [ 0, %bb.nph ], [ %6, %1 ]
  %y.12 = phi i32 [ 0, %bb.nph ], [ %y.0, %1 ]
  %x.11 = phi i32 [ 1, %bb.nph ], [ %x.0, %1 ]
  %3 = icmp sgt i32 %2, 300
  %4 = zext i1 %3 to i32
  %x.0 = shl i32 %x.11, %4
  %5 = xor i32 %4, 1
  %y.0 = add i32 %5, %y.12
  %6 = add nsw i32 %2, 1
  %exitcond = icmp eq i32 %6, 2000
  br i1 %exitcond, label %._crit_edge, label %1

._crit_edge:                                      ; preds = %1
  %7 = mul nsw i32 %y.0, %x.0
  %8 = getelementptr inbounds i32 addrspace(1)* %out, i32 %0
  store i32 %7, i32 addrspace(1)* %8, align 4
  %9 = add nsw i32 %7, 100
  %10 = add nsw i32 %0, 10
  %11 = getelementptr inbounds i32 addrspace(1)* %out, i32 %10
  store i32 %9, i32 addrspace(1)* %11, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}

;CHECK: WIA 0  %{{[a-z]*}}{{[0-9]*}} = mul
;CHECK: ret