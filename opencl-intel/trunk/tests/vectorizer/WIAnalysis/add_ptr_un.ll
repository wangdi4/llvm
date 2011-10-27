

; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s

; ModuleID = 'add_ptr_un.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = add nsw i32 %1, 1
  %3 = getelementptr inbounds i32 addrspace(1)* %out, i32 %2
  %4 = load i32 addrspace(1)* %3, align 4
  %5 = add nsw i32 %4, 2
  %6 = icmp sgt i32 %5, 10
  br i1 %6, label %7, label %10

; <label>:7                                       ; preds = %0
  %8 = add nsw i32 %4, 1
  %9 = getelementptr inbounds i32 addrspace(1)* %out, i32 %1
  store i32 %8, i32 addrspace(1)* %9, align 4
  ret void

; <label>:10                                      ; preds = %0
  %11 = getelementptr inbounds i32 addrspace(1)* %out, i32 %1
  store i32 2, i32 addrspace(1)* %11, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}



;CHECK: add
;CHECK: WIA 2  %{{[a-z]*}}{{[0-9]*}} = getelementptr
;CHECK: ret