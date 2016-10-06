; RUN: opt < %s -hir-avr-generate -avr-predicate -debug -debug-only=avr-predicate -S -o /dev/null 2>&1| FileCheck %s
; REQUIRES: asserts

; Verify the AVR predicator: if with "short-circuit" condition, no goto.

; CHECK: Predicator finished:
; CHECK: LOOP
; CHECK: (3) ASSIGN{(17)EXPR{i1 (18)VALUE{i1 %cmp1}} = (19)EXPR{i1 (20)VALUE{i32 %0} icmp/sgt (21)VALUE{i32 100}}}
; CHECK: (4) ASSIGN{(22)EXPR{i1 (23)VALUE{i1 %cmp4}} = (24)EXPR{i1 (25)VALUE{i32 %0} icmp/eq (26)VALUE{i32 0}}}
; CHECK: (5) ASSIGN{(27)EXPR{i1 (28)VALUE{i1 %or.cond}} = (29)EXPR{i1 (30)VALUE{i1 %cmp1} or (31)VALUE{i1 %cmp4}}}
; CHECK: (47) PREDICATE {P47 := }
; CHECK: (6) if /P47/  ((7)EXPR{i1 (8)VALUE{i1 %or.cond} icmp/eq (9)VALUE{i1 false}})   {
; CHECK: }
; CHECK: (48) PREDICATE {P48 := (47) && (52)EXPR{i1 (50)VALUE{i1 &(7)} icmp/eq (51)VALUE{i1 true}}}
; CHECK: (10) ASSIGN{/P48/ (32)EXPR{i32 (33)VALUE{i32* {al:4}(%b)[i1]}} = (34)EXPR{i32 store (35)VALUE{i32 5 * %0}}}
; CHECK: (49) PREDICATE {P49 := (47) && (55)EXPR{i1 (53)VALUE{i1 &(7)} icmp/eq (54)VALUE{i1 false}} || (48)}
; CHECK: (11) ASSIGN{(36)EXPR{i32 (37)VALUE{i32 %1}} = (38)EXPR{i32 load (39)VALUE{i32* {al:4}(%c)[i1]}}}
; CHECK: (12) ASSIGN{(40)EXPR{i32 (41)VALUE{i32* {al:4}(%c)[i1]}} = (42)EXPR{i32 store (43)VALUE{i32 (%N * %1)}}}


; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)

; void foo(int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (b[i] <= 100 && b[i] != 0) 
;       b[i] = b[i] * 5;
; 
;     c[i] = c[i] * N;
;   }
; }


; ModuleID = 'pred_if_else_sc_nogoto_noopt.ll'
source_filename = "pred_if_else_sc_nogoto.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture readnone %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N, i32 %M, i32 %K) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 100
  %cmp4 = icmp eq i32 %0, 0
  %or.cond = or i1 %cmp1, %cmp4
  br i1 %or.cond, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %mul = mul nsw i32 %0, 5
  store i32 %mul, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %arrayidx10 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx10, align 4
  %mul11 = mul nsw i32 %1, %N
  store i32 %mul11, i32* %arrayidx10, align 4
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

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (branches/vpo 20354) (llvm/branches/vpo 20401)"}
