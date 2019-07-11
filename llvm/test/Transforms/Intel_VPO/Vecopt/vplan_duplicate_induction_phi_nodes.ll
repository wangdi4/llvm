; Check if VPLoopEntities framework is able to handle duplicates of induction PHIs.

; Case 1 - Duplicate PHI is completely identical to original. So duplicate PHI should not be imported and all its uses
; within loop is replaced with original.
; Case 2 - Duplicate PHI is partially identical to original. So it should not be imported as an entity, and left behind
; for regular vectorization.

; RUN: opt -loopopt=0 -VPlanDriver -vpo-vplan-build-stress-test -vplan-print-after-hcfg -vplan-entities-dump < %s 2>&1 | FileCheck %s

; Checks for Case 1
; CHECK-LABEL: Print after building H-CFG
; CHECK: Induction list
; CHECK-NEXT: IntInduction(+) Start: i64 0 Step: i64 1 BinOp: i64 [[ADD:%vp.*]] = add i64 [[PHI1:%vp.*]] i64 1
; CHECK: i64 [[PHI1]] = phi  [ i64 0, [[PH:BB.*]] ],  [ i64 [[ADD]], [[LATCH:BB.*]] ]
; CHECK-NEXT: i64 [[PHI2:%vp.*]] = phi  [ i64 0, [[PH]] ],  [ i64 [[ADD]], [[LATCH]] ]
; CHECK-NEXT: i64 {{%vp.*}} = and i64 [[PHI1]] i64 1

; Checks for Case 2
; CHECK-LABEL: Print after building H-CFG
; CHECK: Induction list
; CHECK-NEXT: IntInduction(+) Start: i64 1 Step: i64 1 BinOp: i64 [[ADD:%vp.*]] = add i64 [[PHI1:%vp.*]] i64 1
; CHECK: i64 [[PHI1]] = phi  [ i64 1, [[PH:BB.*]] ],  [ i64 [[ADD]], [[LATCH:BB.*]] ]
; CHECK-NEXT: i64 [[PHI2:%vp.*]] = phi  [ i64 0, [[PH]] ],  [ i64 [[PHI1]], [[LATCH]] ]
; CHECK-NEXT: i64 {{%vp.*}} = and i64 [[PHI2]] i64 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* nocapture %a, i32* nocapture %b, i32* nocapture readonly %c, i32 %N) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %DIR.OMP.SIMD.121, label %omp.precond.end

DIR.OMP.SIMD.121:                                 ; preds = %entry
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.121
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = add i32 %N, -1
  %wide.trip.count = sext i32 %N to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.inc ]
  %indvars.iv.2 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.inc ]
  %rem2026 = and i64 %indvars.iv.2, 1
  %cmp6 = icmp eq i64 %rem2026, 0
  br i1 %cmp6, label %if.else, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add7 = add nsw i32 %2, 5
  %arrayidx9 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  store i32 %add7, i32* %arrayidx9, align 4, !tbaa !2
  br label %omp.inner.for.inc

if.else:                                          ; preds = %omp.inner.for.body
  %arrayidx11 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx11, align 4, !tbaa !2
  %mul12 = mul nsw i32 %3, 5
  %arrayidx14 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  store i32 %mul12, i32* %arrayidx14, align 4, !tbaa !2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.inc
  store i32 %1, i32* %i.lpriv, align 4, !tbaa !2
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

define dso_local void @foo1(i32* nocapture %a, i32* nocapture %b, i32* nocapture readonly %c, i32 %N) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %DIR.OMP.SIMD.121, label %omp.precond.end

DIR.OMP.SIMD.121:                                 ; preds = %entry
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.121
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = add i32 %N, -1
  %wide.trip.count = sext i32 %N to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ 1, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.inc ]
  %indvars.iv.2 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv, %omp.inner.for.inc ]
  %rem2026 = and i64 %indvars.iv.2, 1
  %cmp6 = icmp eq i64 %rem2026, 0
  br i1 %cmp6, label %if.else, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add7 = add nsw i32 %2, 5
  %arrayidx9 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  store i32 %add7, i32* %arrayidx9, align 4, !tbaa !2
  br label %omp.inner.for.inc

if.else:                                          ; preds = %omp.inner.for.body
  %arrayidx11 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx11, align 4, !tbaa !2
  %mul12 = mul nsw i32 %3, 5
  %arrayidx14 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  store i32 %mul12, i32* %arrayidx14, align 4, !tbaa !2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.inc
  store i32 %1, i32* %i.lpriv, align 4, !tbaa !2
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

attributes #1 = { nounwind }


!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

