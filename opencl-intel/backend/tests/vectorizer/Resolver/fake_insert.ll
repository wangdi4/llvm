; RUN: opt  -runtimelib %p/../Full/runtime.bc -resolve -verify -instcombine -verify -S -o - %s \
; RUN: | FileCheck %s

; ModuleID = '/home/work/zrackove/git/trunk2/src/backend/tests/vectorizer/Packetizer/Mangling/soa_all.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

define void @run_all(<4 x i8> %inp1, <4 x i8> %inp2, <4 x i8> %inp3, <4 x i8> %inp4) nounwind {
; CHECK: @run_all
; CHECK-NOT: call <4 x i8> @fake.insert.element
; CHECK: ret
entry:
  %scalar = extractelement <4 x i8> %inp1, i32 0
  %scalar1 = extractelement <4 x i8> %inp1, i32 1
  %scalar2 = extractelement <4 x i8> %inp1, i32 2
  %scalar3 = extractelement <4 x i8> %inp1, i32 3
  %0 = call <4 x i8> @fake.insert.element0(<4 x i8> undef, i8 %scalar, i32 0)
  %1 = call <4 x i8> @fake.insert.element1(<4 x i8> %0, i8 %scalar1, i32 1)
  %2 = call <4 x i8> @fake.insert.element2(<4 x i8> %1, i8 %scalar2, i32 2)
  %3 = call <4 x i8> @fake.insert.element3(<4 x i8> %2, i8 %scalar3, i32 3)
  %call = tail call i32 @_Z3allDv4_c(<4 x i8> %3) nounwind readnone
  %assembled.vect = insertelement <4 x i32> undef, i32 %call, i32 0
  %assembled.vect4 = insertelement <4 x i32> %assembled.vect, i32 undef, i32 1
  %assembled.vect5 = insertelement <4 x i32> %assembled.vect4, i32 undef, i32 2
  %assembled.vect6 = insertelement <4 x i32> %assembled.vect5, i32 undef, i32 3
  tail call void @foo(<4 x i32> %assembled.vect6) nounwind
  ret void
}

declare i32 @_Z3allDv4_c(<4 x i8>) nounwind readnone

declare void @foo(<4 x i32>)

declare <4 x i8> @fake.insert.element0(<4 x i8>, i8, i32)

declare <4 x i8> @fake.insert.element1(<4 x i8>, i8, i32)

declare <4 x i8> @fake.insert.element2(<4 x i8>, i8, i32)

declare <4 x i8> @fake.insert.element3(<4 x i8>, i8, i32)

declare i1 @allOne(i1)

declare i1 @allZero(i1)

!opencl.kernels = !{!0}

!0 = !{void (<4 x i8>, <4 x i8>, <4 x i8>, <4 x i8>)* @run_all, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
