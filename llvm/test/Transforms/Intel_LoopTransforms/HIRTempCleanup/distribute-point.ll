; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" 2>&1 | FileCheck %s

; Verify that we bail out on propagating stores when the loop has a distribute point.

; CHECK: Function

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK: |   %0 = (@b)[0][i1];
; CHECK: |   %1 = (@c)[0][i1];
; CHECK: |   %sub = %0  -  %1;
; CHECK: |   (@a)[0][i1] = %sub;
; CHECK: |   %3 = (@a)[0][i1]; <distribute_point>
; CHECK: |   %4 = (@c)[0][i1];
; CHECK: |   %mul = %3  *  %4;
; CHECK: |   (@a)[0][i1] = %mul;
; CHECK: |   %5 = (@a)[0][i1];
; CHECK: |   %6 = (@c)[0][i1];
; CHECK: |   %div = %5  /  %6;
; CHECK: |   (@a)[0][i1] = %div;
; CHECK: + END LOOP

; CHECK: Function

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK: |   %0 = (@b)[0][i1];
; CHECK: |   %1 = (@c)[0][i1];
; CHECK: |   %sub = %0  -  %1;
; CHECK: |   (@a)[0][i1] = %sub;
; CHECK: |   %3 = (@a)[0][i1]; <distribute_point>
; CHECK: |   %4 = (@c)[0][i1];
; CHECK: |   %mul = %3  *  %4;
; CHECK: |   (@a)[0][i1] = %mul;
; CHECK: |   %5 = (@a)[0][i1];
; CHECK: |   %6 = (@c)[0][i1];
; CHECK: |   %div = %5  /  %6;
; CHECK: |   (@a)[0][i1] = %div;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp37 = icmp sgt i32 %n, 0
  br i1 %cmp37, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x float], ptr @b, i64 0, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [1000 x float], ptr @c, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx2, align 4
  %arrayidx4 = getelementptr inbounds [1000 x float], ptr @a, i64 0, i64 %indvars.iv
  %sub = fsub float %0, %1
  store float %sub, ptr %arrayidx4, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %3 = load float, ptr %arrayidx4, align 4
  %4 = load float, ptr %arrayidx2, align 4
  %mul = fmul float %3, %4
  store float %mul, ptr %arrayidx4, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %5 = load float, ptr %arrayidx4, align 4
  %6 = load float, ptr %arrayidx2, align 4
  %div = fdiv float %5, %6
  store float %div, ptr %arrayidx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

