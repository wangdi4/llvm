;RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate" -disable-output -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate %s -hir-details 2>&1 | FileCheck %s
;
; Make sure HIROptVarPredicate doesn't get choked. When the source types of LHS/RHS are different HIROptVarPredicate bails out.
;
;CHECK: Function: foo
;
;CHECK:          BEGIN REGION { }
;CHECK:                + DO i64 i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;CHECK:                |
;CHECK:                |   if (i1 + -1 == 0)
;CHECK:                |   <RVAL-REG> LINEAR trunc.i64.i32(i1 + -1) {sb:2}
;CHECK:                |   else
;CHECK:                + END LOOP
;CHECK:          END REGION
;
;CHECK: Function: foo
;
;CHECK:          BEGIN REGION { }
;CHECK:                + DO i64 i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;CHECK:                |
;CHECK:                |   if (i1 + -1 == 0)
;CHECK:                |   <RVAL-REG> LINEAR trunc.i64.i32(i1 + -1) {sb:2}
;CHECK:                |   else
;CHECK:                + END LOOP
;CHECK:          END REGION
;
;
;Module Before HIR
; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind writeonly uwtable
define dso_local void @foo(ptr nocapture noundef writeonly %p, ptr nocapture noundef writeonly %q, i32 noundef %N) local_unnamed_addr #0 {
entry:
  %cmp12 = icmp sgt i32 %N, 0
  br i1 %cmp12, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %N to i64
  %arrayidx = getelementptr inbounds float, ptr %p, i64 1
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %myadd = add i64 %indvars.iv, -1
  %trunc = trunc i64 %myadd to i32
  %cmp1 = icmp eq i32 %trunc, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  store float 1.000000e+00, ptr %arrayidx, align 4, !tbaa !3
  br label %for.inc

if.else:                                          ; preds = %for.body
  %0 = trunc i64 %indvars.iv to i32
  %conv2 = sitofp i32 %0 to float
  %arrayidx4 = getelementptr inbounds float, ptr %q, i64 %indvars.iv
  store float %conv2, ptr %arrayidx4, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !7
}

attributes #0 = { argmemonly nofree norecurse nosync nounwind writeonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
