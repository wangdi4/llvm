; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; Check that @MagickRound is not inlined on the compile step of an -flto
; compilation because it is recognized by the function recognizer and
; inlining is delayed until the link step.

; RUN: opt -passes='function(functionrecognizer),cgscc(inline)' -funcrec-round -pre-lto-inline-cost -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup,function(functionrecognizer),cgscc(inline),inlinereportemitter' -funcrec-round -pre-lto-inline-cost -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe886 < %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK-MD

; CHECK-CL: call fast double @MagickRound(
; CHECK-LABEL: COMPILE FUNC: Wrapper
; CHECK: MagickRound {{.*}}Inline decision is delayed until link time
; CHECK-MD: call fast double @MagickRound(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare double @llvm.ceil.f64(double)

declare double @llvm.floor.f64(double)

define double @Wrapper(double noundef %x) {
  %y = call fast double @MagickRound(double noundef %x)
  ret double %y
}

define internal double @MagickRound(double noundef %x) {
entry:
  %0 = call fast double @llvm.floor.f64(double %x)
  %sub = fsub fast double %x, %0
  %1 = call fast double @llvm.ceil.f64(double %x)
  %sub1 = fsub fast double %1, %x
  %cmp = fcmp fast olt double %sub, %sub1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = call fast double @llvm.floor.f64(double %x)
  br label %return

if.end:                                           ; preds = %entry
  %3 = call fast double @llvm.ceil.f64(double %x)
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi double [ %2, %if.then ], [ %3, %if.end ]
  ret double %retval.0
}
; end INTEL_FEATURE_SW_ADVANCED
