; Test stride-one multidimensional arrays (load and store)
; 
; opt < %s -analyze -slev | FileCheck -check-prefix=LLVM %s
; RUN: opt < %s -analyze -slev-hir | FileCheck -check-prefix=HIR %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; HIR: (12)VALUE{[300 x [300 x [300 x i32]]]* @b} ===> {0|UNIFORM|AVR-12}
; HIR: (13)VALUE{i64 0} ===> {1|CONSTANT<0>|AVR-13}
; HIR: (14)VALUE{i64 i1} ===> {2|UNIFORM|AVR-14}
; HIR: (15)VALUE{i64 i2} ===> {3|UNIFORM|AVR-15}
; HIR: (16)VALUE{i64 i3} ===> {4|STRIDED<1>|AVR-16}
; HIR: (17)EXPR{getelementptr  (12) (13) (14) (15) (16)} ===> {5|STRIDED<1>|AVR-17}
; HIR: (7)(17)EXPR{[300 x [300 x [300 x i32]]]* getelementptr (12)VALUE{[300 x [300 x [300 x i32]]]* @b} (13)VALUE{i64 0} (14)VALUE{i64 i1} (15)VALUE{i64 i2} (16)VALUE{i64 i3} }} ===> {6|STRIDED<1>|AVR-7}
; HIR: (6)EXPR{load (7)} ===> {7|RANDOM|AVR-6}
; HIR: (20)EXPR{(19) mul (18)} ===> {10|RANDOM|AVR-20}
; HIR: (11)(20)EXPR{i32 (19)VALUE{i32 %0} mul (18)VALUE{i32 %K}}} ===> {11|RANDOM|AVR-11}
; HIR: (10)EXPR{store (11)} ===> {12|RANDOM|AVR-10}
; HIR: (21)VALUE{[300 x [300 x [300 x i32]]]* @c} ===> {13|UNIFORM|AVR-21}
; HIR: (22)VALUE{i64 0} ===> {14|CONSTANT<0>|AVR-22}
; HIR: (23)VALUE{i64 i1} ===> {15|UNIFORM|AVR-23}
; HIR: (24)VALUE{i64 i2} ===> {16|UNIFORM|AVR-24}
; HIR: (25)VALUE{i64 i3} ===> {17|STRIDED<1>|AVR-25}
; HIR: (26)EXPR{getelementptr  (21) (22) (23) (24) (25)} ===> {18|STRIDED<1>|AVR-26}
; HIR: (9)(26)EXPR{[300 x [300 x [300 x i32]]]* getelementptr (21)VALUE{[300 x [300 x [300 x i32]]]* @c} (22)VALUE{i64 0} (23)VALUE{i64 i1} (24)VALUE{i64 i2} (25)VALUE{i64 i3} }} ===> {19|STRIDED<1>|AVR-9}

; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o test.ll -loopopt=false (+ all the flags from #2, but
;    -VPODirectiveCleanup, -loop-vectorize, slp-vectorizer and -loop-unroll)

; int b[300][300][300];
; int c[300][300][300];
; 
; void foo(int *p, int N, int K)
; {
;   int i, j, k;
;   if (N == K)
;     return;
; 
;   for (k = 0; k < 300; k++)
;     for (j = 0; j < 300; j++) {
; #pragma omp simd
;       for (i = 0; i < 300; i++) {
;         c[k][j][i] = b[k][j][i] * K * *p;
;       }
;     }
; }


; ModuleID = 'arrays_noopt.ll'
source_filename = "arrays.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common local_unnamed_addr global [300 x [300 x [300 x i32]]] zeroinitializer, align 16
@c = common local_unnamed_addr global [300 x [300 x [300 x i32]]] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* nocapture readnone %p, i32 %N, i32 %K) local_unnamed_addr #0 {
entry:
  %cmp = icmp eq i32 %N, %K
  br i1 %cmp, label %for.end23, label %for.cond2.preheader.preheader

for.cond2.preheader.preheader:                    ; preds = %entry
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond2.preheader.preheader, %for.inc21
  %indvars.iv7 = phi i64 [ %indvars.iv.next8, %for.inc21 ], [ 0, %for.cond2.preheader.preheader ]
  br label %for.body4

for.body4:                                        ; preds = %for.inc18, %for.cond2.preheader
  %indvars.iv4 = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next5, %for.inc18 ]
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.inc

for.inc:                                          ; preds = %for.inc, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body4 ], [ %indvars.iv.next, %for.inc ]
  %arrayidx11 = getelementptr inbounds [300 x [300 x [300 x i32]]], [300 x [300 x [300 x i32]]]* @b, i64 0, i64 %indvars.iv7, i64 %indvars.iv4, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx11, align 4
  %mul = mul nsw i32 %0, %K
  %arrayidx17 = getelementptr inbounds [300 x [300 x [300 x i32]]], [300 x [300 x [300 x i32]]]* @c, i64 0, i64 %indvars.iv7, i64 %indvars.iv4, i64 %indvars.iv
  store i32 %mul, i32* %arrayidx17, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond, label %for.inc18, label %for.inc

for.inc18:                                        ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %indvars.iv.next5 = add nuw nsw i64 %indvars.iv4, 1
  %exitcond6 = icmp eq i64 %indvars.iv.next5, 300
  br i1 %exitcond6, label %for.inc21, label %for.body4

for.inc21:                                        ; preds = %for.inc18
  %indvars.iv.next8 = add nuw nsw i64 %indvars.iv7, 1
  %exitcond9 = icmp eq i64 %indvars.iv.next8, 300
  br i1 %exitcond9, label %for.end23.loopexit, label %for.cond2.preheader

for.end23.loopexit:                               ; preds = %for.inc21
  br label %for.end23

for.end23:                                        ; preds = %for.end23.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
