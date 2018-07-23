; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -hir-dd-analysis -analyze -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s

; *** IR Dump After HIR Loop Memory Motion ***
; Function: main
;
; <0>       BEGIN REGION { modified }
; <34>               %limm = (@b)[0][0];
; <36>               %limm2 = (@b)[0][1];
; <38>               %limm4 = (@b)[0][2];
; <40>               %limm6 = (@b)[0][3];
; <25>            + DO i1 = 0, sext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; <3>             |   %arrayidx6.promoted = (@a)[0][i1];
; <5>             |   %add17 = %arrayidx6.promoted;
; <35>            |   %2 = %limm;
; <29>            |   %add17 = %add17  +  %2;
; <37>            |   %2 = %limm2;
; <31>            |   %add17 = %add17  +  %2;
; <39>            |   %2 = %limm4;
; <33>            |   %add17 = %add17  +  %2;
; <41>            |   %2 = %limm6;
; <10>            |   %add17 = %add17  +  %2;
; <18>            |   (@a)[0][i1] = %add17;
; <25>            + END LOOP
; <0>       END REGION

; CHECK: DD graph for function main:
; CHECK-NOT: 5:31 %add17 --> %add17 OUTPUT
; CHECK-NOT: 5:31 %add17 --> %add17 FLOW
; CHECK-NOT: 5:33 %add17 --> %add17 OUTPUT
; CHECK-NOT: 5:33 %add17 --> %add17 FLOW
; CHECK-NOT: 5:10 %add17 --> %add17 OUTPUT
; CHECK-NOT: 5:10 %add17 --> %add17 FLOW
; CHECK-NOT: 5:18 %add17 --> %add17 FLOW
; CHECK-NOT: 29:33 %add17 --> %add17 OUTPUT
; CHECK-NOT: 29:33 %add17 --> %add17 FLOW
; CHECK-NOT: 29:10 %add17 --> %add17 OUTPUT
; CHECK-NOT: 29:10 %add17 --> %add17 FLOW
; CHECK-NOT: 29:18 %add17 --> %add17 FLOW
; CHECK-NOT: 29:33 %add17 --> %add17 ANTI
; CHECK-NOT: 29:10 %add17 --> %add17 ANTI
; CHECK-NOT: 10:29 %add17 --> %add17 FLOW
; CHECK-NOT: 10:31 %add17 --> %add17 FLOW
; CHECK-NOT: 31:10 %add17 --> %add17 OUTPUT
; CHECK-NOT: 31:10 %add17 --> %add17 FLOW
; CHECK-NOT: 31:18 %add17 --> %add17 FLOW
; CHECK-NOT: 33:29 %add17 --> %add17 FLOW
; CHECK-NOT: 33:18 %add17 --> %add17 FLOW
; CHECK-NOT: 31:10 %add17 --> %add17 ANTI

;Module Before HIR; ModuleID = 'redundant-edge-after-unrolling.cpp'
source_filename = "redundant-edge-after-unrolling.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@a = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@n = dso_local local_unnamed_addr global i32 0, align 4
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_a.cpp, i8* null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #2

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %0 = load i32, i32* @n, align 4, !tbaa !2
  %cmp18 = icmp sgt i32 %0, 0
  br i1 %cmp18, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %1 = sext i32 %0 to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv20 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next21, %for.cond.cleanup3 ]
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %indvars.iv20
  %arrayidx6.promoted = load i32, i32* %arrayidx6, align 4, !tbaa !6
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret i32 0

for.cond.cleanup3:                                ; preds = %for.body4
  %add.lcssa = phi i32 [ %add, %for.body4 ]
  store i32 %add.lcssa, i32* %arrayidx6, align 4, !tbaa !6
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %cmp = icmp slt i64 %indvars.iv.next21, %1
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.cleanup.loopexit

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %add17 = phi i32 [ %arrayidx6.promoted, %for.cond1.preheader ], [ %add, %for.body4 ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %add = add nsw i32 %add17, %2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_a.cpp() #4 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZSt8__ioinit)
  %0 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZSt8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #2
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 01ee138640c026cc9c95bd0e90c07aeedaea0d66) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 896831630959e1bd5705f90f5b82401786bd363f)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA100_i", !3, i64 0}
