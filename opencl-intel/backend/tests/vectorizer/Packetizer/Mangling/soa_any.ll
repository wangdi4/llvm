; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-link-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=16 -verify -S -o - %s \
; RUN: | FileCheck %s

; ModuleID = 'run_any.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

; calls to any() are uniform, so do expect the SOA version to be called
;CHECK: @run_any
;CHECK-NOT: call <16 x i32> @_Z8soa_any4Dv16_cS_S_S_
;CHECK: call i32 @_Z3any
;CHECK-NOT: call <16 x i32> @_Z8soa_any4Dv16_cS_S_S_
;CHECK: ret
define void @run_any(<4 x i8> %inp1, <4 x i8> %inp2, <4 x i8> %inp3, <4 x i8> %inp4) nounwind {
entry:
  %inp1.addr = alloca <4 x i8>, align 4
  %inp2.addr = alloca <4 x i8>, align 4
  %inp3.addr = alloca <4 x i8>, align 4
  %inp4.addr = alloca <4 x i8>, align 4
  %out = alloca <4 x i32>, align 16
  store <4 x i8> %inp1, <4 x i8>* %inp1.addr, align 4
  store <4 x i8> %inp2, <4 x i8>* %inp2.addr, align 4
  store <4 x i8> %inp3, <4 x i8>* %inp3.addr, align 4
  store <4 x i8> %inp4, <4 x i8>* %inp4.addr, align 4
  %0 = load <4 x i8>* %inp1.addr, align 4
  %call = call i32 @_Z3anyDv4_c(<4 x i8> %0) nounwind readnone
  %1 = load <4 x i32>* %out
  %2 = insertelement <4 x i32> %1, i32 %call, i32 0
  store <4 x i32> %2, <4 x i32>* %out
  %3 = load <4 x i8>* %inp2.addr, align 4
  %call1 = call i32 @_Z3anyDv4_c(<4 x i8> %3) nounwind readnone
  %4 = load <4 x i32>* %out
  %5 = insertelement <4 x i32> %4, i32 %call1, i32 1
  store <4 x i32> %5, <4 x i32>* %out
  %6 = load <4 x i8>* %inp3.addr, align 4
  %call2 = call i32 @_Z3anyDv4_c(<4 x i8> %6) nounwind readnone
  %7 = load <4 x i32>* %out
  %8 = insertelement <4 x i32> %7, i32 %call2, i32 2
  store <4 x i32> %8, <4 x i32>* %out
  %9 = load <4 x i8>* %inp4.addr, align 4
  %call3 = call i32 @_Z3anyDv4_c(<4 x i8> %9) nounwind readnone
  %10 = load <4 x i32>* %out
  %11 = insertelement <4 x i32> %10, i32 %call3, i32 3
  store <4 x i32> %11, <4 x i32>* %out
  %12 = load <4 x i32>* %out, align 16
  call void @foo(<4 x i32> %12)
  ret void
}

declare i32 @_Z3anyDv4_c(<4 x i8>) nounwind readnone

declare void @foo(<4 x i32>)

!opencl.kernels = !{!0}

!0 = !{void (<4 x i8>, <4 x i8>, <4 x i8>, <4 x i8>)* @run_any, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
