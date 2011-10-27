
; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s

; ModuleID = 'sdiv_seq_rnd.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = add nsw i32 %1, 100
  %3 = getelementptr inbounds i32 addrspace(1)* %in, i32 %1
  %4 = load i32 addrspace(1)* %3, align 4
  %5 = mul nsw i32 %4, 7
  %6 = add nsw i32 %5, 12
  %7 = icmp eq i32 %6, 0
  %8 = icmp eq i32 %2, -2147483648
  %9 = icmp eq i32 %6, -1
  %10 = and i1 %8, %9
  %11 = or i1 %7, %10
  %12 = select i1 %11, i32 1, i32 %6
  %13 = sdiv i32 %2, %12
  %14 = getelementptr inbounds i32 addrspace(1)* %out, i32 %1
  store i32 %13, i32 addrspace(1)* %14, align 4
  %15 = icmp eq i32 %2, 0
  %16 = icmp eq i32 %6, -2147483648
  %17 = icmp eq i32 %2, -1
  %18 = and i1 %16, %17
  %19 = or i1 %15, %18
  %20 = select i1 %19, i32 1, i32 %2
  %21 = sdiv i32 %6, %20
  %22 = add nsw i32 %1, 10
  %23 = getelementptr inbounds i32 addrspace(1)* %out, i32 %22
  store i32 %21, i32 addrspace(1)* %23, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}


;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: ret
