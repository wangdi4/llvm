; RUN: opt -passes="hir-ssa-deconstruction" < %s -hir-avr-generate -hir-avr-predicate -debug -debug-only=avr-predicate -S -o /dev/null 2>&1| FileCheck %s
; REQUIRES: asserts

; XFAIL: *
; This test did not add -hir-ssa-deconstruction to the command line and hence was constructing incorrect HIR. It triggers  assrtion in the HIR verifier.
; The CHECK directives need to be adjusted to make it work again.


; Verify AVR predicator: two nested if-else statements.

; CHECK: Predicator finished:
; CHECK: LOOP
; CHECK: (2) ASSIGN{(18)EXPR{i32 (19)VALUE{i32 %0}} = (20)EXPR{i32 load (21)VALUE{i32* (%a)[i1]}}}
; CHECK: (54) PREDICATE {P54 := }
; CHECK: (3) if /P54/  ((4)EXPR{i1 (5)VALUE{i32 %0} icmp/sgt (6)VALUE{i32 0}})   {
; CHECK: (55) PREDICATE {P55 := (54) && (60)EXPR{i1 (58)VALUE{i1 &(4)} icmp/eq (59)VALUE{i1 true}}}
; CHECK: (7) ASSIGN{/P55/ (22)EXPR{i32 (23)VALUE{i32 %1}} = (24)EXPR{i32 load (25)VALUE{i32* (%b)[i1]}}}
; CHECK: (8) ASSIGN{/P55/ (26)EXPR{i32 (27)VALUE{i32* (%b)[i1]}} = (28)EXPR{i32 store (29)VALUE{i32 5 * %1}}}
; CHECK: (57) PREDICATE {P57 := (54) && (63)EXPR{i1 (61)VALUE{i1 &(4)} icmp/eq (62)VALUE{i1 false}}}
; CHECK: (9) ASSIGN{/P57/ (30)EXPR{i32 (31)VALUE{i32* (%a)[i1]}} = (32)EXPR{i32 store (33)VALUE{i32 %0 + 5}}}
; CHECK: (56) PREDICATE {P56 := (57) || (55)}
; CHECK: (10) ASSIGN{(34)EXPR{i32 (35)VALUE{i32 %3}} = (36)EXPR{i32 load (37)VALUE{i32* (%c)[i1]}}}
; CHECK: (11) ASSIGN{(38)EXPR{i32 (39)VALUE{i32* (%c)[i1]}} = (40)EXPR{i32 store (41)VALUE{i32 (%N * %3)}}}
; CHECK: (68) PREDICATE {P68 := }
; CHECK: (12) if /P68/  ((13)EXPR{i1 (14)VALUE{i32 (%N * %3)} icmp/sgt (15)VALUE{i32 0}})   {
; CHECK: (69) PREDICATE {P69 := (68) && (74)EXPR{i1 (72)VALUE{i1 &(13)} icmp/eq (73)VALUE{i1 true}}}
; CHECK: (16) ASSIGN{/P69/ (42)EXPR{i32 (43)VALUE{i32* (%a)[i1]}} = (44)EXPR{i32 store (45)VALUE{i32 (%N * %3) + -1 * %2}}}
; CHECK: (71) PREDICATE {P71 := (68) && (77)EXPR{i1 (75)VALUE{i1 &(13)} icmp/eq (76)VALUE{i1 false}}}
; CHECK: (17) ASSIGN{/P71/ (46)EXPR{i32 (47)VALUE{i32* (%b)[i1]}} = (48)EXPR{i32 store (49)VALUE{i32 (%N * %3 * %2)}}}
; CHECK: (70) PREDICATE {P70 := (71) || (69)}


; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)


; void foo(int * restrict a, int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (a[i] > 0) 
;       b[i] = b[i] * 5;
;     else
;       a[i] = a[i] + 5;
; 
;     c[i] = c[i] * N;
; 
;     if (c[i] > 0) 
;       a[i] = c[i] - a[i];
;     else
;       b[i] = a[i] * c[i];
;   }
; }

; ModuleID = 'pred_2x_if_else_noopt.ll'
source_filename = "pred_2x_if_else.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %mul = mul nsw i32 %1, 5
  store i32 %mul, i32* %arrayidx3, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %add = add nsw i32 %0, 5
  store i32 %add, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %2 = phi i32 [ %add, %if.else ], [ %0, %if.then ]
  %arrayidx11 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx11, align 4
  %mul12 = mul nsw i32 %3, %N
  store i32 %mul12, i32* %arrayidx11, align 4
  %cmp17 = icmp sgt i32 %mul12, 0
  br i1 %cmp17, label %if.then18, label %if.else25

if.then18:                                        ; preds = %if.end
  %sub = sub nsw i32 %mul12, %2
  store i32 %sub, i32* %arrayidx, align 4
  br label %for.inc

if.else25:                                        ; preds = %if.end
  %mul30 = mul nsw i32 %2, %mul12
  %arrayidx32 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  store i32 %mul30, i32* %arrayidx32, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then18, %if.else25
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
