; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt < %s -funcrec-round -passes='function(functionrecognizer)' -debug-only=functionrecognizer -S 2>&1 | FileCheck %s

; Test that @MagickRound is recognized as a rounding function.

; CHECK: FUNCTION-RECOGNIZER: FOUND MAGICK-ROUND MagickRound
; CHECK: define{{.*}}@MagickRound({{.*}}) #[[A0:[0-9]+]]
; CHECK: attributes #[[A0]] = { "is-magick-round" }

declare double @llvm.ceil.f64(double)

declare double @llvm.floor.f64(double)

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
