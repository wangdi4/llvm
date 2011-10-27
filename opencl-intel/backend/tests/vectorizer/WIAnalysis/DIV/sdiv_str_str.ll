; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll  | FileCheck %s



; ModuleID = 'sdiv_str_str.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = mul nsw i32 %1, 11
  %3 = add nsw i32 %2, 100
  %4 = mul nsw i32 %1, 7
  %5 = add nsw i32 %4, -12
  %6 = icmp eq i32 %5, 0
  %7 = icmp eq i32 %3, -2147483648
  %8 = icmp eq i32 %5, -1
  %9 = and i1 %7, %8
  %10 = or i1 %6, %9
  %11 = select i1 %10, i32 1, i32 %5
  %12 = sdiv i32 %3, %11
  %13 = getelementptr inbounds i32 addrspace(1)* %out, i32 %1
  store i32 %12, i32 addrspace(1)* %13, align 4
  %14 = icmp eq i32 %3, 0
  %15 = icmp eq i32 %5, -2147483648
  %16 = icmp eq i32 %3, -1
  %17 = and i1 %15, %16
  %18 = or i1 %14, %17
  %19 = select i1 %18, i32 1, i32 %3
  %20 = sdiv i32 %5, %19
  %21 = add nsw i32 %1, 10
  %22 = getelementptr inbounds i32 addrspace(1)* %out, i32 %21
  store i32 %20, i32 addrspace(1)* %22, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}


;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: ret