; TODO:
;  Enable the test by removing the following 2 lines after the pass directly
;  include VectInfo.gen.
; REQUIRES: linux
; UNSUPPORTED: linux

; RUN: opt -passes=dpcpp-kernel-convert-vplan-mask %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-convert-vplan-mask %s -S | FileCheck %s

; This test checks that HandleVPlanMask pass keeps the mask argument
; of function vector variants already using VPlan-styled masks.
; AKA, the mask element type is not converted to i32 type.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test(<4 x double> %a, <4 x double> %b) {
  %1 = call <4 x double> @_Z4acosDv4_dS_(<4 x double> %a, <4 x double> %b)
; CHECK:     call <4 x double> @_Z4acosDv4_dS_(<4 x double> %a, <4 x double>
; CHECK-NOT: call <4 x double> @_Z4acosDv4_dS_(<4 x double> %a, <4 x i32>
  ret void
}

; Function Attrs: convergent nounwind readnone
declare <4 x double> @_Z4acosDv4_dS_(<4 x double>, <4 x double>) local_unnamed_addr #0
; CHECK:     declare <4 x double> @_Z4acosDv4_dS_(<4 x double>, <4 x double>)
; CHECK-NOT: declare <4 x double> @_Z4acosDv4_dS_(<4 x double>, <4 x i32>)

attributes #0 = { convergent nounwind readnone "call-params-num"="1" }

; DEBUGIFY-NOT: WARNING
