; RUN: llc -mcpu=sandybridge < %s
; ModuleID = '<stdin>'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

define  x86_ocl_kernelcc void @__Vectorized_.kfft(float addrspace(1)* nocapture %greal, float addrspace(1)* nocapture %gimag) nounwind {
k_sincos.exit.i:
  %A0 = shl <4 x i32> zeroinitializer, <i32 2, i32 2, i32 2, i32 2>
  %.sum963 = or <4 x i32> undef, %A0
  %.sum74 = zext <4 x i32> %.sum963 to <4 x i64>
  %A2 = zext i32 undef to i64
  %A3 = getelementptr inbounds float addrspace(1)* %greal, i64 %A2
  %A7 = bitcast float addrspace(1)* %A3 to <4 x float> addrspace(1)*
  store <4 x float> addrspace(1)* %A7, <4 x float> addrspace(1)** undef, align 8
  %A8 = bitcast float addrspace(1)* undef to <4 x float> addrspace(1)*
  store <4 x float> addrspace(1)* %A8, <4 x float> addrspace(1)** undef, align 8
  %.sum269270121 = or <4 x i64> %.sum74, <i64 768, i64 768, i64 768, i64 768>
  %extract123 = extractelement <4 x i64> %.sum269270121, i32 1
  %A9 = getelementptr inbounds float addrspace(1)* %gimag, i64 %extract123
  %A10 = bitcast float addrspace(1)* %A9 to <4 x float> addrspace(1)*
  store <4 x float> addrspace(1)* %A10, <4 x float> addrspace(1)** undef, align 8
  unreachable
}
