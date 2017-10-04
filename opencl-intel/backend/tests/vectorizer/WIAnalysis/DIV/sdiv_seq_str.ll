; RUN: llvm-as %s -o %t.bc
; RUN: %oclopt  -runtimelib %p/../../Full/runtime.bc -O3  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s


; ModuleID = 'sdiv_seq_str.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = add nsw i32 %1, 100
  %3 = mul nsw i32 %1, 7
  %4 = add nsw i32 %3, 12
  %5 = icmp eq i32 %4, 0
  %6 = icmp eq i32 %2, -2147483648
  %7 = icmp eq i32 %4, -1
  %8 = and i1 %6, %7
  %9 = or i1 %5, %8
  %10 = select i1 %9, i32 1, i32 %4
  %11 = sdiv i32 %2, %10
  %12 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %1
  store i32 %11, i32 addrspace(1)* %12, align 4
  %13 = icmp eq i32 %2, 0
  %14 = icmp eq i32 %4, -2147483648
  %15 = icmp eq i32 %2, -1
  %16 = and i1 %14, %15
  %17 = or i1 %13, %16
  %18 = select i1 %17, i32 1, i32 %2
  %19 = sdiv i32 %4, %18
  %20 = add nsw i32 %1, 10
  %21 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %20
  store i32 %19, i32 addrspace(1)* %21, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!opencl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_store_float_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}

;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: ret