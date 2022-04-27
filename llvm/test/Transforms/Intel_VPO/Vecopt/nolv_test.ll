; RUN: opt -enable-new-pm=0 -O2 -S -pass-remarks=loop-vectorize  -pass-remarks-analysis=loop-vectorize < %s 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=0 -O3 -S -pass-remarks=loop-vectorize  -pass-remarks-analysis=loop-vectorize < %s 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=0 -O1 -vectorize-loops -S -pass-remarks=loop-vectorize  -pass-remarks-analysis=loop-vectorize < %s 2>&1 | FileCheck %s

; RUN: opt -enable-new-pm=1 -O2 -S -pass-remarks=loop-vectorize  -pass-remarks-analysis=loop-vectorize < %s 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=1 -O3 -S -pass-remarks=loop-vectorize  -pass-remarks-analysis=loop-vectorize < %s 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=1 -O1 -vectorize-loops -S -pass-remarks=loop-vectorize  -pass-remarks-analysis=loop-vectorize < %s 2>&1 | FileCheck %s
; CHECK-NOT: remark: {{.*}} vectorized
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable
define void @foo(i32* nocapture %arr) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
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
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 1cc2ca2a2fc32b4802cf166efd34dda53148cd76) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e886002b323c5e48b95635e71b26cf38adb4e506)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
