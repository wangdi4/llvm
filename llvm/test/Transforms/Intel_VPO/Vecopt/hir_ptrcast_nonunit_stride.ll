; LLVM IR generated from following testcase using icx -O1 -S -emit-llvm
; int arr[128];
; double darr[128];
;
; void foo()
; {
;   int index;
;   double *dp = darr;
;
;   for (index = 0; index < 128; index++) {
;     *((int *)dp) = index;
;     arr[index] = index;
;     dp++;
;   }
; }
;
; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-force-vf=4 < %s 2>&1 -disable-output | FileCheck %s
;
; CHECK:      BEGIN REGION
; CHECK-NEXT:  DO i1 = 0, 127, 4
; CHECK-NEXT:  (<4 x i32>*)(@darr)[0][i1 + <i64 0, i64 1, i64 2, i64 3>] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:  (<4 x i32>*)(@arr)[0][i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:  END LOOP
; CHECK:      END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@darr = common local_unnamed_addr global [128 x double] zeroinitializer, align 16
@arr = common local_unnamed_addr global [128 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %dp.09 = phi ptr [ @darr, %entry ], [ %incdec.ptr, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %dp.09, align 4, !tbaa !1
  %arrayidx = getelementptr inbounds [128 x i32], ptr @arr, i64 0, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx, align 4, !tbaa !5
  %incdec.ptr = getelementptr inbounds double, ptr %dp.09, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 128
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !2, i64 0}
!6 = !{!"array@_ZTSA128_i", !2, i64 0}
