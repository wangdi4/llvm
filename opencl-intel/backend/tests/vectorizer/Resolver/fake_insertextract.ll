; RUN: opt  -runtimelib %p/../Full/runtime.bc -resolve -verify -S < %s \
; RUN: | FileCheck %s
; Make sure no fake calls exist
; CHECK-NOT call{{.*}}@fake

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

define void @test_func(<3 x float> addrspace(1)* nocapture %dst, <3 x float> %cx3) nounwind {
; CHECK: @test_func
; CHECK: [[I0:%[a-zA-Z0-9_]+]] = insertelement <3 x float> undef, float %{{[a-zA-Z0-9_]+}}, i32 0
; CHECK: [[I1:%[a-zA-Z0-9_]+]] = insertelement <3 x float> [[I0]], float %{{[a-zA-Z0-9_]+}}, i32 1
; CHECK: [[I2:%[a-zA-Z0-9_]+]] = insertelement <3 x float> [[I1]], float %{{[a-zA-Z0-9_]+}}, i32 2
; CHECK: [[C0:%[a-zA-Z0-9_]+]] = tail call <3 x float> @_Z14fast_normalizeDv3_f(<3 x float> [[I2]])
; CHECK: [[E0:%[a-zA-Z0-9_]+]] = extractelement <3 x float> [[C0]], i32 0
; CHECK: [[E1:%[a-zA-Z0-9_]+]] = extractelement <3 x float> %call_clone, i32 1
; CHECK: [[E2:%[a-zA-Z0-9_]+]] = extractelement <3 x float> %call_clone, i32 2
; Make sure the extracts are actually used, and used correctly
; CHECK: insertelement <3 x float> undef, float [[E0]], i32 0
; CHECK: insertelement <3 x float> %assembled.vect, float [[E1]], i32 1
; CHECK: insertelement <3 x float> %assembled.vect3, float [[E2]], i32 2
; CHECK: ret
entry:
  %scalar = extractelement <3 x float> %cx3, i32 0
  %scalar1 = extractelement <3 x float> %cx3, i32 1
  %scalar2 = extractelement <3 x float> %cx3, i32 2
  %0 = call <3 x float> @fake.insert.element0(<3 x float> undef, float %scalar, i32 0) nounwind readnone
  %1 = call <3 x float> @fake.insert.element1(<3 x float> %0, float %scalar1, i32 1) nounwind readnone
  %2 = call <3 x float> @fake.insert.element2(<3 x float> %1, float %scalar2, i32 2) nounwind readnone
  %call_clone = tail call <3 x float> @_Z14fast_normalizeDv3_f(<3 x float> %2) nounwind readnone
  %3 = call float @fake.extract.element0(<3 x float> %call_clone, i32 0) nounwind readnone
  %4 = call float @fake.extract.element1(<3 x float> %call_clone, i32 1) nounwind readnone
  %5 = call float @fake.extract.element2(<3 x float> %call_clone, i32 2) nounwind readnone
  %assembled.vect = insertelement <3 x float> undef, float %3, i32 0
  %assembled.vect3 = insertelement <3 x float> %assembled.vect, float %4, i32 1
  %assembled.vect4 = insertelement <3 x float> %assembled.vect3, float %5, i32 2
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %temp = insertelement <4 x i64> undef, i64 %call1, i32 0
  %vector = shufflevector <4 x i64> %temp, <4 x i64> undef, <4 x i32> zeroinitializer
  %6 = add <4 x i64> %vector, <i64 0, i64 1, i64 2, i64 3>
  %extract = extractelement <4 x i64> %6, i32 0
  %extract5 = extractelement <4 x i64> %6, i32 1
  %extract6 = extractelement <4 x i64> %6, i32 2
  %extract7 = extractelement <4 x i64> %6, i32 3
  %7 = getelementptr inbounds <3 x float> addrspace(1)* %dst, i64 %extract
  %8 = getelementptr inbounds <3 x float> addrspace(1)* %dst, i64 %extract5
  %9 = getelementptr inbounds <3 x float> addrspace(1)* %dst, i64 %extract6
  %10 = getelementptr inbounds <3 x float> addrspace(1)* %dst, i64 %extract7
  store <3 x float> %assembled.vect4, <3 x float> addrspace(1)* %7, align 16
  store <3 x float> %assembled.vect4, <3 x float> addrspace(1)* %8, align 16
  store <3 x float> %assembled.vect4, <3 x float> addrspace(1)* %9, align 16
  store <3 x float> %assembled.vect4, <3 x float> addrspace(1)* %10, align 16
  ret void
}

declare <3 x float> @_Z14fast_normalizeDv3_f(<3 x float>) nounwind readnone
declare i64 @_Z13get_global_idj(i32) nounwind readnone
declare <3 x float> @fake.insert.element0(<3 x float>, float, i32) nounwind readnone
declare <3 x float> @fake.insert.element1(<3 x float>, float, i32) nounwind readnone
declare <3 x float> @fake.insert.element2(<3 x float>, float, i32) nounwind readnone
declare float @fake.extract.element0(<3 x float>, i32) nounwind readnone
declare float @fake.extract.element1(<3 x float>, i32) nounwind readnone
declare float @fake.extract.element2(<3 x float>, i32) nounwind readnone
