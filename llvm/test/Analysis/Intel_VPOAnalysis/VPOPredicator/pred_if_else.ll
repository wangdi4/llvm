; RUN: opt < %s -hir-avr-generate -hir-avr-predicate -debug -debug-only=avr-predicate -S -o /dev/null 2>&1| FileCheck %s
; REQUIRES: asserts

; Verify AVR predicator: simple if-else statement.

; CHECK: Predicator finished:
; CHECK: LOOP

; CHECK: (40) PREDICATE {P40 := }
; CHECK: (3) if /P40/  ((4)EXPR{i1 (5)VALUE{i32 %0} icmp/eq (6)VALUE{i32 0}})   {
; CHECK: (41) PREDICATE {P41 := (40) && (46)EXPR{i1 (44)VALUE{i1 &(4)} icmp/eq (45)VALUE{i1 true}}}
; CHECK: (7) ASSIGN{/P41/ (16)EXPR{i32 (17)VALUE{i32 %1}} = (18)EXPR{i32 load (19)VALUE{i32* (%b)[i1]}}}
; CHECK: (8) ASSIGN{/P41/ (20)EXPR{i32 (21)VALUE{i32* (%b)[i1]}} = (22)EXPR{i32 store (23)VALUE{i32 5 * %1}}}
; CHECK: (43) PREDICATE {P43 := (40) && (49)EXPR{i1 (47)VALUE{i1 &(4)} icmp/eq (48)VALUE{i1 false}}}
; CHECK: (9) ASSIGN{/P43/ (24)EXPR{i32 (25)VALUE{i32* (%a)[i1]}} = (26)EXPR{i32 store (27)VALUE{i32 %0 + 5}}}
; CHECK: (42) PREDICATE {P42 := (43) || (41)}
; CHECK: (10) ASSIGN{(28)EXPR{i32 (29)VALUE{i32 %2}} = (30)EXPR{i32 load (31)VALUE{i32* (%c)[i1]}}}
; CHECK: (11) ASSIGN{(32)EXPR{i32 (33)VALUE{i32* (%c)[i1]}} = (34)EXPR{i32 store (35)VALUE{i32 (%N * %2)}}}

; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)

; void foo(int * restrict a, int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (a[i] == 0)
;       b[i] = b[i] * 5;
;     else
;       a[i] = a[i] + 5;
; 
;     c[i] = c[i] * N;
;   }
; }


; ModuleID = 'pred_if_else.ll'
source_filename = "pred_if_else.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N, i32 %M, i32 %K) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp eq i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %mul = mul nsw i32 %1, 5
  store i32 %mul, i32* %arrayidx3, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %add = add nsw i32 %0, 5
  store i32 %add, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %arrayidx11 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx11, align 4
  %mul12 = mul nsw i32 %2, %N
  store i32 %mul12, i32* %arrayidx11, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
