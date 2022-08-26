; LLVM IR generated from the following using icx -O1 -S -emit-llvm
; void foo(int *arr, int n)
; {
;   int i;
;
;   for (i = 0; i < 100; i++)
;     arr[n * i] = i;
; }
; RUN: opt -vplan-force-vf=4 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S  -enable-blob-coeff-vec -enable-nested-blob-vec  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -S -enable-blob-coeff-vec -enable-nested-blob-vec < %s 2>&1 | FileCheck %s

; CHECK:      DO i1 = 0, 99, 4   <DO_LOOP>
; CHECK:          (<4 x i32>*)(%arr)[{{.*}}] = {{.*}}
; CHECK:      END LOOP
; ModuleID = 'tv.c'
source_filename = "tv.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable
define void @foo(i32* nocapture %arr, i32 %n) local_unnamed_addr #0 {
entry:
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = mul nsw i64 %indvars.iv, %0
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %1
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %arrayidx, align 4, !tbaa !2
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
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a1406581a9f24156ce0266f8ac92efd269eacf1e) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f7e9c47a857fd06d29f5eac5ccc0b71173128198)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
