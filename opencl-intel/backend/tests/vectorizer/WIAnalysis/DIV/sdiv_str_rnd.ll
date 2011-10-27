; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s



; ModuleID = 'sdiv_str_rnd.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = mul nsw i32 %1, 11
  %3 = add nsw i32 %2, 100
  %4 = getelementptr inbounds i32 addrspace(1)* %in, i32 %1
  %5 = load i32 addrspace(1)* %4, align 4
  %6 = mul nsw i32 %5, 7
  %7 = add nsw i32 %6, -12
  %8 = icmp eq i32 %7, 0
  %9 = icmp eq i32 %3, -2147483648
  %10 = icmp eq i32 %7, -1
  %11 = and i1 %9, %10
  %12 = or i1 %8, %11
  %13 = select i1 %12, i32 1, i32 %7
  %14 = sdiv i32 %3, %13
  %15 = getelementptr inbounds i32 addrspace(1)* %out, i32 %1
  store i32 %14, i32 addrspace(1)* %15, align 4
  %16 = icmp eq i32 %3, 0
  %17 = icmp eq i32 %7, -2147483648
  %18 = icmp eq i32 %3, -1
  %19 = and i1 %17, %18
  %20 = or i1 %16, %19
  %21 = select i1 %20, i32 1, i32 %3
  %22 = sdiv i32 %7, %21
  %23 = add nsw i32 %1, 10
  %24 = getelementptr inbounds i32 addrspace(1)* %out, i32 %23
  store i32 %22, i32 addrspace(1)* %24, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", metadata !"opencl_store_float_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}



;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: ret
