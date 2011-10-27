
 ; WIA tests 
 ; add uniform * uniform
 ;sub uniform * uniform
 



; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s


; ModuleID = 'add_un_un.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
bb.nph:
  %0 = tail call i32 @get_global_id(i32 0) nounwind
  %1 = getelementptr inbounds i32 addrspace(1)* %out, i32 %0
  br label %2

; <label>:2                                       ; preds = %9, %bb.nph
  %i.03 = phi i32 [ 0, %bb.nph ], [ %.pre-phi, %9 ]
  %y.12 = phi i32 [ 1, %bb.nph ], [ %y.0, %9 ]
  %x.11 = phi i32 [ 23, %bb.nph ], [ %x.0, %9 ]
  %3 = icmp sgt i32 %y.12, 12
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %2
  %tmp7 = add i32 %i.03, 1
  %5 = add nsw i32 %y.12, %x.11
  store i32 %5, i32 addrspace(1)* %1, align 4
  br label %9

; <label>:6                                       ; preds = %2
  %tmp5 = sub i32 -4, %i.03
  %7 = shl i32 %y.12, 1
  %8 = sub nsw i32 %x.11, %7
  store i32 %8, i32 addrspace(1)* %1, align 4
  %tmp6 = add i32 %x.11, %tmp5
  %.pre10 = add nsw i32 %i.03, 1
  br label %9

; <label>:9                                       ; preds = %4, %6
  %.pre-phi = phi i32 [ %tmp7, %4 ], [ %.pre10, %6 ]
  %10 = phi i32 [ %5, %4 ], [ %8, %6 ]
  %x.0 = phi i32 [ %x.11, %4 ], [ %tmp6, %6 ]
  %tmp7.pn = phi i32 [ %tmp7, %4 ], [ 3, %6 ]
  %y.0 = add i32 %y.12, %tmp7.pn
  %exitcond = icmp eq i32 %.pre-phi, 1000
  br i1 %exitcond, label %11, label %2

; <label>:11                                      ; preds = %9
  %.pre11 = getelementptr inbounds i32 addrspace(1)* %out, i32 %0
  %12 = add nsw i32 %y.0, %x.0
  %13 = add nsw i32 %12, %10
  store i32 %13, i32 addrspace(1)* %.pre11, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}

;CHECK: WIA 0  %{{[a-z]*}}{{[0-9]*}} = add
;CHECK: WIA 0  %{{[a-z]*}}{{[0-9]*}} = sub
;CHECK: ret