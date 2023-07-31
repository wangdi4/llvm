; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-mv-const-ub,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; Pragama likely trip count support for loop multi-versioning. Extract ZTT, and suppress the zero trip count case
;
;*** IR Dump Before HIR Multiversioning for constant UB ***
;
;<0>       BEGIN REGION { }
;<40>            + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;<41>            |   + DO i2 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 31> <trip_counts = 8, 6, 0>
;<11>            |   |   %0 = (@B)[0][i2];
;<13>            |   |   %.sink = %0;
;<14>            |   |   if (i2 <u 10)
;<14>            |   |   {
;<19>            |   |      %2 = (@B)[0][i2 + 1];
;<21>            |   |      %.sink = %0 + %2;
;<14>            |   |   }
;<25>            |   |   (@A)[0][i2] = %.sink;
;<41>            |   + END LOOP
;<40>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Multiversioning for constant UB ***
;
;<0>       BEGIN REGION { }
;<40>            + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;<2>             |   if (%M > 0)
;<2>             |   {
;<42>            |      if (sext.i32.i64(%M) + -1 == 7)
;<42>            |      {
;<43>            |         + DO i2 = 0, 7, 1   <DO_LOOP>
;<44>            |         |   %0 = (@B)[0][i2];
;<45>            |         |   %.sink = %0;
;<46>            |         |   if (i2 <u 10)
;<46>            |         |   {
;<47>            |         |      %2 = (@B)[0][i2 + 1];
;<48>            |         |      %.sink = %0 + %2;
;<46>            |         |   }
;<49>            |         |   (@A)[0][i2] = %.sink;
;<43>            |         + END LOOP
;<42>            |      }
;<42>            |      else
;<42>            |      {
;<50>            |         if (sext.i32.i64(%M) + -1 == 5)
;<50>            |         {
;<51>            |            + DO i2 = 0, 5, 1   <DO_LOOP>
;<52>            |            |   %0 = (@B)[0][i2];
;<53>            |            |   %.sink = %0;
;<54>            |            |   if (i2 <u 10)
;<54>            |            |   {
;<55>            |            |      %2 = (@B)[0][i2 + 1];
;<56>            |            |      %.sink = %0 + %2;
;<54>            |            |   }
;<57>            |            |   (@A)[0][i2] = %.sink;
;<51>            |            + END LOOP
;<50>            |         }
;<50>            |         else
;<50>            |         {
;<41>            |            + DO i2 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 31>
;<11>            |            |   %0 = (@B)[0][i2];
;<13>            |            |   %.sink = %0;
;<14>            |            |   if (i2 <u 10)
;<14>            |            |   {
;<19>            |            |      %2 = (@B)[0][i2 + 1];
;<21>            |            |      %.sink = %0 + %2;
;<14>            |            |   }
;<25>            |            |   (@A)[0][i2] = %.sink;
;<41>            |            + END LOOP
;<50>            |         }
;<42>            |      }
;<2>             |   }
;<40>            + END LOOP
;<0>       END REGION
;
; CHECK:    BEGIN REGION
; CHECK:    if (%M > 0)
; CHECK:    if (sext.i32.i64(%M) + -1 == 7)
; CHECK:    DO i2 = 0, 7
; CHECK:    if (sext.i32.i64(%M) + -1 == 5)
; CHECK:    DO i2 = 0, 5
; CHECK:    DO i2 = 0, sext.i32.i64(%M) + -1
;
;Module Before HIR; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [32 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [32 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %N, i32 %M) local_unnamed_addr #0 {
entry:
  %cmp28 = icmp sgt i32 %N, 0
  br i1 %cmp28, label %for.cond1.preheader.lr.ph, label %for.end16

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp226 = icmp sgt i32 %M, 0
  %wide.trip.count = sext i32 %M to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc14, %for.cond1.preheader.lr.ph
  %i.029 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc15, %for.inc14 ]
  br i1 %cmp226, label %for.body3.preheader, label %for.inc14

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.inc
  %indvars.iv = phi i64 [ %1, %for.inc ], [ 0, %for.body3.preheader ]
  %cmp4 = icmp ult i64 %indvars.iv, 10
  %arrayidx = getelementptr inbounds [32 x i32], ptr @B, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %1 = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %arrayidx6 = getelementptr inbounds [32 x i32], ptr @B, i64 0, i64 %1, !intel-tbaa !2
  %2 = load i32, ptr %arrayidx6, align 4, !tbaa !2
  %add7 = add nsw i32 %2, %0
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %.sink = phi i32 [ %add7, %if.then ], [ %0, %for.body3 ]
  %3 = getelementptr inbounds [32 x i32], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %.sink, ptr %3, align 4
  %exitcond = icmp eq i64 %1, %wide.trip.count
  br i1 %exitcond, label %for.inc14.loopexit, label %for.body3, !llvm.loop !7

for.inc14.loopexit:                               ; preds = %for.inc
  br label %for.inc14

for.inc14:                                        ; preds = %for.inc14.loopexit, %for.cond1.preheader
  %inc15 = add nuw nsw i32 %i.029, 1
  %exitcond31 = icmp eq i32 %inc15, %N
  br i1 %exitcond31, label %for.end16.loopexit, label %for.cond1.preheader

for.end16.loopexit:                               ; preds = %for.inc14
  br label %for.end16

for.end16:                                        ; preds = %for.end16.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 105f3862bb742d6adede6b26d29ed74003e24523) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 48000a19ba17626e045ddfab135d503b381e384a)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA32_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.intel.loopcount", i32 8, i32 6, i32 0}
