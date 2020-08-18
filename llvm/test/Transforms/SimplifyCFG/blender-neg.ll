; REQUIRES: asserts
; RUN: opt < %s -debug-only=simplifycfg -simplifycfg -S 2>&1 | FileCheck %s
; CHECK: already sorted

; Blender optimization should bail out here when it sees a constant compare
; in the 1st block position.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local float @foo(float %a, float %b, float %c, float %d, float %e, float %f, float %g, float %h, float %i, float %j, float %k, float %l)  {
entry:
  %retval = alloca float, align 4
  %a.addr = alloca float, align 4
  %b.addr = alloca float, align 4
  %c.addr = alloca float, align 4
  %d.addr = alloca float, align 4
  %e.addr = alloca float, align 4
  %f.addr = alloca float, align 4
  %g.addr = alloca float, align 4
  %h.addr = alloca float, align 4
  %i.addr = alloca float, align 4
  %j.addr = alloca float, align 4
  %k.addr = alloca float, align 4
  %l.addr = alloca float, align 4
  store float %a, float* %a.addr, align 4
  store float %b, float* %b.addr, align 4
  store float %c, float* %c.addr, align 4
  store float %d, float* %d.addr, align 4
  store float %e, float* %e.addr, align 4
  store float %f, float* %f.addr, align 4
  store float %g, float* %g.addr, align 4
  store float %h, float* %h.addr, align 4
  store float %i, float* %i.addr, align 4
  store float %j, float* %j.addr, align 4
  store float %k, float* %k.addr, align 4
  store float %l, float* %l.addr, align 4
  %0 = load float, float* %a.addr, align 4
  %cmp = fcmp fast olt float %0, 0.000000e+00
  br i1 %cmp, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %1 = load float, float* %b.addr, align 4
  %cmp1 = fcmp fast olt float %1, 0.000000e+00
  br i1 %cmp1, label %if.then, label %lor.lhs.false2

lor.lhs.false2:                                   ; preds = %lor.lhs.false
  %2 = load float, float* %c.addr, align 4
  %cmp3 = fcmp fast olt float %2, 0.000000e+00
  br i1 %cmp3, label %if.then, label %lor.lhs.false4

lor.lhs.false4:                                   ; preds = %lor.lhs.false2
  %3 = load float, float* %d.addr, align 4
  %cmp5 = fcmp fast olt float %3, 0.000000e+00
  br i1 %cmp5, label %if.then, label %lor.lhs.false6

lor.lhs.false6:                                   ; preds = %lor.lhs.false4
  %4 = load float, float* %e.addr, align 4
  %cmp7 = fcmp fast olt float %4, 0.000000e+00
  br i1 %cmp7, label %if.then, label %lor.lhs.false8

lor.lhs.false8:                                   ; preds = %lor.lhs.false6
  %5 = load float, float* %f.addr, align 4
  %cmp9 = fcmp fast olt float %5, 0.000000e+00
  br i1 %cmp9, label %if.then, label %lor.lhs.false10

lor.lhs.false10:                                  ; preds = %lor.lhs.false8
  %6 = load float, float* %g.addr, align 4
  %cmp11 = fcmp fast olt float %6, 0.000000e+00
  br i1 %cmp11, label %if.then, label %lor.lhs.false12

lor.lhs.false12:                                  ; preds = %lor.lhs.false10
  %7 = load float, float* %h.addr, align 4
  %cmp13 = fcmp fast olt float %7, 0.000000e+00
  br i1 %cmp13, label %if.then, label %lor.lhs.false14

lor.lhs.false14:                                  ; preds = %lor.lhs.false12
  %8 = load float, float* %i.addr, align 4
  %cmp15 = fcmp fast olt float %8, 0.000000e+00
  br i1 %cmp15, label %if.then, label %lor.lhs.false16

lor.lhs.false16:                                  ; preds = %lor.lhs.false14
  %9 = load float, float* %j.addr, align 4
  %cmp17 = fcmp fast olt float %9, 0.000000e+00
  br i1 %cmp17, label %if.then, label %lor.lhs.false18

lor.lhs.false18:                                  ; preds = %lor.lhs.false16
  %10 = load float, float* %k.addr, align 4
  %cmp19 = fcmp fast olt float %10, 0.000000e+00
  br i1 %cmp19, label %if.then, label %lor.lhs.false20

lor.lhs.false20:                                  ; preds = %lor.lhs.false18
  %11 = load float, float* %l.addr, align 4
  %cmp21 = fcmp fast olt float %11, 0.000000e+00
  br i1 %cmp21, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false20, %lor.lhs.false18, %lor.lhs.false16, %lor.lhs.false14, %lor.lhs.false12, %lor.lhs.false10, %lor.lhs.false8, %lor.lhs.false6, %lor.lhs.false4, %lor.lhs.false2, %lor.lhs.false, %entry
  store float 1.000000e+00, float* %retval, align 4
  br label %return

if.end:                                           ; preds = %lor.lhs.false20
  store float 0.000000e+00, float* %retval, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %12 = load float, float* %retval, align 4
  ret float %12
}


