; ModuleID = 'rem3.ll'
; LLVM IR generated from following test using clang -O1 -S -emit-llvm followed by
; opt -loop-simplify.
; int foo(int *arr, unsigned n)
; {
;   unsigned index;
;
;   for (index = 0; index < n; index++)
;     arr[index] = index;
;
;   return 0;
; }
;

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-before=hir-vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

;
; HIR Test.

; Before vectorization
; CHECK: DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 100>   <LEGAL_MAX_TC = 100> <min_trip_count = 10> <avg_trip_count = 55> <max_trip_count = 100>

; After vectorization
; CHECK: DO i1 = 0, {{.*}}, 4   <DO_LOOP>
; CHECK: (<4 x i32>*)(@arr)[0][i1] = i1 +
; CHECK: END LOOP
; CHECK: DO i1 = {{.*}}, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: (@arr)[0][i1] = i1
; CHECK: END LOOP
; ModuleID = 'rem3.ll'
source_filename = "rem3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common global [1024 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32 %n) #0 {
entry:
  %cmp5 = icmp eq i32 %n, 0
  br i1 %cmp5, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !5

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 12546)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = distinct !{!5, !6, !7, !8}
!6 = !{!"llvm.loop.intel.loopcount_maximum", i32 100}
!7 = !{!"llvm.loop.intel.loopcount_minimum", i32 10}
!8 = !{!"llvm.loop.intel.loopcount_average", i32 55}

