; INTEL_FEATURE_SW_ADVANCED
; CMPLRLLVM-33537, case 2

; RUN: opt -passes="loop-mssa(licm)" -enable-intel-advanced-opts=true -S < %s | FileCheck %s

target triple = "i686-pc-linux-gnu"

;; C reproducer:
;; void f(int *ptr, int n) {
;;   for (int i = 0; i < n; ++i) {
;;     int x = *ptr;
;;     if (x)
;;       break;
;;
;;     *ptr = x + 1;
;;   }
;; }

; We do not want to do a PRE-type hoisting of "*ptr", as it inserts a
; loop-carried phi value, and causes suboptimal HIR transformations.

; CHECK-LABEL: for.body.lr.ph:
; CHECK-NOT: load
; CHECK-LABEL: for.body:
; CHECK: load i32, ptr %ptr
; CHECK-LABEL: if.end:

define dso_local void @f(ptr nocapture %ptr, i32 %n) #0 {
entry:
  %cmp7 = icmp slt i32 0, %n
  br i1 %cmp7, label %for.body.lr.ph, label %cleanup1

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %if.end
  %i.08 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %if.end ]
  %0 = load i32, ptr %ptr, align 4
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %if.end, label %for.body.cleanup1_crit_edge

if.end:                                           ; preds = %for.body
  store i32 1, ptr %ptr, align 4
  %inc = add nuw nsw i32 %i.08, 1
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.body, label %for.cond.cleanup1_crit_edge

for.body.cleanup1_crit_edge:                      ; preds = %for.body
  br label %cleanup1

for.cond.cleanup1_crit_edge:                      ; preds = %if.end
  br label %cleanup1

cleanup1:                                         ; preds = %for.cond.cleanup1_crit_edge, %for.body.cleanup1_crit_edge, %entry
  ret void
}

attributes #0 = { "target-cpu"="skylake-avx512" "target-features"="+avx512f,+avx512vl,+avx512dq"}
 ; end INTEL_FEATURE_SW_ADVANCED
