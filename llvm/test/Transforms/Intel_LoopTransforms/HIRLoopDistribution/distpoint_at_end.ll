;RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-distribute-memrec" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that we successfully parse and handle distribute point which occurs at the
; end of the loop due to scalaropt removing redundant instructions after the pragma.
; Such loops should not be distributed, so we attach the distpoint to the first inst.

; C Source
; void f() {
;     for (int i = start; i < end; i++) {
;         A[i] = B[i];
; #pragma distribute_point
;         A[i] += 0;
;     }
; }

;   *** IR Dump Before HIR Loop Distribution MemRec (hir-loop-distribute-memrec) ***
;   Function: f
;   BEGIN REGION { }
;         + DO i1 = 0, zext.i32.i64((-1 + (-1 * %start) + %end)), 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;         |   %1 = (%B)[i1 + sext.i32.i64(%start)]; <distribute_point>
;         |   (%A)[i1 + sext.i32.i64(%start)] = %1;
;         + END LOOP
;   END REGION

;   *** IR Dump After HIR Loop Distribution MemRec (hir-loop-distribute-memrec) ***
;   Function: f

;   BEGIN REGION { }
;         + DO i1 = 0, zext.i32.i64((-1 + (-1 * %start) + %end)), 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;         |   %1 = (%B)[i1 + sext.i32.i64(%start)];
;         |   (%A)[i1 + sext.i32.i64(%start)] = %1;
;         + END LOOP
;   END REGION

; CHECK: BEGIN REGION { }
; CHECK-NEXT: DO i1
; CHECK-NEXT: %1 = (%B)[i1 + sext.i32.i64(%start)]; <distribute_point>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @f(ptr noalias nocapture %A, ptr noalias nocapture readonly %B, i32 %start, i32 %end) local_unnamed_addr #0 {
entry:
  %cmp11 = icmp slt i32 %start, %end
  br i1 %cmp11, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %start to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  %1 = load float, ptr %arrayidx, align 4, !tbaa !3
  %arrayidx2 = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  store float %1, ptr %arrayidx2, align 4, !tbaa !3
  %2 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond.not = icmp eq i32 %lftr.wideiv, %end
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !7
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %indvars.iv.i = phi i64 [ 0, %entry ], [ %indvars.iv.next.i, %for.body.i ]
  %0 = call token @llvm.directive.region.entry() #1 [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ], !noalias !9
  call void @llvm.directive.region.exit(token %0) #1 [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ], !noalias !9
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %1 = icmp eq i64 %indvars.iv.next.i, 100
  br i1 %1, label %f.exit, label %for.body.i, !llvm.loop !7

f.exit:                                           ; preds = %for.body.i
  ret i32 0
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = !{!10, !12}
!10 = distinct !{!10, !11, !"f: %A"}
!11 = distinct !{!11, !"f"}
!12 = distinct !{!12, !11, !"f: %B"}
