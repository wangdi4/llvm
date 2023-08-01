; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; HIR Loop Collapse Sanity Test: testcase reduced from icc's loopcollapse test - collaps_004.c
;
; Note:
; the original function is big with 2 large loopnests. They are broken into 2 different testcases as
; collapse_004.ll and collapse_004.2.ll;
;
; This testcase triggers loop collapse on i2-i3 level.
;
; *** Source Code ***
;int foo(void) {
;  int i, j, k, n, res;
;  int a1[15][15][15], b1[15][15][15], c1[15][15], d1[15][15][15];
;  int a2[15][15][15], b2[15][15][15], c2[15][15], d2[15][15][15];
;  int a3[15][15][15], b3[15][15][15], c3[15][15], d3[15][15][15];
;
;  for (k = 0; k < 15; k++) {
;    for (i = 0; i < 15; i++) {
;      for (j = 0; j < 15; j++) {
;        a1[k][i][j] = 1;
;        a2[k][i][j] = 1;
;        a3[k][i][j] = 1;
;        b1[k][i][j] = 1;
;        b2[k][i][j] = 1;
;        b3[k][i][j] = 1;
;        c1[i][j] = 1;
;        c2[i][j] = 1;
;        c3[i][j] = 1;
;        d1[k][i][j] = 1;
;        d2[k][i][j] = 1;
;        d3[k][i][j] = 1;
;      }
;    }
;  }
;
;  return a1[0][0][0] + b2[1][1][1] + c3[2][2] + d2[3][3][3];
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 14, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 14, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 14, 1   <DO_LOOP>
; CHECK:        |   |   |   (%a1)[0][i1][i2][i3] = 1;
; CHECK:        |   |   |   (%b2)[0][i1][i2][i3] = 1;
; CHECK:        |   |   |   (%c3)[0][i2][i3] = 1;
; CHECK:        |   |   |   (%d2)[0][i1][i2][i3] = 1;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 14, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 224, 1   <DO_LOOP>
; CHECK:        |   |   (%a1)[0][i1][0][i2] = 1;
; CHECK:        |   |   (%b2)[0][i1][0][i2] = 1;
; CHECK:        |   |   (%c3)[0][0][i2] = 1;
; CHECK:        |   |   (%d2)[0][i1][0][i2] = 1;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  %a1 = alloca [15 x [15 x [15 x i32]]], align 16
  %b2 = alloca [15 x [15 x [15 x i32]]], align 16
  %d2 = alloca [15 x [15 x [15 x i32]]], align 16
  %c3 = alloca [15 x [15 x i32]], align 16
  call void @llvm.lifetime.start.p0(i64 13500, ptr nonnull %a1) #2
  call void @llvm.lifetime.start.p0(i64 13500, ptr nonnull %b2) #2
  call void @llvm.lifetime.start.p0(i64 13500, ptr nonnull %d2) #2
  call void @llvm.lifetime.start.p0(i64 900, ptr nonnull %c3) #2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc74, %entry
  %indvars.iv137 = phi i64 [ 0, %entry ], [ %indvars.iv.next138, %for.inc74 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc71, %for.cond1.preheader
  %indvars.iv134 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next135, %for.inc71 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %a1, i64 0, i64 %indvars.iv137, i64 %indvars.iv134, i64 %indvars.iv
  store i32 1, ptr %arrayidx10, align 4, !tbaa !1
  %arrayidx34 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %b2, i64 0, i64 %indvars.iv137, i64 %indvars.iv134, i64 %indvars.iv
  store i32 1, ptr %arrayidx34, align 4, !tbaa !1
  %arrayidx52 = getelementptr inbounds [15 x [15 x i32]], ptr %c3, i64 0, i64 %indvars.iv134, i64 %indvars.iv
  store i32 1, ptr %arrayidx52, align 4, !tbaa !1
  %arrayidx64 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %d2, i64 0, i64 %indvars.iv137, i64 %indvars.iv134, i64 %indvars.iv
  store i32 1, ptr %arrayidx64, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 15
  br i1 %exitcond, label %for.inc71, label %for.body6

for.inc71:                                        ; preds = %for.body6
  %indvars.iv.next135 = add nuw nsw i64 %indvars.iv134, 1
  %exitcond136 = icmp eq i64 %indvars.iv.next135, 15
  br i1 %exitcond136, label %for.inc74, label %for.cond4.preheader

for.inc74:                                        ; preds = %for.inc71
  %indvars.iv.next138 = add nuw nsw i64 %indvars.iv137, 1
  %exitcond139 = icmp eq i64 %indvars.iv.next138, 15
  br i1 %exitcond139, label %for.end76, label %for.cond1.preheader

for.end76:                                        ; preds = %for.inc74
  %0 = load i32, ptr %a1, align 16, !tbaa !1
  %arrayidx82 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %b2, i64 0, i64 1, i64 1, i64 1
  %1 = load i32, ptr %arrayidx82, align 4, !tbaa !1
  %add = add nsw i32 %1, %0
  %arrayidx84 = getelementptr inbounds [15 x [15 x i32]], ptr %c3, i64 0, i64 2, i64 2
  %2 = load i32, ptr %arrayidx84, align 8, !tbaa !1
  %add85 = add nsw i32 %add, %2
  %arrayidx88 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %d2, i64 0, i64 3, i64 3, i64 3
  %3 = load i32, ptr %arrayidx88, align 4, !tbaa !1
  %add89 = add nsw i32 %add85, %3
  call void @llvm.lifetime.end.p0(i64 900, ptr nonnull %c3) #2
  call void @llvm.lifetime.end.p0(i64 13500, ptr nonnull %d2) #2
  call void @llvm.lifetime.end.p0(i64 13500, ptr nonnull %b2) #2
  call void @llvm.lifetime.end.p0(i64 13500, ptr nonnull %a1) #2
  ret i32 %add89
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #0 = { nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21400) (llvm/branches/loopopt 21436)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
