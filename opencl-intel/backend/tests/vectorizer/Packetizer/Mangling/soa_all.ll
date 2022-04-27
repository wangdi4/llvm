; RUN: %oclopt  -runtimelib %p/../../Full/runtime.bc -packetize -packet-size=16 -verify %s -S -o - \
; RUN: | FileCheck %s


; calls to all() are uniform, so do expect the SOA version to be called
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

define void @run_all(<4 x i8> %inp1, <4 x i8> %inp2, <4 x i8> %inp3, <4 x i8> %inp4) nounwind {
; CHECK: @run_all
; CHECK-NOT: call <16 x i32> @_Z8soa_all4Dv16_cS_S_S_
; CHECK: call i32 @_Z3all
; CHECK-NOT: call <16 x i32> @_Z8soa_all4Dv16_cS_S_S_
; CHECK: ret
entry:
  %scalar = extractelement <4 x i8> %inp1, i32 0
  %scalar1 = extractelement <4 x i8> %inp1, i32 1
  %scalar2 = extractelement <4 x i8> %inp1, i32 2
  %scalar3 = extractelement <4 x i8> %inp1, i32 3
  %0 = call <4 x i8> @fake.insert.element0(<4 x i8> undef, i8 %scalar, i32 0) nounwind readnone
  %1 = call <4 x i8> @fake.insert.element1(<4 x i8> %0, i8 %scalar1, i32 1) nounwind readnone
  %2 = call <4 x i8> @fake.insert.element2(<4 x i8> %1, i8 %scalar2, i32 2) nounwind readnone
  %3 = call <4 x i8> @fake.insert.element3(<4 x i8> %2, i8 %scalar3, i32 3) nounwind readnone
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

declare <4 x i8> @fake.insert.element0(<4 x i8>, i8, i32) nounwind readnone

declare <4 x i8> @fake.insert.element1(<4 x i8>, i8, i32) nounwind readnone

declare <4 x i8> @fake.insert.element2(<4 x i8>, i8, i32) nounwind readnone

declare <4 x i8> @fake.insert.element3(<4 x i8>, i8, i32) nounwind readnone

declare i1 @allOne(i1)

declare i1 @allZero(i1)
