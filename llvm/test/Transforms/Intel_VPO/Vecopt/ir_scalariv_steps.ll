; ModuleID = 's4.c'
; void baz1(long);
;
; void foo() {
;   long index;
;
; #pragma omp simd simdlen(2)
;   for (index = 0; index < 1024; index++) {
;       baz1(index);
;   }
; }
; RUN: opt -VPlanDriver -enable-vp-value-codegen=false -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-IRCG
; RUN: opt -VPlanDriver -enable-vp-value-codegen=true  -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-VPCG
; TODO: Merge IRCG and VPCG checks after CMPLRLLVM-10781 is fixed.
;
; CHECK: vector.ph:
; CHECK: vector.body:
; CHECK:   %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
; CHECK-IRCG: [[IND1:.*]] = add i64 %index, 1
; CHECK-VPCG: [[IND1:.*]] = extractelement <2 x i64> [[VEC_PHI:.*]], i32 1
; CHECK: call void @baz1(i64 %{{.*}})
; CHECK-NOT: {{.*}} = add i64 %index, 0

; source_filename = "s4.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo() {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %entry
  %.omp.iv.06 = phi i64 [ 0, %entry ], [ %add1, %omp.inner.for.body ]
  tail call void @baz1(i64 %.omp.iv.06) #3
  %add1 = add nuw nsw i64 %.omp.iv.06, 1
  %exitcond = icmp eq i64 %add1, 1024
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare void @baz1(i64) 

; Function Attrs: argmemonly nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: argmemonly nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21478)"}
