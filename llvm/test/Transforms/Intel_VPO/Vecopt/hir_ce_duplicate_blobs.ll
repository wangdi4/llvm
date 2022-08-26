; LLVM IR generated using icx -O1 -S -emit-llvm
; int arr1[1024];
; long arr2[1024];
; 
; void foo(long n)
; {
;   int index;
; 
;   for (index = 0; index < 1024; index++) {
;     n = arr2[index];
;     arr1[n * index + n] = index;
;   }
; }
; 
; RUN: opt -vplan-force-vf=4 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -S < %s 2>&1 | FileCheck %s

; CHECK:      DO i1 = 0, 1023, 4   <DO_LOOP>
; CHECK:      END LOOP
; ModuleID = 'b.c'
source_filename = "b.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr2 = common dso_local local_unnamed_addr global [1024 x i64] zeroinitializer, align 16
@arr1 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo(i64 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i64], [1024 x i64]* @arr2, i64 0, i64 %indvars.iv
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !2
  %mul = mul nsw i64 %0, %indvars.iv
  %add = add nsw i64 %mul, %0
  %arrayidx1 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %add
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx1, align 4, !tbaa !7
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
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 229c5f66f37f029e03e0da7a8a5111061e7a487b) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 9735720c77003976293bdfeb29b906469a38c5b5)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_l", !4, i64 0}
!4 = !{!"long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"array@_ZTSA1024_i", !9, i64 0}
!9 = !{!"int", !5, i64 0}
