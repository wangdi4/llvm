; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=4 -verify %s -S -o - \
; RUN: | FileCheck %s

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


declare i64 @_Z13get_global_idj(i32) nounwind readnone

define void @__Vectorized_.sincos(<4 x double> addrspace(1)* nocapture %out, <4 x double> addrspace(1)* %out2, <4 x double> addrspace(1)* nocapture %in) nounwind {
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S1_(<4 x double> {{%[a-zA-Z0-9_]+}}, <4 x double>* [[S0:%[a-zA-Z0-9_]+]], <4 x double>* [[C0:%[a-zA-Z0-9_]+]])
; CHECK: load <4 x double>, <4 x double>* [[S0]]
; CHECK: load <4 x double>, <4 x double>* [[C0]]
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S1_(<4 x double> {{%[a-zA-Z0-9_]+}}, <4 x double>* [[S1:%[a-zA-Z0-9_]+]], <4 x double>* [[C1:%[a-zA-Z0-9_]+]])
; CHECK: load <4 x double>, <4 x double>* [[S1]]
; CHECK: load <4 x double>, <4 x double>* [[C1]]
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S1_(<4 x double> {{%[a-zA-Z0-9_]+}}, <4 x double>* [[S2:%[a-zA-Z0-9_]+]], <4 x double>* [[C2:%[a-zA-Z0-9_]+]])
; CHECK: load <4 x double>, <4 x double>* [[S2]]
; CHECK: load <4 x double>, <4 x double>* [[C2]]
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S1_(<4 x double> {{%[a-zA-Z0-9_]+}}, <4 x double>* [[S3:%[a-zA-Z0-9_]+]], <4 x double>* [[C3:%[a-zA-Z0-9_]+]])
; CHECK: load <4 x double>, <4 x double>* [[S3]]
; CHECK: load <4 x double>, <4 x double>* [[C3]]
; CHECK: ret
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %in, i64 %idxprom
  %0 = load <4 x double>, <4 x double> addrspace(1)* %arrayidx, align 32
  %scalar = extractelement <4 x double> %0, i32 0
  %scalar2 = extractelement <4 x double> %0, i32 1
  %scalar3 = extractelement <4 x double> %0, i32 2
  %scalar4 = extractelement <4 x double> %0, i32 3
  %add.ptr = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %out2, i64 %idxprom
  %_clone = call <2 x double> @_Z20__retbyvector_sincosd(double %scalar) nounwind readnone
  %fake_extract = call double @fake.extract.element0(<2 x double> %_clone, i32 0) nounwind readnone
  %fake_extract5 = call double @fake.extract.element1(<2 x double> %_clone, i32 1) nounwind readnone
  %_clone6 = call <2 x double> @_Z20__retbyvector_sincosd(double %scalar2) nounwind readnone
  %fake_extract7 = call double @fake.extract.element2(<2 x double> %_clone6, i32 0) nounwind readnone
  %fake_extract8 = call double @fake.extract.element3(<2 x double> %_clone6, i32 1) nounwind readnone
  %_clone9 = call <2 x double> @_Z20__retbyvector_sincosd(double %scalar3) nounwind readnone
  %fake_extract10 = call double @fake.extract.element4(<2 x double> %_clone9, i32 0) nounwind readnone
  %fake_extract11 = call double @fake.extract.element5(<2 x double> %_clone9, i32 1) nounwind readnone
  %_clone12 = call <2 x double> @_Z20__retbyvector_sincosd(double %scalar4) nounwind readnone
  %fake_extract13 = call double @fake.extract.element6(<2 x double> %_clone12, i32 0) nounwind readnone
  %fake_extract14 = call double @fake.extract.element7(<2 x double> %_clone12, i32 1) nounwind readnone
  %assembled.vect18 = insertelement <4 x double> undef, double %fake_extract, i32 0
  %assembled.vect19 = insertelement <4 x double> %assembled.vect18, double %fake_extract7, i32 1
  %assembled.vect20 = insertelement <4 x double> %assembled.vect19, double %fake_extract10, i32 2
  %assembled.vect21 = insertelement <4 x double> %assembled.vect20, double %fake_extract13, i32 3
  %assembled.vect = insertelement <4 x double> undef, double %fake_extract5, i32 0
  %assembled.vect15 = insertelement <4 x double> %assembled.vect, double %fake_extract8, i32 1
  %assembled.vect16 = insertelement <4 x double> %assembled.vect15, double %fake_extract11, i32 2
  %assembled.vect17 = insertelement <4 x double> %assembled.vect16, double %fake_extract14, i32 3
  store <4 x double> %assembled.vect17, <4 x double> addrspace(1)* %add.ptr
  %arrayidx3 = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %out, i64 %idxprom
  store <4 x double> %assembled.vect21, <4 x double> addrspace(1)* %arrayidx3, align 32
  ret void
}
declare <2 x double> @_Z20__retbyvector_sincosd(double) nounwind readnone

define void @__Vectorized_.native_sincos(<4 x double> addrspace(1)* nocapture %out, <4 x double> addrspace(1)* %out2, <4 x double> addrspace(1)* nocapture %in) nounwind {
; CHECK: call void @_Z21native_sincos_ret2ptrDv4_dPS_S1_(<4 x double> {{%[a-zA-Z0-9_]+}}, <4 x double>* [[S0:%[a-zA-Z0-9_]+]], <4 x double>* [[C0:%[a-zA-Z0-9_]+]])
; CHECK: load <4 x double>, <4 x double>* [[S0]]
; CHECK: load <4 x double>, <4 x double>* [[C0]]
; CHECK: call void @_Z21native_sincos_ret2ptrDv4_dPS_S1_(<4 x double> {{%[a-zA-Z0-9_]+}}, <4 x double>* [[S1:%[a-zA-Z0-9_]+]], <4 x double>* [[C1:%[a-zA-Z0-9_]+]])
; CHECK: load <4 x double>, <4 x double>* [[S1]]
; CHECK: load <4 x double>, <4 x double>* [[C1]]
; CHECK: call void @_Z21native_sincos_ret2ptrDv4_dPS_S1_(<4 x double> {{%[a-zA-Z0-9_]+}}, <4 x double>* [[S2:%[a-zA-Z0-9_]+]], <4 x double>* [[C2:%[a-zA-Z0-9_]+]])
; CHECK: load <4 x double>, <4 x double>* [[S2]]
; CHECK: load <4 x double>, <4 x double>* [[C2]]
; CHECK: call void @_Z21native_sincos_ret2ptrDv4_dPS_S1_(<4 x double> {{%[a-zA-Z0-9_]+}}, <4 x double>* [[S3:%[a-zA-Z0-9_]+]], <4 x double>* [[C3:%[a-zA-Z0-9_]+]])
; CHECK: load <4 x double>, <4 x double>* [[S3]]
; CHECK: load <4 x double>, <4 x double>* [[C3]]
; CHECK: ret
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %in, i64 %idxprom
  %0 = load <4 x double>, <4 x double> addrspace(1)* %arrayidx, align 32
  %scalar = extractelement <4 x double> %0, i32 0
  %scalar2 = extractelement <4 x double> %0, i32 1
  %scalar3 = extractelement <4 x double> %0, i32 2
  %scalar4 = extractelement <4 x double> %0, i32 3
  %add.ptr = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %out2, i64 %idxprom
  %_clone = call <2 x double> @_Z27__retbyvector_native_sincosd(double %scalar) nounwind readnone
  %fake_extract = call double @fake.extract.element0(<2 x double> %_clone, i32 0) nounwind readnone
  %fake_extract5 = call double @fake.extract.element1(<2 x double> %_clone, i32 1) nounwind readnone
  %_clone6 = call <2 x double> @_Z27__retbyvector_native_sincosd(double %scalar2) nounwind readnone
  %fake_extract7 = call double @fake.extract.element2(<2 x double> %_clone6, i32 0) nounwind readnone
  %fake_extract8 = call double @fake.extract.element3(<2 x double> %_clone6, i32 1) nounwind readnone
  %_clone9 = call <2 x double> @_Z27__retbyvector_native_sincosd(double %scalar3) nounwind readnone
  %fake_extract10 = call double @fake.extract.element4(<2 x double> %_clone9, i32 0) nounwind readnone
  %fake_extract11 = call double @fake.extract.element5(<2 x double> %_clone9, i32 1) nounwind readnone
  %_clone12 = call <2 x double> @_Z27__retbyvector_native_sincosd(double %scalar4) nounwind readnone
  %fake_extract13 = call double @fake.extract.element6(<2 x double> %_clone12, i32 0) nounwind readnone
  %fake_extract14 = call double @fake.extract.element7(<2 x double> %_clone12, i32 1) nounwind readnone
  %assembled.vect18 = insertelement <4 x double> undef, double %fake_extract, i32 0
  %assembled.vect19 = insertelement <4 x double> %assembled.vect18, double %fake_extract7, i32 1
  %assembled.vect20 = insertelement <4 x double> %assembled.vect19, double %fake_extract10, i32 2
  %assembled.vect21 = insertelement <4 x double> %assembled.vect20, double %fake_extract13, i32 3
  %assembled.vect = insertelement <4 x double> undef, double %fake_extract5, i32 0
  %assembled.vect15 = insertelement <4 x double> %assembled.vect, double %fake_extract8, i32 1
  %assembled.vect16 = insertelement <4 x double> %assembled.vect15, double %fake_extract11, i32 2
  %assembled.vect17 = insertelement <4 x double> %assembled.vect16, double %fake_extract14, i32 3
  store <4 x double> %assembled.vect17, <4 x double> addrspace(1)* %add.ptr
  %arrayidx3 = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %out, i64 %idxprom
  store <4 x double> %assembled.vect21, <4 x double> addrspace(1)* %arrayidx3, align 32
  ret void
}
declare <2 x double> @_Z27__retbyvector_native_sincosd(double) nounwind readnone

declare double @fake.extract.element0(<2 x double>, i32) nounwind readnone

declare double @fake.extract.element1(<2 x double>, i32) nounwind readnone

declare double @fake.extract.element2(<2 x double>, i32) nounwind readnone

declare double @fake.extract.element3(<2 x double>, i32) nounwind readnone

declare double @fake.extract.element4(<2 x double>, i32) nounwind readnone

declare double @fake.extract.element5(<2 x double>, i32) nounwind readnone

declare double @fake.extract.element6(<2 x double>, i32) nounwind readnone

declare double @fake.extract.element7(<2 x double>, i32) nounwind readnone
