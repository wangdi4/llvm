;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-reroll,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s

; Check that Loop Distribution succeeds for distribute point with safe reduction.
; Old compiler was asserting because stale SafeReductionInfo could not find the new
; loop created after distribution.

; Before Distribution
;        BEGIN REGION { }
;              + DO i1 = 0, trunc.i64.i32((((-1 * ptrtoint.double*.i64(%val_ptr)) + ptrtoint.double*.i64(%val_end_of_row)) /u 8)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
;              |   %1 = (%colnum_ptr)[i1];
;              |   %2 = (%src)[%1];
;              |   %4 = (%val_ptr)[i1]; <distribute_point>
;              |   %mul = %4  *  %2;
;              |   %sum.011 = %mul  +  %sum.011; <Safe Reduction>
;              + END LOOP
;        END REGION


; After Distribution

; CHECK:  BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, (trunc.i64.i32((((-1 * ptrtoint.ptr.i64(%val_ptr)) + ptrtoint.ptr.i64(%val_end_of_row)) /u 8)) + -1)/u64, 1
;              |   %min = (-64 * i1 + trunc.i64.i32((((-1 * ptrtoint.double*.i64(%val_ptr)) + ptrtoint.double*.i64(%val_end_of_row)) /u 8)) + -1 <= 63) ? -64 * i1 + trunc.i64.i32((((-1 * ptrtoint.double*.i64(%val_ptr)) + ptrtoint.double*.i64(%val_end_of_row)) /u 8)) + -1 : 63;
;              |
; CHECK:       |   + DO i2 = 0, %min, 1   <DO_LOOP>
;              |   |   %1 = (%colnum_ptr)[64 * i1 + i2];
;              |   |   %2 = (%src)[%1];
;              |   |   (%.TempArray)[0][i2] = %2;
;              |   + END LOOP
;              |
;              |
; CHECK:       |   + DO i2 = 0, %min, 1   <DO_LOOP>
;              |   |   %2 = (%.TempArray)[0][i2];
;              |   |   %4 = (%val_ptr)[64 * i1 + i2];
;              |   |   %mul = %4  *  %2;
; CHECK:       |   |   %sum.011 = %mul  +  %sum.011;
;              |   + END LOOP
;              + END LOOP
; CHECK:  END REGION





target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local double @sub(ptr noundef %val_ptr, ptr noundef %val_end_of_row, ptr nocapture noundef readonly %src, ptr nocapture noundef readonly %colnum_ptr) local_unnamed_addr #0 {
entry:
  %sub.ptr.lhs.cast = ptrtoint ptr %val_end_of_row to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %val_ptr to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %0 = lshr exact i64 %sub.ptr.sub, 3
  %conv = trunc i64 %0 to i32
  %cmp10 = icmp sgt i32 %conv, 0
  br i1 %cmp10, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %add.lcssa = phi double [ %add, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %sum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add.lcssa, %for.cond.cleanup.loopexit ]
  ret double %sum.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %j.014 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %val_ptr.addr.013 = phi ptr [ %incdec.ptr2, %for.body ], [ %val_ptr, %for.body.preheader ]
  %colnum_ptr.addr.012 = phi ptr [ %incdec.ptr, %for.body ], [ %colnum_ptr, %for.body.preheader ]
  %sum.011 = phi double [ %add, %for.body ], [ 0.000000e+00, %for.body.preheader ]
  %incdec.ptr = getelementptr inbounds i32, ptr %colnum_ptr.addr.012, i64 1
  %1 = load i32, ptr %colnum_ptr.addr.012, align 4, !tbaa !3
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds double, ptr %src, i64 %idxprom
  %2 = load double, ptr %arrayidx, align 8, !tbaa !7
  %3 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %incdec.ptr2 = getelementptr inbounds double, ptr %val_ptr.addr.013, i64 1
  %4 = load double, ptr %val_ptr.addr.013, align 8, !tbaa !7
  %mul = fmul fast double %4, %2
  %add = fadd fast double %mul, %sum.011
  call void @llvm.directive.region.exit(token %3) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %inc = add nuw nsw i32 %j.014, 1
  %exitcond.not = icmp eq i32 %inc, %conv
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !9
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
