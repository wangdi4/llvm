; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-cg" -force-hir-cg 2>&1 | FileCheck %s

; Verify that noalias intrinsics are eliminated in HIR.
; Also verify that the IR verifer does not complain when we generate code by
; ommitting the intrinsics but retaining the metadata on loads/stores.

; TODO: process these intrinsics when they start getting used in alias analysis.

; CHECK: + DO i1 = 0, 4095, 1   <DO_LOOP>

; CHECK-NOT: llvm.experimental.noalias.scope.decl

; CHECK: |   %0 = (%y)[i1];
; CHECK: |   %conv.i = sitofp.i32.float(2 * %0);
; CHECK: |   (%x)[i1] = %conv.i;
; CHECK: |   %1 = (%y)[i1 + -1];
; CHECK: |   %conv.i15 = sitofp.i32.float(2 * %1);
; CHECK: |   (%x)[i1 + -1] = %conv.i15;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define dso_local void @foo(ptr nocapture %x, ptr nocapture readonly %y) local_unnamed_addr #1 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %i.016 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add.ptr = getelementptr inbounds float, ptr %x, i64 %i.016, !intel-tbaa !6
  %add.ptr1 = getelementptr inbounds i32, ptr %y, i64 %i.016, !intel-tbaa !2
  tail call void @llvm.experimental.noalias.scope.decl(metadata !8)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !11)
  %0 = load i32, ptr %add.ptr1, align 4, !tbaa !2, !alias.scope !11, !noalias !8
  %mul.i = shl nsw i32 %0, 1
  %conv.i = sitofp i32 %mul.i to float
  store float %conv.i, ptr %add.ptr, align 4, !tbaa !6, !alias.scope !8, !noalias !11
  %add.ptr3 = getelementptr inbounds float, ptr %add.ptr, i64 -1, !intel-tbaa !6
  %add.ptr5 = getelementptr inbounds i32, ptr %add.ptr1, i64 -1, !intel-tbaa !2
  tail call void @llvm.experimental.noalias.scope.decl(metadata !13)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !16)
  %1 = load i32, ptr %add.ptr5, align 4, !tbaa !2, !alias.scope !16, !noalias !13
  %mul.i14 = shl nsw i32 %1, 1
  %conv.i15 = sitofp i32 %mul.i14 to float
  store float %conv.i15, ptr %add.ptr3, align 4, !tbaa !6, !alias.scope !13, !noalias !16
  %inc = add nuw nsw i64 %i.016, 1
  %exitcond.not = icmp eq i64 %inc, 4096
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !18
}

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.experimental.noalias.scope.decl(metadata) #2

attributes #0 = { nofree norecurse nounwind uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nofree nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { inaccessiblememonly nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.2.0.YYYYMMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
!8 = !{!9}
!9 = distinct !{!9, !10, !"bar: %x"}
!10 = distinct !{!10, !"bar"}
!11 = !{!12}
!12 = distinct !{!12, !10, !"bar: %y"}
!13 = !{!14}
!14 = distinct !{!14, !15, !"bar: %x"}
!15 = distinct !{!15, !"bar"}
!16 = !{!17}
!17 = distinct !{!17, !15, !"bar: %y"}
!18 = distinct !{!18, !19}
!19 = !{!"llvm.loop.mustprogress"}
