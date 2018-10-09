; LLVM IR generated from testcase below using icx -O1 -S -emit-llvm
; int arr[1024][2], arr2[1024];
;
; void foo()
; {
;   int i1, t1;
;
;   for (i1 = 0; i1 < 100; i1++) {
;     t1 = arr[i1][0];
;     t1 += arr[i1][1];
;     arr2[i1] = t1;
;   }
; }
;
; Test to check that we generate wide load and appropriate shuffles for the
; two loads with stride 2. Test loads from multi dimensional array.
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -enable-vplan-vls-cg -hir-cg -S -print-after=VPlanDriverHIR  < %s 2>&1  | FileCheck %s
; CHECK: DO i1 = 0, 99, 4
; CHECK:  [[WLd:%.*]] = (<8 x i32>*)(@arr)[0][i1][0];
; CHECK:  [[V1:%.*]] = shufflevector [[WLd]],  undef,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK:  [[V2:%.*]] = shufflevector [[WLd]],  undef,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK:  (<4 x i32>*)(@arr2)[0][i1] = [[V1]] + [[V2]];
; CHECK: END LOOP
; ModuleID = 't10.c'
source_filename = "t10.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local local_unnamed_addr global [1024 x [2 x i32]] zeroinitializer, align 16
@arr2 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds [1024 x [2 x i32]], [1024 x [2 x i32]]* @arr, i64 0, i64 %indvars.iv, i64 0
  %0 = load i32, i32* %arrayidx1, align 8, !tbaa !2
  %arrayidx4 = getelementptr inbounds [1024 x [2 x i32]], [1024 x [2 x i32]]* @arr, i64 0, i64 %indvars.iv, i64 1
  %1 = load i32, i32* %arrayidx4, align 4, !tbaa !2
  %add = add nsw i32 %1, %0
  %arrayidx6 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 65e8f9d46b54671e271ba934ab45010c98c98cce) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm b16eab8e883485af9dff2600d4185d17b47f5d3b)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA1024_A2_i", !4, i64 0}
!4 = !{!"array@_ZTSA2_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA1024_i", !5, i64 0}
