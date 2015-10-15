; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -O3  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll  | FileCheck %s


; ModuleID = 'sdiv_seq_seq.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = add nsw i32 %1, 100
  %3 = icmp eq i32 %1, 0
  %4 = icmp eq i32 %2, -2147483648
  %5 = icmp eq i32 %1, -1
  %6 = and i1 %4, %5
  %7 = or i1 %3, %6
  %8 = select i1 %7, i32 1, i32 %1
  %9 = sdiv i32 %2, %8
  %10 = getelementptr inbounds i32 addrspace(1)* %out, i32 %1
  store i32 %9, i32 addrspace(1)* %10, align 4
  %11 = icmp eq i32 %2, 0
  %12 = icmp eq i32 %1, -2147483648
  %13 = icmp eq i32 %2, -1
  %14 = and i1 %12, %13
  %15 = or i1 %11, %14
  %16 = select i1 %15, i32 1, i32 %2
  %17 = sdiv i32 %1, %16
  %18 = add nsw i32 %1, 10
  %19 = getelementptr inbounds i32 addrspace(1)* %out, i32 %18
  store i32 %17, i32 addrspace(1)* %19, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!opencl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_store_float_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}


;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: WIA 4  %{{[a-z]*}}{{[0-9]*}} = sdiv
;CHECK: ret