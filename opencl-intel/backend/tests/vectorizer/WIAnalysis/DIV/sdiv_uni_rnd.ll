; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s


; ModuleID = 'sdiv_uni_rnd.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
bb.nph:
  %0 = tail call i32 @get_global_id(i32 0) nounwind
  %1 = mul nsw i32 %0, 7
  %2 = getelementptr inbounds i32 addrspace(1)* %in, i32 %1
  %3 = load i32 addrspace(1)* %2, align 4
  %4 = add nsw i32 %3, 12
  br label %5

; <label>:5                                       ; preds = %5, %bb.nph
  %indvar = phi i32 [ 0, %bb.nph ], [ %i.03, %5 ]
  %y.12 = phi i32 [ %4, %bb.nph ], [ %tmp, %5 ]
  %x.11 = phi i32 [ 143, %bb.nph ], [ %x.0, %5 ]
  %i.03 = add i32 %indvar, 1
  %6 = icmp sgt i32 %i.03, 12
  %7 = add nsw i32 %x.11, -1
  %x.0 = select i1 %6, i32 %7, i32 %x.11
  %8 = add nsw i32 %y.12, 12
  %y.0 = select i1 %6, i32 %8, i32 %y.12
  %exitcond = icmp eq i32 %i.03, 999
  %tmp = add i32 %y.0, %i.03
  br i1 %exitcond, label %._crit_edge, label %5

._crit_edge:                                      ; preds = %5
  %9 = icmp eq i32 %tmp, 0
  %10 = icmp eq i32 %x.0, -2147483648
  %11 = icmp eq i32 %tmp, -1
  %12 = and i1 %10, %11
  %13 = or i1 %9, %12
  %14 = select i1 %13, i32 1, i32 %tmp
  %15 = sdiv i32 %x.0, %14
  %16 = getelementptr inbounds i32 addrspace(1)* %out, i32 %0
  store i32 %15, i32 addrspace(1)* %16, align 4
  %17 = icmp eq i32 %x.0, 0
  %18 = icmp eq i32 %tmp, -2147483648
  %19 = icmp eq i32 %x.0, -1
  %20 = and i1 %18, %19
  %21 = or i1 %17, %20
  %22 = select i1 %21, i32 1, i32 %x.0
  %23 = sdiv i32 %tmp, %22
  %24 = add nsw i32 %0, 10
  %25 = getelementptr inbounds i32 addrspace(1)* %out, i32 %24
  store i32 %23, i32 addrspace(1)* %25, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}

;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: ret