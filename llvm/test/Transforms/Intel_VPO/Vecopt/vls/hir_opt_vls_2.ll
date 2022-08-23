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

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -enable-vplan-vls-cg -hir-cg -disable-output -print-after=hir-vplan-vec  < %s 2>&1  -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -enable-vplan-vls-cg -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -enable-vplan-vls-cg -hir-cg -disable-output -print-after=hir-vplan-vec  < %s 2>&1  -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -enable-vplan-vls-cg -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s
; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
; CHECK-LABEL:  Function: foo
; CHECK-EMPTY:
; CHECK-NEXT:  BEGIN REGION { modified }
; CHECK-NEXT:        + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTEXTENDED0:%.*]] = shufflevector i1 + <i32 0, i32 1, i32 2, i32 3> + 2,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:        |   [[SHUFFLE0:%.*]] = shufflevector undef,  [[DOTEXTENDED0]],  <i32 16, i32 1, i32 2, i32 17, i32 4, i32 5, i32 18, i32 7, i32 8, i32 19, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:        |   [[DOTEXTENDED10:%.*]] = shufflevector i1 + <i64 0, i64 1, i64 2, i64 3> + 1,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:        |   [[SHUFFLE20:%.*]] = shufflevector [[SHUFFLE0]],  [[DOTEXTENDED10]],  <i32 0, i32 16, i32 2, i32 3, i32 17, i32 5, i32 6, i32 18, i32 8, i32 9, i32 19, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:        |   [[DOTEXTENDED30:%.*]] = shufflevector i1 + <i32 0, i32 1, i32 2, i32 3>,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:        |   [[SHUFFLE40:%.*]] = shufflevector [[SHUFFLE20]],  [[DOTEXTENDED30]],  <i32 0, i32 1, i32 16, i32 3, i32 4, i32 17, i32 6, i32 7, i32 18, i32 9, i32 10, i32 19, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:        |   (<16 x i32>*)(@arr1)[0][3 * i1] = [[SHUFFLE40]]
; CHECK-NEXT:        + END LOOP
; CHECK:       END REGION
;
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
