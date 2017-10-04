; RUN: llvm-as %s -o %t.bc
; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -O3  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s

; ModuleID = 'add_seq_seq.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @store_float(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = add nsw i32 %1, 10
  %3 = icmp sgt i32 %2, 11
  %4 = add nsw i32 %2, %1
  %5 = shl i32 %2, 2
  %z.0 = select i1 %3, i32 %4, i32 %5
  %6 = add nsw i32 %z.0, -1
  %7 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %1
  store i32 %6, i32 addrspace(1)* %7, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!opencl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @store_float, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_store_float_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}

;CHECK: WIA 3  %{{[a-z]*}}{{[0-9]*}} = add
;CHECK: ret