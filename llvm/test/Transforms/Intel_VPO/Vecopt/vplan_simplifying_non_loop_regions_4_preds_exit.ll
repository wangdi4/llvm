; RUN: opt < %s -VPlanDriver -vplan-predicator-report -disable-vplan-codegen \
; RUN:       -vplan-print-after-simplify-cfg 2>&1 | FileCheck %s --check-prefix=CHECK-PHI
;
; Tests that simplification of non-loop region with 4 predecessors for exit VPBB
; doesn't cause compfail.
; It should be revisited after adding verification that non-loop regions
; were built where they're required
;
; ModuleID = 'simplifying_non_loop_regions_4_preds_exit.ll'
source_filename = "simplifying_non_loop_regions_4_preds_exit.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = local_unnamed_addr global [1600 x i32] zeroinitializer, align 16

; CHECK-PHI-LABEL: {{%vp.*}} = phi [ i64 0, {{BB[0-9]+}} ], [ {{%vp.*}}, {{BB[0-9]+}} ]

; Function Attrs: noinline norecurse nounwind uwtable
define void @_Z3foov() local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %if.end15, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end15 ]
  %arrayidx = getelementptr inbounds [1600 x i32], [1600 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.end15

if.then:                                          ; preds = %for.body
  %div33 = udiv i32 %0, 3
  %arrayidx5 = getelementptr inbounds [1600 x i32], [1600 x i32]* @b, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4, !tbaa !2
  %cmp6 = icmp eq i32 %1, 10
  br i1 %cmp6, label %if.end15, label %if.then7

if.then7:                                         ; preds = %if.then
  %arrayidx11 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx11, align 4, !tbaa !2
  %mul = mul nsw i32 %2, %1
  %cmp12 = icmp sgt i32 %mul, 200
  %add = zext i1 %cmp12 to i32
  %spec.select = add nsw i32 %mul, %add
  br label %if.end15

if.end15:                                         ; preds = %if.then7, %if.then, %for.body
  %t.0 = phi i32 [ %div33, %if.then ], [ 0, %for.body ], [ %spec.select, %if.then7 ]
  %arrayidx17 = getelementptr inbounds [1600 x i32], [1600 x i32]* @b, i64 0, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx17, align 4, !tbaa !2
  %mul18 = mul nsw i32 %3, %t.0
  %arrayidx20 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %indvars.iv
  store i32 %mul18, i32* %arrayidx20, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1600
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %if.end15
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 9b5153adabe7b2c246d382035a5f4d5b61ae18a6) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4ebccf2f9f1ff4cdd33eaa6b8c9b2ff15694508d)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1600_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
