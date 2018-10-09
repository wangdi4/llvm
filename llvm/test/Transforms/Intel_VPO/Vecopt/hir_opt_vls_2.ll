; LLVM IR generated from testcase below using icx -O1 -S -emit-llvm
; int arr1[1024];
;
; void foo()
; {
;   int i1;
;
;   for (i1 = 0; i1 < 100; i1++)  {
;       arr1[3 * i1 + 2] = i1;
;       arr1[3 * i1] = i1 + 2;
;       arr1[3 * i1 + 1] = i1 + 1;
;   }
; }
;
; Test to check that we combine the store values, shuffle these values using
; interleaved mask and do a wide store.
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -enable-vplan-vls-cg -hir-cg -S -print-after=VPlanDriverHIR  < %s 2>&1  | FileCheck %s
; CHECK: DO i1 = 0, 99, 4
; CHECK:  [[COMB:%.*]] = shufflevector i1 + <i32 0, i32 1, i32 2, i32 3> + 2,  i1 + <i64 0, i64 1, i64 2, i64 3> + 1,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK:  [[EXT:%.*]] = shufflevector i1 + <i64 0, i64 1, i64 2, i64 3>,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK:  [[COMB1:%.*]] = shufflevector [[COMB]],  [[EXT]],  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11>
; CHECK:  [[VINT:%.*]] = shufflevector [[COMB1]],  undef,  <i32 0, i32 4, i32 8, i32 1, i32 5, i32 9, i32 2, i32 6, i32 10, i32 3, i32 7, i32 11>
; CHECK:  (<12 x i32>*)(@arr1)[0][3 * i1] = [[VINT]]
; CHECK: END LOOP
; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = mul nuw nsw i64 %indvars.iv, 3
  %1 = add nuw nsw i64 %0, 2
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %1
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %arrayidx, align 4, !tbaa !2
  %arrayidx4 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %0
  %3 = trunc i64 %indvars.iv to i32
  %4 = add i32 %3, 2
  store i32 %4, i32* %arrayidx4, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %5 = add nuw nsw i64 %0, 1
  %arrayidx9 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %5
  %6 = trunc i64 %indvars.iv.next to i32
  store i32 %6, i32* %arrayidx9, align 4, !tbaa !2
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
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
