; RUN: opt -passes=sycl-kernel-relaxed-math -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-relaxed-math -S %s | FileCheck %s

define void @check_sincos_float_global() {
; CHECK-LABEL: define void @check_sincos_float_global
; CHECK: call float @_Z9sincos_rmfPU3AS1f
; CHECK: call <2 x float> @_Z9sincos_rmDv2_fPU3AS1S_
; CHECK: call <3 x float> @_Z9sincos_rmDv3_fPU3AS1S_
; CHECK: call <4 x float> @_Z9sincos_rmDv4_fPU3AS1S_
; CHECK: call <8 x float> @_Z9sincos_rmDv8_fPU3AS1S_
; CHECK: call <16 x float> @_Z9sincos_rmDv16_fPU3AS1S_
entry:
  %call01 = call float @_Z6sincosfPU3AS1f(float undef, ptr addrspace(1) undef)
  %call02 = call <2 x float> @_Z6sincosDv2_fPU3AS1S_(<2 x float> undef, ptr addrspace(1) undef)
  %call03 = call <3 x float> @_Z6sincosDv3_fPU3AS1S_(<3 x float> undef, ptr addrspace(1) undef)
  %call04 = call <4 x float> @_Z6sincosDv4_fPU3AS1S_(<4 x float> undef, ptr addrspace(1) undef)
  %call05 = call <8 x float> @_Z6sincosDv8_fPU3AS1S_(<8 x float> undef, ptr addrspace(1) undef)
  %call06 = call <16 x float> @_Z6sincosDv16_fPU3AS1S_(<16 x float> undef, ptr addrspace(1) undef)
  ret void
}

define void @check_sincos_float_local() {
; CHECK-LABEL: define void @check_sincos_float_local
; CHECK: call float @_Z9sincos_rmfPU3AS2f
; CHECK: call <2 x float> @_Z9sincos_rmDv2_fPU3AS2S_
; CHECK: call <3 x float> @_Z9sincos_rmDv3_fPU3AS2S_
; CHECK: call <4 x float> @_Z9sincos_rmDv4_fPU3AS2S_
; CHECK: call <8 x float> @_Z9sincos_rmDv8_fPU3AS2S_
; CHECK: call <16 x float> @_Z9sincos_rmDv16_fPU3AS2S_
entry:
  %call01 = call float @_Z6sincosfPU3AS2f(float undef, ptr addrspace(2) undef)
  %call02 = call <2 x float> @_Z6sincosDv2_fPU3AS2S_(<2 x float> undef, ptr addrspace(2) undef)
  %call03 = call <3 x float> @_Z6sincosDv3_fPU3AS2S_(<3 x float> undef, ptr addrspace(2) undef)
  %call04 = call <4 x float> @_Z6sincosDv4_fPU3AS2S_(<4 x float> undef, ptr addrspace(2) undef)
  %call05 = call <8 x float> @_Z6sincosDv8_fPU3AS2S_(<8 x float> undef, ptr addrspace(2) undef)
  %call06 = call <16 x float> @_Z6sincosDv16_fPU3AS2S_(<16 x float> undef, ptr addrspace(2) undef)
  ret void
}

define void @check_sincos_float_private() {
; CHECK-LABEL: define void @check_sincos_float_private
; CHECK: call float @_Z9sincos_rmfPU3AS3f
; CHECK: call <2 x float> @_Z9sincos_rmDv2_fPU3AS3S_
; CHECK: call <3 x float> @_Z9sincos_rmDv3_fPU3AS3S_
; CHECK: call <4 x float> @_Z9sincos_rmDv4_fPU3AS3S_
; CHECK: call <8 x float> @_Z9sincos_rmDv8_fPU3AS3S_
; CHECK: call <16 x float> @_Z9sincos_rmDv16_fPU3AS3S_
entry:
  %call01 = call float @_Z6sincosfPU3AS3f(float undef, ptr addrspace(3) undef)
  %call02 = call <2 x float> @_Z6sincosDv2_fPU3AS3S_(<2 x float> undef, ptr addrspace(3) undef)
  %call03 = call <3 x float> @_Z6sincosDv3_fPU3AS3S_(<3 x float> undef, ptr addrspace(3) undef)
  %call04 = call <4 x float> @_Z6sincosDv4_fPU3AS3S_(<4 x float> undef, ptr addrspace(3) undef)
  %call05 = call <8 x float> @_Z6sincosDv8_fPU3AS3S_(<8 x float> undef, ptr addrspace(3) undef)
  %call06 = call <16 x float> @_Z6sincosDv16_fPU3AS3S_(<16 x float> undef, ptr addrspace(3) undef)
  ret void
}

define void @check_sincos_float_generic() {
; CHECK-LABEL: define void @check_sincos_float_generic
; CHECK: call float @_Z9sincos_rmfPU3AS4f
; CHECK: call <2 x float> @_Z9sincos_rmDv2_fPU3AS4S_
; CHECK: call <3 x float> @_Z9sincos_rmDv3_fPU3AS4S_
; CHECK: call <4 x float> @_Z9sincos_rmDv4_fPU3AS4S_
; CHECK: call <8 x float> @_Z9sincos_rmDv8_fPU3AS4S_
; CHECK: call <16 x float> @_Z9sincos_rmDv16_fPU3AS4S_
entry:
  %call01 = call float @_Z6sincosfPU3AS4f(float undef, ptr addrspace(4) undef)
  %call02 = call <2 x float> @_Z6sincosDv2_fPU3AS4S_(<2 x float> undef, ptr addrspace(4) undef)
  %call03 = call <3 x float> @_Z6sincosDv3_fPU3AS4S_(<3 x float> undef, ptr addrspace(4) undef)
  %call04 = call <4 x float> @_Z6sincosDv4_fPU3AS4S_(<4 x float> undef, ptr addrspace(4) undef)
  %call05 = call <8 x float> @_Z6sincosDv8_fPU3AS4S_(<8 x float> undef, ptr addrspace(4) undef)
  %call06 = call <16 x float> @_Z6sincosDv16_fPU3AS4S_(<16 x float> undef, ptr addrspace(4) undef)
  ret void
}

define void @check_sincos_float_noaddrspace() {
; CHECK-LABEL: define void @check_sincos_float_noaddrspace
; CHECK: call float @_Z9sincos_rmfPf
; CHECK: call <2 x float> @_Z9sincos_rmDv2_fPS_
; CHECK: call <3 x float> @_Z9sincos_rmDv3_fPS_
; CHECK: call <4 x float> @_Z9sincos_rmDv4_fPS_
; CHECK: call <8 x float> @_Z9sincos_rmDv8_fPS_
; CHECK: call <16 x float> @_Z9sincos_rmDv16_fPS_
entry:
  %call01 = call float @_Z6sincosfPf(float undef, ptr undef)
  %call02 = call <2 x float> @_Z6sincosDv2_fPS_(<2 x float> undef, ptr undef)
  %call03 = call <3 x float> @_Z6sincosDv3_fPS_(<3 x float> undef, ptr undef)
  %call04 = call <4 x float> @_Z6sincosDv4_fPS_(<4 x float> undef, ptr undef)
  %call05 = call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> undef, ptr undef)
  %call06 = call <16 x float> @_Z6sincosDv16_fPS_(<16 x float> undef, ptr undef)
  ret void
}

declare float @_Z6sincosfPU3AS1f(float, ptr addrspace(1))
declare <2 x float> @_Z6sincosDv2_fPU3AS1S_(<2 x float>, ptr addrspace(1))
declare <3 x float> @_Z6sincosDv3_fPU3AS1S_(<3 x float>, ptr addrspace(1))
declare <4 x float> @_Z6sincosDv4_fPU3AS1S_(<4 x float>, ptr addrspace(1))
declare <8 x float> @_Z6sincosDv8_fPU3AS1S_(<8 x float>, ptr addrspace(1))
declare <16 x float> @_Z6sincosDv16_fPU3AS1S_(<16 x float>, ptr addrspace(1))

declare float @_Z6sincosfPU3AS2f(float, ptr addrspace(2))
declare <2 x float> @_Z6sincosDv2_fPU3AS2S_(<2 x float>, ptr addrspace(2))
declare <3 x float> @_Z6sincosDv3_fPU3AS2S_(<3 x float>, ptr addrspace(2))
declare <4 x float> @_Z6sincosDv4_fPU3AS2S_(<4 x float>, ptr addrspace(2))
declare <8 x float> @_Z6sincosDv8_fPU3AS2S_(<8 x float>, ptr addrspace(2))
declare <16 x float> @_Z6sincosDv16_fPU3AS2S_(<16 x float>, ptr addrspace(2))

declare float @_Z6sincosfPU3AS3f(float, ptr addrspace(3))
declare <2 x float> @_Z6sincosDv2_fPU3AS3S_(<2 x float>, ptr addrspace(3))
declare <3 x float> @_Z6sincosDv3_fPU3AS3S_(<3 x float>, ptr addrspace(3))
declare <4 x float> @_Z6sincosDv4_fPU3AS3S_(<4 x float>, ptr addrspace(3))
declare <8 x float> @_Z6sincosDv8_fPU3AS3S_(<8 x float>, ptr addrspace(3))
declare <16 x float> @_Z6sincosDv16_fPU3AS3S_(<16 x float>, ptr addrspace(3))

declare float @_Z6sincosfPU3AS4f(float, ptr addrspace(4))
declare <2 x float> @_Z6sincosDv2_fPU3AS4S_(<2 x float>, ptr addrspace(4))
declare <3 x float> @_Z6sincosDv3_fPU3AS4S_(<3 x float>, ptr addrspace(4))
declare <4 x float> @_Z6sincosDv4_fPU3AS4S_(<4 x float>, ptr addrspace(4))
declare <8 x float> @_Z6sincosDv8_fPU3AS4S_(<8 x float>, ptr addrspace(4))
declare <16 x float> @_Z6sincosDv16_fPU3AS4S_(<16 x float>, ptr addrspace(4))

declare float @_Z6sincosfPf(float, ptr)
declare <2 x float> @_Z6sincosDv2_fPS_(<2 x float>, ptr)
declare <3 x float> @_Z6sincosDv3_fPS_(<3 x float>, ptr)
declare <4 x float> @_Z6sincosDv4_fPS_(<4 x float>, ptr)
declare <8 x float> @_Z6sincosDv8_fPS_(<8 x float>, ptr)
declare <16 x float> @_Z6sincosDv16_fPS_(<16 x float>, ptr)

!opencl.ocl.version = !{!0}

!0 = !{i32 2, i32 0}

; DEBUGIFY-NOT: WARNING
