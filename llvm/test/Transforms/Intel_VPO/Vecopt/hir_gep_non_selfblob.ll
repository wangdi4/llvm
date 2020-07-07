; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -enable-vp-value-codegen-hir -vplan-force-vf=4 -disable-output -tbaa -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s
;
; The following test computes an address, does a bitcast of the address, and then uses
; the bitcast result to compute yet another address. HIR requires the base canon
; expression to be a self blob. If the ref corresponding to the same is not a self
; blob, we need to generate a copy instruction and use the lval of the copy as the
; base canon expression. The test checks correct vector code generation for this
; case.
;
; Scalar HIR:
;      DO i1 = 0, 99, 1   <DO_LOOP>
;         %0 = (%neighbor)[i1];
;         %1 = bitcast.%struct.site*.i8*(&((%lattice)[%0]));
;         (%dest)[i1] = &((%1)[%field]);
;      END LOOP
;
; Relevant VPInstructions look like:
;   %struct.site* %vp18448 = getelementptr inbounds %struct.site* %lattice i64 %vp5968
;   i8* %vp5760 = bitcast %struct.site* %vp18448
;   i8* %vp18704 = getelementptr inbounds i8* %vp5760 i64 %field
; When generating ref for %vp18704, we need to copy the ref corresponding to %vp18448
; (with bitcast folded in), and use the LVAL of the copy instruction to generate the
; new addressof ref.
;
; CHECK-LABEL:  *** IR Dump After VPlan Vectorization Driver HIR ***
; CHECK:                     DO i1 = 0, 99, 4   <DO_LOOP> <novectorize>
; CHECK-NEXT:                  %.vec = (<4 x i64>*)(%neighbor)[i1];
; CHECK-NEXT:                  %nsbgepcopy = &((<4 x i8*>)(%lattice)[%.vec]);
; CHECK-NEXT:                  (<4 x i8*>*)(%dest)[i1] = &((<4 x i8*>)(%nsbgepcopy)[%field]);
; CHECK-NEXT:                END LOOP
;
%struct.site = type { i16, i16, i16, i16 }

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @start_gather(i64 %field, i64* nocapture readonly %neighbor, i8** nocapture %dest, %struct.site* %lattice) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %neighbor, i64 %indvars.iv
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !2
  %add.ptr = getelementptr inbounds %struct.site, %struct.site* %lattice, i64 %0, !intel-tbaa !6
  %1 = bitcast %struct.site* %add.ptr to i8*
  %add.ptr1 = getelementptr inbounds i8, i8* %1, i64 %field, !intel-tbaa !9
  %arrayidx3 = getelementptr inbounds i8*, i8** %dest, i64 %indvars.iv
  store i8* %add.ptr1, i8** %arrayidx3, align 8, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"struct@", !8, i64 0, !8, i64 2, !8, i64 4, !8, i64 6}
!8 = !{!"short", !4, i64 0}
!9 = !{!4, !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"pointer@_ZTSPc", !4, i64 0}
