; RUN: opt < %s -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation \
; RUN:     -hir-vec-dir-insert -hir-vplan-vec -debug-only=vplan-idioms \
; RUN:     2>&1 | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec" \
; RUN:     < %s -S -debug-only=vplan-idioms 2>&1 | FileCheck %s

; REQUIRES: asserts
;
; Verify that loop with similar properties as string comparison is not vectorized.
;
; int foo(const int c_size, char *a, const char b)
; {
;     for (int i = 0; i < c_size; ++i) {
;         if (a[c_size-i] != b)
;           return i;
;     }
;   return -1;
; }

; CHECK: StrEq and PtrEq loop was not recognized

;Module Before HIR
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(i32 %c_size, i8* nocapture readonly %a, i8 signext %b) local_unnamed_addr #0 {
entry:
  %cmp9 = icmp sgt i32 %c_size, 0
  br i1 %cmp9, label %for.body.preheader, label %return

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %c_size to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %1 = sub nsw i64 %0, %indvars.iv
  %arrayidx = getelementptr inbounds i8, i8* %a, i64 %1
  %2 = load i8, i8* %arrayidx, align 1, !tbaa !2
  %3 = add nsw i8 %2, 1
  %cmp2 = icmp eq i8 %3, %b
  br i1 %cmp2, label %for.inc, label %return.loopexit.split.loop.exit13

for.inc:                                          ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %return.loopexit

return.loopexit.split.loop.exit13:                ; preds = %for.body
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %for.body ]
  %4 = trunc i64 %indvars.iv.lcssa to i32
  br label %return

return.loopexit:                                  ; preds = %for.inc
  br label %return

return:                                           ; preds = %return.loopexit, %return.loopexit.split.loop.exit13, %entry
  %retval.1 = phi i32 [ -1, %entry ], [ %4, %return.loopexit.split.loop.exit13 ], [ -1, %return.loopexit ]
  ret i32 %retval.1
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang b97cd1e0ccdf66edc0b2a4aadd0de0874ecd119f) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 2f2577128fb98c7755eb33a23b02369927cbfe16)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
