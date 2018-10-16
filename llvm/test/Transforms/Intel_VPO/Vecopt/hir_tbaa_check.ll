; LLVM IR generated from testcase below using icx -O1 -S -emit-llvm
; int arr1[1024], arr2[1024];
;
; void foo()
; {
;   int i;
;
;   for (i = 0; i < 1024; i++)
;     arr1[i] = arr2[i] + 1;
; }
;
; Test to check that we preserve tbaa metadata.
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -hir-cg -S -print-after=VPlanDriverHIR  < %s 2>&1  | FileCheck %s
; CHECK: DO i1 = 0, 1023, 4
; CHECK:  %[[ARRIDX:.*]] = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %1
; CHECK:  %[[BITCAST:.*]] = bitcast i32* %[[ARRIDX]] to <4 x i32>*
; CHECK:  %{{.*}} = load <4 x i32>, <4 x i32>* %[[BITCAST]], align 4, !tbaa ![[TBAA:.*]]
; CHECK:  %[[ARRIDX1:.*]] = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %3
; CHECK:  %[[BITCAST1:.*]] = bitcast i32* %[[ARRIDX1]] to <4 x i32>*
; CHECK:  store <4 x i32> {{.*}}, <4 x i32>* %[[BITCAST1]], align 4, !tbaa ![[TBAA]]
; CHECK: ![[TBAA]] = !{![[TBAA1:.*]], !{{.*}}
; CHECK: ![[TBAA1]] = !{!"array@_ZTSA1024_i", {{.*}}

; ModuleID = 'hir_tbaa.c'
source_filename = "hir_tbaa.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr2 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr1 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %0, 1
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 8917aed201bae2133195342c218e4ac7e137d59c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 659cf604211fb998f5a4885ce1a45a26ab4754e9)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
