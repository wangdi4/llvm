; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-pragma-loop-blocking -print-after=hir-pragma-loop-blocking -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-pragma-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s
;
;
; Check linearized version of matmul works for pragma blocking
;
; Source Code:
; int i, j, k; // M = 1024
; #pragma block_loop factor(64) level(1:3)
;  for (i = 0; i < M; i++)
;    for (j = 0; j < M; j++)
;      for (k = 0; k < M; k++)
;         c[j + i *M] += a[k + i *M] * b[j + k*M];


; CHECK:  BEGIN REGION { modified }
; CHECK:    + DO i1 = 0, 15, 1   <DO_LOOP>
; CHECK:    |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK:    |   |   + DO i3 = 0, 15, 1   <DO_LOOP>
; CHECK:    |   |   |   + DO i4 = 0, 63, 1   <DO_LOOP>
; CHECK:    |   |   |   |   + DO i5 = 0, 63, 1   <DO_LOOP>
; CHECK:    |   |   |   |   |   + DO i6 = 0, 63, 1   <DO_LOOP>
; CHECK:    |   |   |   |   |   |   %add1638 = (@c)[0][65536 * i1 + 64 * i2 + 1024 * i4 + i5];
; CHECK:    |   |   |   |   |   |   %mul11 = (@a)[0][65536 * i1 + 64 * i3 + 1024 * i4 + i6]  *  (@b)[0][64 * i2 + 65536 * i3 + i5 + 1024 * i6];
; CHECK:    |   |   |   |   |   |   %add1638 = %add1638  +  %mul11;
; CHECK:    |   |   |   |   |   |   (@c)[0][65536 * i1 + 64 * i2 + 1024 * i4 + i5] = %add1638;
; CHECK:    |   |   |   |   |   + END LOOP
; CHECK:    |   |   |   |   + END LOOP
; CHECK:    |   |   |   + END LOOP
; CHECK:    |   |   + END LOOP
; CHECK:    |   + END LOOP
; CHECK:    + END LOOP

;Module Before HIR
source_filename = "mm.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1048576 x double] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [1048576 x double] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [1048576 x double] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @_Z8multiplyv() local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 64), "QUAL.PRAGMA.LEVEL"(i32 2), "QUAL.PRAGMA.FACTOR"(i32 64), "QUAL.PRAGMA.LEVEL"(i32 3), "QUAL.PRAGMA.FACTOR"(i32 64) ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc20, %entry
  %indvars.iv46 = phi i64 [ 0, %entry ], [ %indvars.iv.next47, %for.inc20 ]
  %1 = shl nsw i64 %indvars.iv46, 10
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc17, %for.cond1.preheader
  %indvars.iv42 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next43, %for.inc17 ]
  %2 = add nuw nsw i64 %indvars.iv42, %1
  %arrayidx15 = getelementptr inbounds [1048576 x double], [1048576 x double]* @c, i64 0, i64 %2, !intel-tbaa !2
  %arrayidx15.promoted = load double, double* %arrayidx15, align 8, !tbaa !2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %add1638 = phi double [ %arrayidx15.promoted, %for.cond4.preheader ], [ %add16, %for.body6 ]
  %3 = add nuw nsw i64 %indvars.iv, %1
  %arrayidx = getelementptr inbounds [1048576 x double], [1048576 x double]* @a, i64 0, i64 %3, !intel-tbaa !2
  %4 = load double, double* %arrayidx, align 8, !tbaa !2
  %5 = shl nuw nsw i64 %indvars.iv, 10
  %6 = add nuw nsw i64 %5, %indvars.iv42
  %arrayidx10 = getelementptr inbounds [1048576 x double], [1048576 x double]* @b, i64 0, i64 %6, !intel-tbaa !2
  %7 = load double, double* %arrayidx10, align 8, !tbaa !2
  %mul11 = fmul double %4, %7
  %add16 = fadd double %add1638, %mul11
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.inc17, label %for.body6

for.inc17:                                        ; preds = %for.body6
  %add16.lcssa = phi double [ %add16, %for.body6 ]
  store double %add16.lcssa, double* %arrayidx15, align 8, !tbaa !2
  %indvars.iv.next43 = add nuw nsw i64 %indvars.iv42, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next43, 1024
  br i1 %exitcond45, label %for.inc20, label %for.cond4.preheader

for.inc20:                                        ; preds = %for.inc17
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next47, 1024
  br i1 %exitcond49, label %for.end22, label %for.cond1.preheader

for.end22:                                        ; preds = %for.inc20
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1048576_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
