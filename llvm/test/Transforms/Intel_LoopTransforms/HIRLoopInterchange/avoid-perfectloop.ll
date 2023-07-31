; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-interchange,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s

; Enabling a perfect loop nest is correct. But we choose not to do so.
; Interchange afterwards is not valid, so perfect loop nest does not help the performance.

; CHECK:Function: main

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:               |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:               |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:               |   |   |   %s.087 = (@a)[0][i3][i1]  +  %s.087;
; CHECK:               |   |   + END LOOP
; CHECK:               |   |
; CHECK:               |   |   (@b)[0][i2][i1] = %s.087;
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION


; CHECK:Function: main


; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:               |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:               |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:               |   |   |   %s.087 = (@a)[0][i3][i1]  +  %s.087;
; CHECK:               |   |   + END LOOP
; CHECK:               |   |
; CHECK:               |   |   (@b)[0][i2][i1] = %s.087;
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION

;Module Before HIR
; ModuleID = 'interch_3.c'
source_filename = "interch_3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16
@.str = private unnamed_addr constant [4 x i8] c"%d \00", align 1
@c = common dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc10, %entry
  %indvars.iv105 = phi i64 [ 0, %entry ], [ %indvars.iv.next106, %for.inc10 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv102 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next103, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [20 x [20 x i32]], ptr @a, i64 0, i64 %indvars.iv105, i64 %indvars.iv102, !intel-tbaa !2
  store i32 7, ptr %arrayidx5, align 4, !tbaa !2
  %arrayidx9 = getelementptr inbounds [20 x [20 x i32]], ptr @b, i64 0, i64 %indvars.iv105, i64 %indvars.iv102, !intel-tbaa !2
  store i32 777, ptr %arrayidx9, align 4, !tbaa !2
  %indvars.iv.next103 = add nuw nsw i64 %indvars.iv102, 1
  %exitcond104 = icmp eq i64 %indvars.iv.next103, 20
  br i1 %exitcond104, label %for.inc10, label %for.body3

for.inc10:                                        ; preds = %for.body3
  %indvars.iv.next106 = add nuw nsw i64 %indvars.iv105, 1
  %exitcond107 = icmp eq i64 %indvars.iv.next106, 20
  br i1 %exitcond107, label %for.cond16.preheader.preheader, label %for.cond1.preheader

for.cond16.preheader.preheader:                   ; preds = %for.inc10
  br label %for.cond16.preheader

for.cond16.preheader:                             ; preds = %for.cond16.preheader.preheader, %for.inc36
  %indvars.iv99 = phi i64 [ %indvars.iv.next100, %for.inc36 ], [ 0, %for.cond16.preheader.preheader ]
  %s.087 = phi i32 [ %add.lcssa.lcssa, %for.inc36 ], [ 0, %for.cond16.preheader.preheader ]
  br label %for.cond19.preheader

for.cond19.preheader:                             ; preds = %for.end28, %for.cond16.preheader
  %indvars.iv96 = phi i64 [ 0, %for.cond16.preheader ], [ %indvars.iv.next97, %for.end28 ]
  %s.185 = phi i32 [ %s.087, %for.cond16.preheader ], [ %add.lcssa, %for.end28 ]
  br label %for.body21

for.body21:                                       ; preds = %for.body21, %for.cond19.preheader
  %indvars.iv93 = phi i64 [ 0, %for.cond19.preheader ], [ %indvars.iv.next94, %for.body21 ]
  %s.283 = phi i32 [ %s.185, %for.cond19.preheader ], [ %add, %for.body21 ]
  %arrayidx25 = getelementptr inbounds [20 x [20 x i32]], ptr @a, i64 0, i64 %indvars.iv93, i64 %indvars.iv99, !intel-tbaa !2
  %0 = load i32, ptr %arrayidx25, align 4, !tbaa !2
  %add = add nsw i32 %0, %s.283
  %indvars.iv.next94 = add nuw nsw i64 %indvars.iv93, 1
  %exitcond95 = icmp eq i64 %indvars.iv.next94, 10
  br i1 %exitcond95, label %for.end28, label %for.body21

for.end28:                                        ; preds = %for.body21
  %add.lcssa = phi i32 [ %add, %for.body21 ]
  %arrayidx32 = getelementptr inbounds [20 x [20 x i32]], ptr @b, i64 0, i64 %indvars.iv96, i64 %indvars.iv99, !intel-tbaa !2
  store i32 %add.lcssa, ptr %arrayidx32, align 4, !tbaa !2
  %indvars.iv.next97 = add nuw nsw i64 %indvars.iv96, 1
  %exitcond98 = icmp eq i64 %indvars.iv.next97, 10
  br i1 %exitcond98, label %for.inc36, label %for.cond19.preheader

for.inc36:                                        ; preds = %for.end28
  %add.lcssa.lcssa = phi i32 [ %add.lcssa, %for.end28 ]
  %indvars.iv.next100 = add nuw nsw i64 %indvars.iv99, 1
  %exitcond101 = icmp eq i64 %indvars.iv.next100, 10
  br i1 %exitcond101, label %for.cond42.preheader.preheader, label %for.cond16.preheader

for.cond42.preheader.preheader:                   ; preds = %for.inc36
  br label %for.cond42.preheader

for.cond42.preheader:                             ; preds = %for.cond42.preheader.preheader, %for.end51
  %indvars.iv90 = phi i64 [ %indvars.iv.next91, %for.end51 ], [ 0, %for.cond42.preheader.preheader ]
  br label %for.body44

for.body44:                                       ; preds = %for.body44, %for.cond42.preheader
  %indvars.iv = phi i64 [ 0, %for.cond42.preheader ], [ %indvars.iv.next, %for.body44 ]
  %arrayidx48 = getelementptr inbounds [20 x [20 x i32]], ptr @b, i64 0, i64 %indvars.iv90, i64 %indvars.iv, !intel-tbaa !2
  %1 = load i32, ptr %arrayidx48, align 4, !tbaa !2
  %call = tail call i32 (ptr, ...) @printf(ptr @.str, i32 %1)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end51, label %for.body44

for.end51:                                        ; preds = %for.body44
  %putchar = tail call i32 @putchar(i32 10)
  %indvars.iv.next91 = add nuw nsw i64 %indvars.iv90, 1
  %exitcond92 = icmp eq i64 %indvars.iv.next91, 10
  br i1 %exitcond92, label %for.end55, label %for.cond42.preheader

for.end55:                                        ; preds = %for.end51
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: nounwind
declare i32 @putchar(i32) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7ff8c8bec53b2e9d8cff7a7ea8017ba074ab1feb) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 6c7b35a2833d93ccbfffa481b951cef0dc6bc5ad)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA20_A20_i", !4, i64 0}
!4 = !{!"array@_ZTSA20_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
