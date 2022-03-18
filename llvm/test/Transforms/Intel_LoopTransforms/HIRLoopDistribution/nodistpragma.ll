;RUN: opt -hir-ssa-deconstruction -hir-loop-distribute-memrec -hir-cg -force-hir-cg -intel-ir-optreport-emitter -intel-opt-report=low -disable-output < %s 2>&1 | FileCheck %s
;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,hir-cg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -force-hir-cg -intel-opt-report=low -disable-output   < %s 2>&1 | FileCheck %s

; Check that pragma before first statement in loop results in no distribution, marked
; in opt-report

; Incoming HIR:
;   BEGIN REGION { }
;         + DO i1 = 0, 1022, 1   <DO_LOOP>
;         |   %conv = sitofp.i32.double(i1); <distribute_point>
;         |   %2 = (%x)[i1];
;         |   %add = %2  +  %conv;
;         |   (%c)[i1] = %add;
;         |   %add6 = %add  +  %conv; <distribute_point>
;         |   (%x)[i1 + 1] = %add6;
;         + END LOOP
;   END REGION
;
;
;        Global loop optimization report for : loop_distribution_pragma2
;
;        LOOP BEGIN
; CHECK:   remark #25482: No Distribution as requested by pragma
;        LOOP END



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @loop_distribution_pragma2(double* noalias nocapture %c, double* noalias nocapture %x) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %1 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %1 to double
  %arrayidx = getelementptr inbounds double, double* %x, i64 %indvars.iv
  %2 = load double, double* %arrayidx, align 8, !tbaa !3
  %add = fadd fast double %2, %conv
  %arrayidx2 = getelementptr inbounds double, double* %c, i64 %indvars.iv
  store double %add, double* %arrayidx2, align 8, !tbaa !3
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %add6 = fadd fast double %add, %conv
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx9 = getelementptr inbounds double, double* %x, i64 %indvars.iv.next
  store double %add6, double* %arrayidx9, align 8, !tbaa !3
  call void @llvm.directive.region.exit(token %3) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1023
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !7

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
