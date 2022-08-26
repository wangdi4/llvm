;
; RUN: opt -tbaa -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -hir-details < %s 2>&1 | FileCheck %s --check-prefix=HIRCHECK
; RUN: opt -vplan-vec -disable-output -print-after=vplan-vec  < %s 2>&1  | FileCheck %s --check-prefix=LLVMCHECK
;
; LIT test to check that alignment is being set correctly for VLS group load and store.
; The alignment value to be used needs to come from the alignment of the first memory
; reference(lowest offset reference) in the group. For the test case below, the alignment
; to be used needs to be 8.
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.f2 = type { i32, i32 }

@farr = external dso_local local_unnamed_addr global [100 x %struct.f2], align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

; HIRCHECK:    DO i64 i1 = 0, 99, 4   <DO_LOOP>
; LLVMCHECK: vector.body:
for.body:                                         ; preds = %entry, %for.body
  %l1.014 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %b = getelementptr inbounds [100 x %struct.f2], [100 x %struct.f2]* @farr, i64 0, i64 %l1.014, i32 1, !intel-tbaa !2
;
; HIRCHECK:    [[GEP_BASE:%.*]] = &((i32*)(@farr)[0][i1].1);
; HIRCHECK:    <RVAL-REG> {al:8}(<8 x i32>*)(NON-LINEAR i32* [[GEP_BASE]])[i64 -1]
; LLVMCHECK:   %{{.*}} = load <8 x i32>, <8 x i32>* %{{.*}}, align 8
;
  %0 = load i32, i32* %b, align 4, !tbaa !2
  %a = getelementptr inbounds [100 x %struct.f2], [100 x %struct.f2]* @farr, i64 0, i64 %l1.014, i32 0, !intel-tbaa !8
  %1 = load i32, i32* %a, align 8, !tbaa !8
  %add = add nsw i32 %1, %0
;
; HIRCHECK:    (<8 x i32>*)(@farr)[0][i1].0 = %{{.*}};
; HIRCHECK:    <LVAL-REG> {al:8}(<8 x i32>*)(LINEAR [100 x %struct.f2]* @farr)[i64 0][LINEAR i64 i1].0
; LLVMCHECK:   store <8 x i32> %{{.*}}, <8 x i32>* %{{.*}}, align 8
;
  store i32 %add, i32* %b, align 4, !tbaa !2
  store i32 %add, i32* %a, align 8, !tbaa !8
  %inc = add nuw nsw i64 %l1.014, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !9

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() #1
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nofree norecurse nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!2 = !{!3, !5, i64 4}
!3 = !{!"array@_ZTSA100_2f2", !4, i64 0}
!4 = !{!"struct@f2", !5, i64 0, !5, i64 4}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!3, !5, i64 0}
!9 = distinct !{!9, !10, !11}
!10 = !{!"llvm.loop.mustprogress"}
!11 = !{!"llvm.loop.unroll.disable"}
