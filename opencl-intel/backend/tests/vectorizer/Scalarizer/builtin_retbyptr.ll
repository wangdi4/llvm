; RUN: opt  -runtimelib %p/../Full/runtime.bc -scalarize -verify -S -o - %s \
; RUN: | FileCheck %s

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @sincos_kernel(double addrspace(1)* nocapture %out, double addrspace(1)* %out2, double addrspace(1)* nocapture %in) nounwind {
; CHECK: [[C1:%[_a-z0-9]+]] = call <2 x double> @_Z20__retbyvector_sincosd(double {{%[_a-z0-9]+}})
; CHECK: [[E0:%[_a-z0-9]+]] = call double @fake.extract.element0(<2 x double> [[C1]], i32 0)
; CHECK: [[E1:%[_a-z0-9]+]] = call double @fake.extract.element1(<2 x double> [[C1]], i32 1)
; CHECK: store double [[E1]], double addrspace(1)*
; CHECK: store double [[E0]], double addrspace(1)*
; CHECK: ret
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds double addrspace(1)* %in, i64 %idxprom
  %0 = load double addrspace(1)* %arrayidx, align 128
  %add.ptr = getelementptr inbounds double addrspace(1)* %out2, i64 %idxprom
  %1 = call <2 x double> @_Z20__retbyvector_sincosd(double %0) nounwind readnone
  %_Z20__retbyvector_sincosd_extract. = extractelement <2 x double> %1, i32 0
  %_Z20__retbyvector_sincosd_extract.1 = extractelement <2 x double> %1, i32 1
  store double %_Z20__retbyvector_sincosd_extract.1, double addrspace(1)* %add.ptr
  %arrayidx3 = getelementptr inbounds double addrspace(1)* %out, i64 %idxprom
  store double %_Z20__retbyvector_sincosd_extract., double addrspace(1)* %arrayidx3, align 128
  ret void
}

define void @sincos4_kernel(<4 x double> addrspace(1)* nocapture %out, <4 x double> addrspace(1)* %out2, <4 x double> addrspace(1)* nocapture %in) nounwind {
; CHECK: [[D2:%[_a-z0-9]+]] = call <2 x double> @_Z20__retbyvector_sincosd(double
; CHECK: [[F0:%[_a-z0-9]+]] = call double @fake.extract.element2(<2 x double> [[D2]], i32 0)
; CHECK: [[F1:%[_a-z0-9]+]] = call double @fake.extract.element3(<2 x double> [[D2]], i32 1)
; CHECK: [[D3:%[_a-z0-9]+]] = call <2 x double> @_Z20__retbyvector_sincosd(double
; CHECK: [[F2:%[_a-z0-9]+]] = call double @fake.extract.element4(<2 x double> [[D3]], i32 0)
; CHECK: [[F3:%[_a-z0-9]+]] = call double @fake.extract.element5(<2 x double> [[D3]], i32 1)
; CHECK: [[D4:%[_a-z0-9]+]] = call <2 x double> @_Z20__retbyvector_sincosd(double
; CHECK: [[F4:%[_a-z0-9]+]] = call double @fake.extract.element6(<2 x double> [[D4]], i32 0)
; CHECK: [[F5:%[_a-z0-9]+]] = call double @fake.extract.element7(<2 x double> [[D4]], i32 1)
; CHECK: [[D5:%[_a-z0-9]+]] = call <2 x double> @_Z20__retbyvector_sincosd(double
; CHECK: [[F6:%[_a-z0-9]+]] = call double @fake.extract.element8(<2 x double> [[D5]], i32 0)
; CHECK: [[F7:%[_a-z0-9]+]] = call double @fake.extract.element9(<2 x double> [[D5]], i32 1)
; CHECK: [[A0:%[._a-z0-9]+]] = insertelement <4 x double> undef, double [[F0]], i32 0
; CHECK: [[A1:%[._a-z0-9]+]] = insertelement <4 x double> [[A0]], double [[F2]], i32 1
; CHECK: [[A2:%[._a-z0-9]+]] = insertelement <4 x double> [[A1]], double [[F4]], i32 2
; CHECK: [[A3:%[._a-z0-9]+]] = insertelement <4 x double> [[A2]], double [[F6]], i32 3
; CHECK: [[A4:%[._a-z0-9]+]] = insertelement <4 x double> undef, double [[F1]], i32 0
; CHECK: [[A5:%[._a-z0-9]+]] = insertelement <4 x double> [[A4]], double [[F3]], i32 1
; CHECK: [[A6:%[._a-z0-9]+]] = insertelement <4 x double> [[A5]], double [[F5]], i32 2
; CHECK: [[A7:%[._a-z0-9]+]] = insertelement <4 x double> [[A6]], double [[F7]], i32 3
; CHECK: store <4 x double> [[A7]], <4 x double> addrspace(1)*
; CHECK: store <4 x double> [[A3]], <4 x double> addrspace(1)*
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds <4 x double> addrspace(1)* %in, i64 %idxprom
  %0 = load <4 x double> addrspace(1)* %arrayidx, align 128
  %add.ptr = getelementptr inbounds <4 x double> addrspace(1)* %out2, i64 %idxprom
  %1 = call [2 x <4 x double>] @_Z19__retbyarray_sincosDv4_d(<4 x double> %0)
  %_Z19__retbyarray_sincosDv4_d_extract. = extractvalue [2 x <4 x double>] %1, 0
  %_Z19__retbyarray_sincosDv4_d_extract.1 = extractvalue [2 x <4 x double>] %1, 1
  store <4 x double> %_Z19__retbyarray_sincosDv4_d_extract.1, <4 x double> addrspace(1)* %add.ptr
  %arrayidx3 = getelementptr inbounds <4 x double> addrspace(1)* %out, i64 %idxprom
  store <4 x double> %_Z19__retbyarray_sincosDv4_d_extract., <4 x double> addrspace(1)* %arrayidx3, align 128
  ret void
}

define void @native_sincos_kernel(double addrspace(1)* nocapture %out, double addrspace(1)* %out2, double addrspace(1)* nocapture %in) nounwind {
; CHECK: [[C1:%[_a-z0-9]+]] = call <2 x double> @_Z27__retbyvector_native_sincosd(double {{%[_a-z0-9]+}})
; CHECK: [[E0:%[_a-z0-9]+]] = call double @fake.extract.element10(<2 x double> [[C1]], i32 0)
; CHECK: [[E1:%[_a-z0-9]+]] = call double @fake.extract.element11(<2 x double> [[C1]], i32 1)
; CHECK: store double [[E1]], double addrspace(1)*
; CHECK: store double [[E0]], double addrspace(1)*
; CHECK: ret
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds double addrspace(1)* %in, i64 %idxprom
  %0 = load double addrspace(1)* %arrayidx, align 128
  %add.ptr = getelementptr inbounds double addrspace(1)* %out2, i64 %idxprom
  %1 = call <2 x double> @_Z27__retbyvector_native_sincosd(double %0) nounwind readnone
  %_Z27__retbyvector_native_sincosd_extract. = extractelement <2 x double> %1, i32 0
  %_Z27__retbyvector_native_sincosd_extract.1 = extractelement <2 x double> %1, i32 1
  store double %_Z27__retbyvector_native_sincosd_extract.1, double addrspace(1)* %add.ptr
  %arrayidx3 = getelementptr inbounds double addrspace(1)* %out, i64 %idxprom
  store double %_Z27__retbyvector_native_sincosd_extract., double addrspace(1)* %arrayidx3, align 128
  ret void
}

define void @native_sincos4_kernel(<4 x double> addrspace(1)* nocapture %out, <4 x double> addrspace(1)* %out2, <4 x double> addrspace(1)* nocapture %in) nounwind {
; CHECK: [[D2:%[_a-z0-9]+]] = call <2 x double> @_Z27__retbyvector_native_sincosd(double
; CHECK: [[F0:%[_a-z0-9]+]] = call double @fake.extract.element12(<2 x double> [[D2]], i32 0)
; CHECK: [[F1:%[_a-z0-9]+]] = call double @fake.extract.element13(<2 x double> [[D2]], i32 1)
; CHECK: [[D3:%[_a-z0-9]+]] = call <2 x double> @_Z27__retbyvector_native_sincosd(double
; CHECK: [[F2:%[_a-z0-9]+]] = call double @fake.extract.element14(<2 x double> [[D3]], i32 0)
; CHECK: [[F3:%[_a-z0-9]+]] = call double @fake.extract.element15(<2 x double> [[D3]], i32 1)
; CHECK: [[D4:%[_a-z0-9]+]] = call <2 x double> @_Z27__retbyvector_native_sincosd(double
; CHECK: [[F4:%[_a-z0-9]+]] = call double @fake.extract.element16(<2 x double> [[D4]], i32 0)
; CHECK: [[F5:%[_a-z0-9]+]] = call double @fake.extract.element17(<2 x double> [[D4]], i32 1)
; CHECK: [[D5:%[_a-z0-9]+]] = call <2 x double> @_Z27__retbyvector_native_sincosd(double
; CHECK: [[F6:%[_a-z0-9]+]] = call double @fake.extract.element18(<2 x double> [[D5]], i32 0)
; CHECK: [[F7:%[_a-z0-9]+]] = call double @fake.extract.element19(<2 x double> [[D5]], i32 1)
; CHECK: [[A0:%[._a-z0-9]+]] = insertelement <4 x double> undef, double [[F0]], i32 0
; CHECK: [[A1:%[._a-z0-9]+]] = insertelement <4 x double> [[A0]], double [[F2]], i32 1
; CHECK: [[A2:%[._a-z0-9]+]] = insertelement <4 x double> [[A1]], double [[F4]], i32 2
; CHECK: [[A3:%[._a-z0-9]+]] = insertelement <4 x double> [[A2]], double [[F6]], i32 3
; CHECK: [[A4:%[._a-z0-9]+]] = insertelement <4 x double> undef, double [[F1]], i32 0
; CHECK: [[A5:%[._a-z0-9]+]] = insertelement <4 x double> [[A4]], double [[F3]], i32 1
; CHECK: [[A6:%[._a-z0-9]+]] = insertelement <4 x double> [[A5]], double [[F5]], i32 2
; CHECK: [[A7:%[._a-z0-9]+]] = insertelement <4 x double> [[A6]], double [[F7]], i32 3
; CHECK: store <4 x double> [[A7]], <4 x double> addrspace(1)*
; CHECK: store <4 x double> [[A3]], <4 x double> addrspace(1)*
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds <4 x double> addrspace(1)* %in, i64 %idxprom
  %0 = load <4 x double> addrspace(1)* %arrayidx, align 128
  %add.ptr = getelementptr inbounds <4 x double> addrspace(1)* %out2, i64 %idxprom
  %1 = call [2 x <4 x double>] @_Z26__retbyarray_native_sincosDv4_d(<4 x double> %0)
  %_Z26__retbyarray_native_sincosDv4_d_extract. = extractvalue [2 x <4 x double>] %1, 0
  %_Z26__retbyarray_native_sincosDv4_d_extract.1 = extractvalue [2 x <4 x double>] %1, 1
  store <4 x double> %_Z26__retbyarray_native_sincosDv4_d_extract.1, <4 x double> addrspace(1)* %add.ptr
  %arrayidx3 = getelementptr inbounds <4 x double> addrspace(1)* %out, i64 %idxprom
  store <4 x double> %_Z26__retbyarray_native_sincosDv4_d_extract., <4 x double> addrspace(1)* %arrayidx3, align 128
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

declare <2 x double> @_Z20__retbyvector_sincosd(double) nounwind readnone
declare <2 x double> @_Z27__retbyvector_native_sincosd(double) nounwind readnone

declare [2 x <4 x double>] @_Z19__retbyarray_sincosDv4_d(<4 x double>) nounwind readnone
declare [2 x <4 x double>] @_Z26__retbyarray_native_sincosDv4_d(<4 x double>) nounwind readnone
