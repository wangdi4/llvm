; RUN: %oclopt -runtimelib %p/../Full/runtime.bc -runtime=ocl -CLBltnPreVec -verify -S -o - %s \
; RUN: | FileCheck %s

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @sincos_kernel1(double addrspace(1)* nocapture %out, double addrspace(1)* %out2, double addrspace(1)* nocapture %in) nounwind {
; CHECK: [[V2:%[0-9a-zA-Z._]+]] = call <2 x double> @_Z20__retbyvector_sincosd(double %0)
; CHECK: = extractelement <2 x double> [[V2]], i32 0
; CHECK: [[C2:%[0-9a-zA-Z._]+]] = extractelement <2 x double> [[V2]], i32 1
; CHECK: store double [[C2]], double addrspace(1)* %add.ptr
; CHECK: ret
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds double, double addrspace(1)* %in, i64 %idxprom
  %0 = load double, double addrspace(1)* %arrayidx, align 128
  %add.ptr = getelementptr inbounds double, double addrspace(1)* %out2, i64 %idxprom
  %call1 = tail call double @_Z6sincosdPU3AS1d(double %0, double addrspace(1)* %add.ptr) nounwind
  %arrayidx3 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom
  store double %call1, double addrspace(1)* %arrayidx3, align 128
  ret void
}

define void @sincos_kernel16(<16 x double> addrspace(1)* nocapture %out, <16 x double> addrspace(1)* %out2, <16 x double> addrspace(1)* nocapture %in) nounwind {
; CHECK: [[V1:%[0-9]+]] = call [2 x <16 x double>] @_Z19__retbyarray_sincosDv16_d(<16 x double> %0)
; CHECK: = extractvalue [2 x <16 x double>] [[V1]], 0
; CHECK: [[C1:%[0-9a-zA-Z._]+]] = extractvalue [2 x <16 x double>] [[V1]], 1
; CHECK: store <16 x double> [[C1]], <16 x double> addrspace(1)* %add.ptr
; CHECK: ret
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds <16 x double>, <16 x double> addrspace(1)* %in, i64 %idxprom
  %0 = load <16 x double>, <16 x double> addrspace(1)* %arrayidx, align 128
  %add.ptr = getelementptr inbounds <16 x double>, <16 x double> addrspace(1)* %out2, i64 %idxprom
  %call1 = tail call <16 x double> @_Z6sincosDv16_dPU3AS1S_(<16 x double> %0, <16 x double> addrspace(1)* %add.ptr) nounwind
  %arrayidx3 = getelementptr inbounds <16 x double>, <16 x double> addrspace(1)* %out, i64 %idxprom
  store <16 x double> %call1, <16 x double> addrspace(1)* %arrayidx3, align 128
  ret void
}

define void @native_sincos_kernel1(double addrspace(1)* nocapture %out, double addrspace(1)* %out2, double addrspace(1)* nocapture %in) nounwind {
; CHECK: [[V2:%[0-9]+]] = call <2 x double> @_Z27__retbyvector_native_sincosd(double %0)
; CHECK: = extractelement <2 x double> [[V2]], i32 0
; CHECK: [[C2:%[0-9a-zA-Z._]+]] = extractelement <2 x double> [[V2]], i32 1
; CHECK: store double [[C2]], double addrspace(1)* %add.ptr
; CHECK: ret
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds double, double addrspace(1)* %in, i64 %idxprom
  %0 = load double, double addrspace(1)* %arrayidx, align 128
  %add.ptr = getelementptr inbounds double, double addrspace(1)* %out2, i64 %idxprom
  %call1 = tail call double @_Z13native_sincosdPU3AS1d(double %0, double addrspace(1)* %add.ptr) nounwind
  %arrayidx3 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom
  store double %call1, double addrspace(1)* %arrayidx3, align 128
  ret void
}

define void @native_sincos_kernel16(<16 x double> addrspace(1)* nocapture %out, <16 x double> addrspace(1)* %out2, <16 x double> addrspace(1)* nocapture %in) nounwind {
; CHECK: [[V1:%[0-9]+]] = call [2 x <16 x double>] @_Z26__retbyarray_native_sincosDv16_d(<16 x double> %0)
; CHECK: = extractvalue [2 x <16 x double>] [[V1]], 0
; CHECK: [[C1:%[0-9a-zA-Z._]+]] = extractvalue [2 x <16 x double>] [[V1]], 1
; CHECK: store <16 x double> [[C1]], <16 x double> addrspace(1)* %add.ptr
; CHECK: ret
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds <16 x double>, <16 x double> addrspace(1)* %in, i64 %idxprom
  %0 = load <16 x double>, <16 x double> addrspace(1)* %arrayidx, align 128
  %add.ptr = getelementptr inbounds <16 x double>, <16 x double> addrspace(1)* %out2, i64 %idxprom
  %call1 = tail call <16 x double> @_Z13native_sincosDv16_dPU3AS1S_(<16 x double> %0, <16 x double> addrspace(1)* %add.ptr) nounwind
  %arrayidx3 = getelementptr inbounds <16 x double>, <16 x double> addrspace(1)* %out, i64 %idxprom
  store <16 x double> %call1, <16 x double> addrspace(1)* %arrayidx3, align 128
  ret void
}
declare i64 @_Z13get_global_idj(i32) nounwind readnone

declare double @_Z6sincosdPU3AS1d(double, double addrspace(1)*) nounwind
declare <16 x double> @_Z6sincosDv16_dPU3AS1S_(<16 x double>, <16 x double> addrspace(1)*)
declare double @_Z13native_sincosdPU3AS1d(double, double addrspace(1)*) nounwind
declare <16 x double> @_Z13native_sincosDv16_dPU3AS1S_(<16 x double>, <16 x double> addrspace(1)*)

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)
