; Test two random selects
; 
; opt < %s -analyze -slev | FileCheck -check-prefix=LLVM %s
; RUN: opt < %s -analyze -slev-hir | FileCheck -check-prefix=HIR %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; HIR: AVR SIMD Lane Evolution Analysis:
; HIR: (9)VALUE{i64 i1} ===> {0|STRIDED<1>|AVR-9}
; HIR: (10)VALUE{i64 50} ===> {1|CONSTANT<50>|AVR-10}
; HIR: (11)(30)EXPR{i32* (28)VALUE{i32* %b} getelementptr (29)VALUE{i64 0}}} ===> {5|UNIFORM|AVR-11}
; HIR: (12)(33)EXPR{i32* (31)VALUE{i32* %c} getelementptr (32)VALUE{i64 0}}} ===> {9|UNIFORM|AVR-12}
; HIR: (8)EXPR{select  (9) (10) (11) (12)} ===> {10|RANDOM|AVR-8}
; HIR: (16)VALUE{i64 i1} ===> {11|STRIDED<1>|AVR-16}
; HIR: (17)VALUE{i64 50} ===> {12|CONSTANT<50>|AVR-17}
; HIR: (18)VALUE{i32 5} ===> {13|CONSTANT<5>|AVR-18}
; HIR: (19)VALUE{i32 10} ===> {14|CONSTANT<10>|AVR-19}
; HIR: (15)EXPR{select  (16) (17) (18) (19)} ===> {15|RANDOM|AVR-15}

; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o test.ll -loopopt=false (+ all the flags from #2, but
;    -VPODirectiveCleanup, -loop-vectorize, slp-vectorizer and -loop-unroll)

; void foo(int *restrict b, int *restrict c, int N, int K) {
;   int i;
;   if (N == K)
;     return;
; 
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (i < 50)
;       b[i] = b[i] * 5;
;     else
;       b[i] = c[i] * 10;
;   }
; }

;ModuleID = 'select_noopt.ll'
source_filename = "select.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;Function Attrs : noinline nounwind uwtable
define void @foo(i32 *noalias nocapture %b, i32 *noalias nocapture readonly %c, i32 %N, i32 %K) local_unnamed_addr #0 {
entry:
  %cmp = icmp eq i32 %N, %K
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.body, %if.end
  %indvars.iv = phi i64 [ 0, %if.end ], [ %indvars.iv.next, %for.body ]
  %cmp2 = icmp slt i64 %indvars.iv, 50
  %0 = select i1 %cmp2, i32* %b, i32* %c
  %1 = select i1 %cmp2, i32 5, i32 10
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx, align 4
  %mul = mul nsw i32 %2, %1
  %arrayidx5 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  store i32 %mul, i32* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %return

return:                                           ; preds = %entry, %for.end
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
