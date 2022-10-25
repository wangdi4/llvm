; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 < %s -disable-output 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; CQ378383: Test to see that a single branch with a test for a global against
; a constant is tolerated when we are inlining with -Os (inline threshold 15).

; ModuleID = 'sm.c'

; CHECK: Begin
; CHECK: <<Callee is single basic block with test>>
; CHECK: End Inlining Report

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@Ch_1_Glob = common global i8 0, align 1

; Function Attrs: nounwind optsize uwtable
define i32 @Proc_2(i32* nocapture %Int_Par_Ref) #0 {
entry:
  %0 = load i8, i8* @Ch_1_Glob, align 1
  %cmp = icmp eq i8 %0, 65
  br i1 %cmp, label %if.then, label %do.end

if.then:                                          ; preds = %entry
  %1 = load i32, i32* %Int_Par_Ref, align 4
  %sub = add nsw i32 %1, 9
  store i32 %sub, i32* %Int_Par_Ref, align 4
  br label %do.end

do.end:                                           ; preds = %if.then, %entry
  ret i32 0
}

; Function Attrs: nounwind optsize readonly uwtable
define i32 @main() #1 {
entry:
  %retval = alloca i32, align 4
  %Int_1_Loc = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 3, i32* %Int_1_Loc, align 4
  %call = call i32 @Proc_2(i32* %Int_1_Loc)
  ret i32 %call
}

attributes #0 = { nounwind optsize uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind optsize readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

; end INTEL_FEATURE_SW_ADVANCED
