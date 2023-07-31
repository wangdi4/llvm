; RUN:  opt -passes=slp-vectorizer %s
; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "/export/iusers/pchawla/ics/loopopt2/llvm/lib/Target/X86/X86ISelLowering.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: inlinehint nounwind ssp uwtable
define hidden fastcc void @"_ZZ41lowerV8I16GeneralSingleInputVectorShuffleRKN4llvm5SDLocENS_3MVTENS_7SDValueENS_15MutableArrayRefIiEERKNS_12X86SubtargetERNS_12SelectionDAGEENK4$_60clENS_8ArrayRefIiEESE_SE_SE_ii"(ptr %AToBInputs.0) unnamed_addr #0 align 2 {
entry:
  %gepload20 = load i32, ptr %AToBInputs.0, align 4, !tbaa !1
  %0 = shl i32 undef, 1
  %hir.cmp.92 = icmp eq i32 %gepload20, %0
  %1 = zext i1 %hir.cmp.92 to i32
  %arrayIdx23 = getelementptr inbounds i32, ptr %AToBInputs.0, i64 1
  %gepload24 = load i32, ptr %arrayIdx23, align 4, !tbaa !1
  %hir.cmp.15 = icmp eq i32 %gepload24, %0
  %2 = zext i1 %hir.cmp.15 to i32
  %3 = or i32 %0, 1
  %hir.cmp.95 = icmp eq i32 %gepload20, %3
  %4 = zext i1 %hir.cmp.95 to i32
  %hir.cmp.30 = icmp eq i32 %gepload24, %3
  %5 = zext i1 %hir.cmp.30 to i32
  %6 = add nuw nsw i32 %2, %1
  %7 = add nuw nsw i32 %6, %4
  %add41 = add nuw nsw i32 %7, %5
  %cmp55 = icmp eq i32 %add41, 1
  unreachable
}

attributes #0 = { inlinehint nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4a7ee65f1c73d397dc23f4a8444c419ca3be43f3)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
