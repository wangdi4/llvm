; Test for VPlanDriver not crashing for non-vector targets
; Test that vectorization does not kick in
; ModuleID = 't.c'
; RUN: opt -vplan-vec -S < %s | FileCheck %s
; RUN: opt -passes="vplan-vec" -S < %s | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vplan-vec -hir-cg -S  < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,hir-cg" -S < %s | FileCheck %s
; CHECK-NOT: load <{{.*}} x i32>
; CHECK-NOT: add <{{.*}} x i32>
; CHECK-NOT: store <{{.*}} x i32>
; CHECK: ret void
source_filename = "t.c"
target datalayout = "e-m:e-p:32:32-i64:32-f64:32-f128:32-n8:16:32-a:0:32-S32"
target triple = "i586-intel-elfiamcu"

@arr2 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr1 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %DIR.OMP.SIMD

DIR.OMP.SIMD:                                   ; preds = %simd.begin.region
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %0, 1
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %simd.end.region, label %for.body

simd.end.region:                                  ; preds = %simd.loop
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="lakemont" "unsafe-fp-math"="false" "use-soft-float"="true" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 81c1a2457ffaab4ce4cf6d764eae1942ff6b5fa2) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 349a376333b5a176d228613d078891dabc737bed)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
