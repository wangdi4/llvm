; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -print-after=hir-prefetching < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
; Source code
; #pragma  prefetch *:1:20
;  for (i=0; i< N; i++) {
;      A[i] = B[i] + C[M[i]];
;  }
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<33>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<13>               |   %1 = (@B)[0][i1];
;<15>               |   %2 = (@M)[0][i1];
;<18>               |   %3 = (@C)[0][%2];
;<21>               |   (@A)[0][i1] = %1 + %3;
;<33>               + END LOOP
;<33>
;<32>               ret undef;
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: foo
;
; CHECK:         BEGIN REGION { modified }
; CHECK-NEXT:        + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK-NEXT:           |   %1 = (@B)[0][i1];
; CHECK-NEXT:           |   %2 = (@M)[0][i1];
; CHECK-NEXT:           |   %3 = (@C)[0][%2];
; CHECK-NEXT:           |   (@A)[0][i1] = %1 + %3;
; CHECK-NEXT:           |   if (i1 + 20 <=u zext.i32.i64(%N) + -1)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      %Load = (@M)[0][i1 + 20];
; CHECK-NEXT:           |      @llvm.prefetch.p0i8(&((i8*)(@C)[0][%Load]),  0,  2,  1);
; CHECK-NEXT:           |   }
; CHECK-NEXT:           |   @llvm.prefetch.p0i8(&((i8*)(@B)[0][i1 + 20]),  0,  2,  1);
; CHECK-NEXT:           |   @llvm.prefetch.p0i8(&((i8*)(@M)[0][i1 + 20]),  0,  2,  1);
; CHECK-NEXT:           |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 20]),  0,  2,  1);
; CHECK-NEXT:           + END LOOP
;
; CHECK:                ret undef;
; CHECK:          END REGION
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -print-after=hir-prefetching -hir-cg -force-hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter < %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low -force-hir-cg 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT
;
; OPTREPORT:  LOOP BEGIN
; OPTREPORT:     remark #25018: Total number of lines prefetched=4
; OPTREPORT:     remark #25019: Number of spatial prefetches=3, default dist=20
; OPTREPORT:     remark #25033: Number of indirect prefetches=1, default dist=20
; OPTREPORT:     remark #25150: Using directive-based hint=1, distance=20 for indirect memory reference
; OPTREPORT:     remark #25147: Using directive-based hint=1, distance=20 for prefetching spatial memory reference
; OPTREPORT:     remark #25147: Using directive-based hint=1, distance=20 for prefetching spatial memory reference
; OPTREPORT:     remark #25147: Using directive-based hint=1, distance=20 for prefetching spatial memory reference
; OPTREPORT:  LOOP END

;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@M = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32 %N) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(i8* null), "QUAL.PRAGMA.HINT"(i32 1), "QUAL.PRAGMA.DISTANCE"(i32 20) ]
  %cmp12 = icmp sgt i32 %N, 0
  br i1 %cmp12, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count14 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %arrayidx2 = getelementptr inbounds [1000 x i32], [1000 x i32]* @M, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %2 = load i32, i32* %arrayidx2, align 4, !tbaa !3
  %idxprom3 = sext i32 %2 to i64
  %arrayidx4 = getelementptr inbounds [1000 x i32], [1000 x i32]* @C, i64 0, i64 %idxprom3, !intel-tbaa !3
  %3 = load i32, i32* %arrayidx4, align 4, !tbaa !3
  %add = add nsw i32 %3, %1
  %arrayidx6 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv, !intel-tbaa !3
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count14
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !8

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret i32 undef
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA1000_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
